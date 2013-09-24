#include "pxl/hep.hh"
#include "pxl/core.hh"
#include "pxl/core/macros.hh"
#include "pxl/core/PluginManager.hh"
#include "pxl/modules/Module.hh"
#include "pxl/modules/ModuleFactory.hh"

#include <algorithm>

static pxl::Logger logger("SimpleFinalStateMatching");

bool compare(pxl::Particle* p1, pxl::Particle* p2)
{
    return p1->getPt()>p2->getPt();
}

class SimpleFinalStateMatching : public pxl::Module
{
    private:
    
    pxl::Sink* _input;
    pxl::Source* _output;

    std::string _inputGenEventViewName;
    std::string _inputRecoEventViewName;
    std::string _outputEventViewName;
    
    std::string _inputRecoMETName;
    std::string _inputRecoElectronName;
    std::string _inputRecoMuonName;
    std::string _inputRecoJetName;
    std::string _inputRecoBJetName;
    
    bool _discardBTagging;
    bool _copyOnlyFinalParticles;
    

    public:
    SimpleFinalStateMatching() :
        Module(),
        _inputGenEventViewName("Generated"),
        _inputRecoEventViewName("Reconstructed"),
        _outputEventViewName("Matched"),
        _inputRecoMETName("Neutrino"),
        _inputRecoElectronName("TightElectron"),
        _inputRecoMuonName("TightMuon"),
        _inputRecoJetName("SelectedJet"),
        _inputRecoBJetName("SelectedBJet"),
        _discardBTagging(false),
        _copyOnlyFinalParticles(true)
    {
        _input = addSink("input", "input");
        _output = addSource("output", "output");

        addOption("generator event view","generator event view",_inputGenEventViewName);
        addOption("reconstructed event view","generator event view",_inputRecoEventViewName);
        addOption("output event view","generator event view",_outputEventViewName);
        
        addOption("discard bTagging","allows matches between jets and bjets",_discardBTagging);
        addOption("copy only final","copies only the final state generator particles",_copyOnlyFinalParticles);
        
        addOption("reco met","name of the reconstructed met",_inputRecoMETName);
        addOption("reco electon","names of the reconstructed electon",_inputRecoElectronName);
        addOption("reco muon","names of the reconstructed muon",_inputRecoMuonName);
        addOption("reco jet","name of the reconstructed jets",_inputRecoJetName);
        addOption("reco bjet","name of the reconstructed bjets",_inputRecoBJetName);
    }

    ~SimpleFinalStateMatching()
    {
    }

    // every Module needs a unique type
    static const std::string &getStaticType()
    {
        static std::string type ("SimpleFinalStateMatching");
        return type;
    }

    // static and dynamic methods are needed
    const std::string &getType() const
    {
        return getStaticType();
    }

    bool isRunnable() const
    {
        // this module does not provide events, so return false
        return false;
    }

    void initialize() throw (std::runtime_error)
    {
    }

    void beginJob() throw (std::runtime_error)
    {
        getOption("generator event view",_inputGenEventViewName);
        getOption("reconstructed event view",_inputRecoEventViewName);
        getOption("output event view",_outputEventViewName);
        
        getOption("discard bTagging",_discardBTagging);
        getOption("copy only final",_copyOnlyFinalParticles);
        
        getOption("reco met",_inputRecoMETName);
        getOption("reco electon",_inputRecoElectronName);
        getOption("reco muon",_inputRecoMuonName);
        getOption("reco jet",_inputRecoJetName);
        getOption("reco bjet",_inputRecoBJetName);
    }

    void endJob()
    {
        
    }
    
    void copyParticleProperties(pxl::Particle* target, pxl::Particle* source)
    {
        target->setCharge(source->getCharge());
        target->setPdgNumber(source->getPdgNumber());
        target->setP4(source->getVector());
        target->setUserRecords(source->getUserRecords());
    }
    

    bool isFinal(pxl::Particle* particle)
    {
        return particle->getDaughterRelations().size()==0;
    }

