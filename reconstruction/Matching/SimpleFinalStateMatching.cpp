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
        _discardBTagging(false)
    {
        _input = addSink("input", "input");
        _output = addSource("output", "output");

        addOption("generator event view","generator event view",_inputGenEventViewName);
        addOption("reconstructed event view","generator event view",_inputRecoEventViewName);
        addOption("output event view","generator event view",_outputEventViewName);
        
        addOption("discard bTagging","allows matches between jets and bjets",_discardBTagging);
        
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

    void addMatchToView(std::vector<pxl::Particle*>& gen, std::vector<pxl::Particle*>& reco,pxl::EventView* eventView)
    {
        if (gen.size()<=reco.size())
        {
            for (unsigned igen=0; igen<gen.size(); ++igen)
            {
                pxl::Particle* genMatch = eventView->create<pxl::Particle>();
                genMatch->setName(gen[igen]->getName());
                copyParticleProperties(genMatch,gen[igen]);
                pxl::Particle* recoMatch = eventView->create<pxl::Particle>();
                recoMatch->setName(reco[igen]->getName());
                copyParticleProperties(recoMatch,reco[igen]);
                recoMatch->linkMother(genMatch);
            }
        }
        else
        {
            for (unsigned ireco=0; ireco<reco.size(); ++ireco)
            {
                pxl::Particle* genMatch = eventView->create<pxl::Particle>();
                genMatch->setName(gen[ireco]->getName());
                copyParticleProperties(genMatch,gen[ireco]);
                pxl::Particle* recoMatch = eventView->create<pxl::Particle>();
                recoMatch->setName(reco[ireco]->getName());
                copyParticleProperties(recoMatch,reco[ireco]);
                recoMatch->linkMother(genMatch);
            }
        }
    }

    void match(std::vector<pxl::Particle*>& gen, std::vector<pxl::Particle*>& reco)
    {
        std::vector<pxl::Particle*> bestMatch;
        if (gen.size()<=reco.size())
        {
            std::sort(reco.begin(),reco.end(),compare);
            float mindR=1000.0;
            do {
                float dR=0.0;
                for (unsigned igen=0; igen<gen.size(); ++igen)
                {
                    dR+=gen[igen]->getVector().deltaR(reco[igen]->getVector());
                }
                if (mindR>dR)
                {
                    mindR=dR;
                    bestMatch=std::vector<pxl::Particle*>();
                    for (unsigned ireco=0; ireco<reco.size(); ++ireco)
                    {
                        bestMatch.push_back(reco[ireco]);
                    }

                }
            } while ( std::next_permutation(reco.begin(),reco.end(),compare));
            for (unsigned ireco=0; ireco<reco.size(); ++ireco)
            {
                reco[ireco]=bestMatch[ireco];
            }
        }
        else
        {
            std::sort(gen.begin(),gen.end(),compare);
            float mindR=1000.0;
            do {
                float dR=0.0;
                for (unsigned ireco=0; ireco<reco.size(); ++ireco)
                {
                    dR+=reco[ireco]->getVector().deltaR(gen[ireco]->getVector());
                }
                if (mindR>dR)
                {
                    mindR=dR;
                    bestMatch=std::vector<pxl::Particle*>();
                    for (unsigned igen=0; igen<gen.size(); ++igen)
                    {
                        bestMatch.push_back(gen[igen]);
                    }

                }
            } while ( std::next_permutation(gen.begin(),gen.end(),compare));\
            for (unsigned igen=0; igen<gen.size(); ++igen)
            {
                gen[igen]=bestMatch[igen];
            }
        }
    }

    bool analyse(pxl::Sink *sink) throw (std::runtime_error)
    {
        try
        {
            pxl::Event *event  = dynamic_cast<pxl::Event *> (sink->get());
            if (event)
            {


                pxl::EventView* outputEventView = event->create<pxl::EventView>();
                outputEventView->setName(_outputEventViewName);

                std::vector<pxl::Particle*> genMuons;
                std::vector<pxl::Particle*> genElectrons;
                std::vector<pxl::Particle*> genQuarks;
                std::vector<pxl::Particle*> genBQuarks;
                pxl::Particle* genMET = outputEventView->create<pxl::Particle>();
                genMET->setName("GenMET");

                std::vector<pxl::Particle*> recoMuons;
                std::vector<pxl::Particle*> recoElectrons;
                std::vector<pxl::Particle*> recoJets;
                std::vector<pxl::Particle*> recoBJets;
                pxl::Particle* recoMET = outputEventView->create<pxl::Particle>();
                recoMET->setName(_inputRecoMETName);
                recoMET->linkMother(genMET);

                std::vector<pxl::EventView*> eventViews;
                event->getObjectsOfType(eventViews);
                for (unsigned ieventView=0; ieventView<eventViews.size();++ieventView)
                {
                    pxl::EventView* eventView = eventViews[ieventView];
                    if (eventView->getName()==_inputGenEventViewName)
                    {
                        std::vector<pxl::Particle*> genParticles;
                        eventView->getObjectsOfType(genParticles);
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
                

                match(genMuons,recoMuons);
                addMatchToView(genMuons,recoMuons,outputEventView);
                match(genElectrons,recoElectrons);
                addMatchToView(genElectrons,recoElectrons,outputEventView);
                match(genQuarks,recoJets);
                addMatchToView(genQuarks,recoJets,outputEventView);
                match(genBQuarks,recoBJets);
                addMatchToView(genBQuarks,recoBJets,outputEventView);
                


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
