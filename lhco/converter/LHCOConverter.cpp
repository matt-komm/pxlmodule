#include "pxl/hep.hh"
#include "pxl/core.hh"
#include "pxl/core/macros.hh"
#include "pxl/core/PluginManager.hh"
#include "pxl/modules/Module.hh"
#include "pxl/modules/ModuleFactory.hh"

#include "LHCO/LHCO.hpp"

static pxl::Logger logger("LHCOConverter");

class LHCOConverter : public pxl::Module
{
    private:
    pxl::Source* _output;

    std::ofstream* ofs;

    std::string _outFileName;
    std::string _inputEventViewName;

    std::string _jetName;
    std::string _bjetName;
    std::string _electronName;
    std::string _muonName;
    std::string _metName;
    std::string _photonName;

    public:
    LHCOConverter() :
        Module(),
        _outFileName("input.lhco"),
        _inputEventViewName("Reconstructed"),
        _jetName("SelectedJet"),
        _bjetName("SelectedBJet"),
        _electronName("TightElectron"),
        _muonName("TightMuon"),
        _metName("MET"),
        _photonName("TightPhoton")
    {
        addSink("input", "Input");
        _output = addSource("output", "output");

        addOption("output file","name of the lhco output file",_outFileName,pxl::OptionDescription::USAGE_FILE_SAVE);

        addOption("event view","name of the event view used to build the lhco event",_inputEventViewName);
        addOption("jet name","name of jets",_jetName);
        addOption("bjet name","name of bjets",_bjetName);
        addOption("electron name","name of jets",_electronName);
        addOption("muon name","name of jets",_muonName);
        addOption("photon name","name of jets",_metName);
        addOption("met name","name of jets",_photonName);

    }

    ~LHCOConverter()
    {
    }

    // every Module needs a unique type
    static const std::string &getStaticType()
    {
        static std::string type ("LHCOConverter");
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
        getOption("output file",_outFileName);

        getOption("event view",_inputEventViewName);
        getOption("jet name",_jetName);
        getOption("bjet name",_bjetName);
        getOption("electron name",_electronName);
        getOption("muon name",_muonName);
        getOption("photon name",_metName);
        getOption("met name",_photonName);

        ofs = new std::ofstream(_outFileName, std::ofstream::out);
    }

    void endJob()
    {
        ofs->close();
    }

    bool analyse(pxl::Sink *sink) throw (std::runtime_error)
    {
        try
        {
            pxl::Event *event  = dynamic_cast<pxl::Event *> (sink->get());
            if (event)
            {
                int numJets=0;
                std::vector<pxl::EventView*> eventViews;
                event->getObjectsOfType(eventViews);
                for (unsigned ieventView=0; ieventView<eventViews.size();++ieventView)
                {
                    pxl::EventView* eventView = eventViews[ieventView];
                    if (eventView->getName()==_inputEventViewName)
                    {
                        LHCOEvent lhcoEvent;
                        lhcoEvent.setEventID(event->getUserRecord("Event number").toUInt32());
                        std::vector<pxl::Particle*> particles;
                        eventView->getObjectsOfType(particles);
                        for (unsigned iparticle=0; iparticle<particles.size();++iparticle)
                        {
                            pxl::Particle* particle = particles[iparticle];
                            if (particle->getName()==_jetName)
                            {
                                lhcoEvent.addJet(
                                    particle->getEta(),//eta=0.0,
                                    particle->getPhi(),//phi=0.0,
                                    particle->getPt(),//pt=0.0,
                                    particle->getMass()//mass=0.0,
                                    //ntrk=0.0,
                                    //dummy1=0.0,
                                    //dummy2=0.0
                                );
                            } else if (particle->getName()==_bjetName) {
                                lhcoEvent.addBJet(
                                    particle->getEta(),//eta=0.0,
                                    particle->getPhi(),//phi=0.0,
                                    particle->getPt(),//pt=0.0,
                                    particle->getMass()//mass=0.0,
                                    //ntrk=0.0,
                                    //dummy1=0.0,
                                    //dummy2=0.0
                                );
                            } else if (particle->getName()==_electronName) {
                                lhcoEvent.addElectron(
                                    particle->getEta(),//eta=0.0,
                                    particle->getPhi(),//phi=0.0,
                                    particle->getPt(),//pt=0.0,
                                    (int)particle->getCharge()//int charge=-1,
                                    //float dummy1=0.0,
                                    //float dummy2=0.0
                                );
                            } else if (particle->getName()==_muonName) {
                                lhcoEvent.addMuon(
                                    particle->getEta(),//eta=0.0,
                                    particle->getPhi(),//phi=0.0,
                                    particle->getPt(),//pt=0.0,
                                    (int)particle->getCharge()//int charge=-1,
                                    //float dummy1=0.0,
                                    //float dummy2=0.0
                                );
                            } else if (particle->getName()==_photonName) {
                                lhcoEvent.addPhoton(
                                    particle->getEta(),//eta=0.0,
                                    particle->getPhi(),//phi=0.0,
                                    particle->getPt()//pt=0.0,
                                    //float hademfraction=0.0,
                                    //float dummy1=0.0,
                                    //float dummy2=0.0
                                );
                            } else if (particle->getName()==_metName) {
                                lhcoEvent.setMET(
                                    particle->getPhi(),//phi=0.0,
                                    particle->getPt()//pt=0.0,
                                    //float dummy1=0.0,
                                    //float dummy2=0.0
                                );
                            }
                        }
                        lhcoEvent.writeEvent(*ofs);
                    }
                }

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

PXL_MODULE_INIT(LHCOConverter)
PXL_PLUGIN_INIT