    void addMatchToView(std::vector<pxl::Particle*>& gen, std::vector<pxl::Particle*>& reco,pxl::EventView* eventView, float(*matchFunction)(const pxl::Particle*, const pxl::Particle*))
    {
        if (gen.size()<=reco.size())
        {
            for (unsigned igen=0; igen<gen.size(); ++igen)
            {
                pxl::Particle* genMatch=gen[igen];
                if (_copyOnlyFinalParticles)
                {
                    genMatch = eventView->create<pxl::Particle>();
                    genMatch->setName(gen[igen]->getName());
                    copyParticleProperties(genMatch,gen[igen]);
                }
                pxl::Particle* recoMatch = eventView->create<pxl::Particle>();
                recoMatch->setName(reco[igen]->getName());
                copyParticleProperties(recoMatch,reco[igen]);
                recoMatch->linkMother(genMatch);
                genMatch->setUserRecord("match_value",matchFunction(genMatch,recoMatch));
            }
        }
        else
        {
            for (unsigned ireco=0; ireco<reco.size(); ++ireco)
            {
                pxl::Particle* genMatch=gen[ireco];
                if (_copyOnlyFinalParticles)
                {
                    genMatch = eventView->create<pxl::Particle>();
                    genMatch->setName(gen[ireco]->getName());
                    copyParticleProperties(genMatch,gen[ireco]);
                }
                pxl::Particle* recoMatch = eventView->create<pxl::Particle>();
                recoMatch->setName(reco[ireco]->getName());
                copyParticleProperties(recoMatch,reco[ireco]);
                recoMatch->linkMother(genMatch);
                genMatch->setUserRecord("match_value",matchFunction(genMatch,recoMatch));
            }
        }
    }

    static float deltaRmatch(const pxl::Particle* p1, const pxl::Particle* p2)
    {
        return p1->getVector().deltaR(p2->getVector());
    }

    void match(std::vector<pxl::Particle*>& gen, std::vector<pxl::Particle*>& reco, float(*matchFunction)(const pxl::Particle*, const pxl::Particle*))
    {
        std::vector<pxl::Particle*> bestMatch;
        int permutation=0;
        if (gen.size()<=reco.size())
        {
            std::sort(reco.begin(),reco.end(),compare);
            float minMatchValue=1000.0;
            do {
                float matchValue=0.0;
                for (unsigned igen=0; igen<gen.size(); ++igen)
                {
                    matchValue+=matchFunction(gen[igen],reco[igen]);
                }
                if (minMatchValue>matchValue)
                {
                    minMatchValue=matchValue;
                    bestMatch=std::vector<pxl::Particle*>();
                    for (unsigned ireco=0; ireco<reco.size(); ++ireco)
                    {
                        bestMatch.push_back(reco[ireco]);
                    }

                }
                ++permutation;
            } while ( std::next_permutation(reco.begin(),reco.end(),compare));
            for (unsigned ireco=0; ireco<reco.size(); ++ireco)
            {
                reco[ireco]=bestMatch[ireco];
            }
        }
        else
        {
            std::sort(gen.begin(),gen.end(),compare);
            float minMatchValue=1000.0;
            do {
                float matchValue=0.0;
                for (unsigned ireco=0; ireco<reco.size(); ++ireco)
                {
                    matchValue+=matchFunction(gen[ireco],reco[ireco]);
                }
                if (minMatchValue>matchValue)
                {
                    minMatchValue=matchValue;
                    bestMatch=std::vector<pxl::Particle*>();
                    for (unsigned igen=0; igen<gen.size(); ++igen)
                    {
                        bestMatch.push_back(gen[igen]);
                    }

                }
                ++permutation;
            } while ( std::next_permutation(gen.begin(),gen.end(),compare));\
            for (unsigned igen=0; igen<gen.size(); ++igen)
            {
                gen[igen]=bestMatch[igen];
            }
        }
        logger(pxl::LOG_LEVEL_DEBUG,"elements: ",std::max(gen.size(),reco.size()),"permutations: ",permutation);
    }

    bool analyse(pxl::Sink *sink) throw (std::runtime_error)
    {
        try
        {
            pxl::Event *event  = dynamic_cast<pxl::Event *> (sink->get());
            if (event)
            {
                pxl::EventView* outputEventView;
                std::vector<pxl::Particle*> genMuons;
                std::vector<pxl::Particle*> genElectrons;
                std::vector<pxl::Particle*> genQuarks;
                std::vector<pxl::Particle*> genBQuarks;
                pxl::Particle* genMET = new pxl::Particle();
                genMET->setName("GenMET");

                std::vector<pxl::Particle*> recoMuons;
                std::vector<pxl::Particle*> recoElectrons;
                std::vector<pxl::Particle*> recoJets;
                std::vector<pxl::Particle*> recoBJets;
                pxl::Particle* recoMET = new pxl::Particle();
                recoMET->setName(_inputRecoMETName);

                std::vector<pxl::EventView*> eventViews;
                event->getObjectsOfType(eventViews);
                for (unsigned ieventView=0; ieventView<eventViews.size();++ieventView)
                {
                    pxl::EventView* eventView = eventViews[ieventView];
                    if (eventView->getName()==_inputGenEventViewName)
                    {
                        if (_copyOnlyFinalParticles)
                        {
                            outputEventView=event->create<pxl::EventView>();
                            outputEventView->setName(_outputEventViewName);
                        }
                        else
                        {
                            outputEventView=dynamic_cast<pxl::EventView*>(eventView->clone());
                            outputEventView->setName(_outputEventViewName);
                            event->insertObject(outputEventView);
                        }

                        std::vector<pxl::Particle*> genParticles;
                        //work directly with the already copied objects
                        if (!_copyOnlyFinalParticles)
                        {
                            outputEventView->getObjectsOfType(genParticles);
                        }
                        else
                        {
                            eventView->getObjectsOfType(genParticles);
                        }
                        for (unsigned iparticle=0; iparticle<genParticles.size();++iparticle)
                        {
                            pxl::Particle* particle = genParticles[iparticle];
                            if (!isFinal(particle))
                            {
                                continue;
                            }
                            if (abs(particle->getPdgNumber())==11)
                            {
                                genElectrons.push_back(particle);
                            }
                            else if (abs(particle->getPdgNumber())==13)
                            {
                                genMuons.push_back(particle);
                            }
                            else if (abs(particle->getPdgNumber())==12 || abs(particle->getPdgNumber())==14 || abs(particle->getPdgNumber())==16)
                            {
                                genMET->getVector()+=particle->getVector();
                            }
                            else if (abs(particle->getPdgNumber())==5)
                            {
                                if (_discardBTagging)
                                {
                                    genQuarks.push_back(particle);
                                }
                                else
                                {
                                    genBQuarks.push_back(particle);
                                }
                            }
                            else if (abs(particle->getPdgNumber())<5)
                            {
                                genQuarks.push_back(particle);
                            }
                        }
                    }
                    else if (eventView->getName()==_inputRecoEventViewName)
                    {
                        std::vector<pxl::Particle*> recoParticles;
                        eventView->getObjectsOfType(recoParticles);
                        for (unsigned iparticle=0; iparticle<recoParticles.size();++iparticle)
                        {
                            pxl::Particle* particle = recoParticles[iparticle];
                            if (particle->getName()==_inputRecoElectronName)
                            {
                                recoElectrons.push_back(particle);
                            }
                            else if (particle->getName()==_inputRecoMuonName)
                            {
                                recoMuons.push_back(particle);
                            }
                            else if (particle->getName()==_inputRecoMETName)
                            {
                                copyParticleProperties(recoMET,particle);
                            }
                            else if (particle->getName()==_inputRecoBJetName)
                            {
                                if (_discardBTagging)
                                {
                                    recoJets.push_back(particle);
                                }
                                else
                                {
                                    recoBJets.push_back(particle);
                                }
                            }
                            else if (particle->getName()==_inputRecoJetName)
                            {
                                recoJets.push_back(particle);
                            }
                        }
                    }
                }
                

                float (*matchFunction)(const pxl::Particle*, const pxl::Particle*) = &SimpleFinalStateMatching::deltaRmatch;

                match(genMuons,recoMuons,matchFunction);
                addMatchToView(genMuons,recoMuons,outputEventView,matchFunction);
                match(genElectrons,recoElectrons,matchFunction);
                addMatchToView(genElectrons,recoElectrons,outputEventView,matchFunction);
                match(genQuarks,recoJets,matchFunction);
                addMatchToView(genQuarks,recoJets,outputEventView,matchFunction);
                match(genBQuarks,recoBJets,matchFunction);
                addMatchToView(genBQuarks,recoBJets,outputEventView,matchFunction);

                outputEventView->insertObject(genMET);
                outputEventView->insertObject(recoMET);
                genMET->linkDaughter(recoMET);

                _output->setTargets(event);
                return _output->processTargets();


                
            }
        }
        catch(std::exception &e)
        {
            throw std::runtime_error(getName()+": "+e.what());
        }
        catch(...)
        {
            throw std::runtime_error(getName()+": unknown exception");
        }

        logger(pxl::LOG_LEVEL_ERROR , "Analysed event is not an pxl::Event !");
        return false;
    }

    void shutdown() throw(std::runtime_error)
    {
    }

    void destroy() throw (std::runtime_error)
    {
        delete this;
    }
};

PXL_MODULE_INIT(SimpleFinalStateMatching)
PXL_PLUGIN_INIT
