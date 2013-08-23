#include "pxl/hep.hh"
#include "pxl/core.hh"
#include "pxl/core/macros.hh"
#include "pxl/core/PluginManager.hh"
#include "pxl/modules/Module.hh"
#include "pxl/modules/ModuleFactory.hh"

static pxl::Logger logger("DefaultJetSelection");

class DefaultJetSelection : public pxl::Module
{
    private:
    pxl::Source* _sourceSelected_0jets;
    pxl::Source* _sourceSelected_1jets;
    pxl::Source* _sourceSelected_2jets;
    pxl::Source* _sourceSelected_3jets;
    pxl::Source* _sourceSelected_4jets;
    pxl::Source* _sourceSelected_5jets;
    pxl::Source* _sourceSelected_6jets;
    pxl::Source* _sourceSelected_otherjets;
    pxl::Source* _sourceVeto;

    std::string _inputJetName;
    std::string _inputEventViewName;
    std::string _selectedJetName;
    bool _cleanEvent;
    int64_t _maxJets;
    int64_t _minJets;

    public:
    DefaultJetSelection() :
        Module(),
        _inputJetName("Jet"),
        _inputEventViewName("Reconstructed"),
        _selectedJetName("SelectedJet"),
        _cleanEvent(true)
    {
        addSink("input", "Input");
        _sourceSelected_otherjets = addSource(">6 jets", ">6 jets");
        _sourceSelected_6jets = addSource("6 jets", "6 jets");
        _sourceSelected_5jets = addSource("5 jets", "5 jets");
        _sourceSelected_4jets = addSource("4 jets", "4 jets");
        _sourceSelected_3jets = addSource("3 jets", "3 jets");
        _sourceSelected_2jets = addSource("2 jets", "2 jets");
        _sourceSelected_1jets = addSource("1 jets", "1 jets");
        _sourceSelected_0jets = addSource("0 jets", "0 jets");

        addOption("event view","name of the event view where jets are selected",_inputEventViewName);
        addOption("input jet name","name of particles to consider for selection",_inputJetName);
        addOption("name of selected jets","",_selectedJetName);
        addOption("clean event","this option will clean the event of all jets falling selection",_cleanEvent);
    }

    ~DefaultJetSelection()
    {
    }

    // every Module needs a unique type
    static const std::string &getStaticType()
    {
        static std::string type ("DefaultJetSelection");
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
        getOption("event view",_inputEventViewName);
        getOption("input jet name",_inputJetName);
        getOption("name of selected jets",_selectedJetName);
        getOption("clean event",_cleanEvent);
    }

    bool passSelection(pxl::Particle* particle)
    {
        if (not (particle->getPt()>30.0)) {
            return false;
        }
        if (not (particle->getUserRecord("numberOfDaughters").toInt32()>1)) {
            return false;
        }
        //-----------------------------------------------------------------------
        //double nhf = 0;
        //-----------------------------------------------------------------------
        //NEF
        if (not (particle->getUserRecord("neutralEmEnergyFraction").toFloat()<0.99)) {
            return false;
        }
        if (fabs(particle->getEta())<2.4) {
            //CEF
            if (not (particle->getUserRecord("chargedEmEnergyFraction").toFloat()<0.99)) {
                return false;
            }
            //CHF
            if (not (particle->getUserRecord("chargedHadronEnergyFraction").toFloat()>0.0)) {
                return false;
            }
            //NCH
            if (not (particle->getUserRecord("chargedMultiplicity").toInt32()>0)) {
                return false;
            }
        }
        return true;
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
                        std::vector<pxl::Particle*> particles;
                        eventView->getObjectsOfType(particles);
                        for (unsigned iparticle=0; iparticle<particles.size();++iparticle)
                        {
                            pxl::Particle* particle = particles[iparticle];
                            if (particle->getName()==_inputJetName)
                            {
                                if (passSelection(particle))
                                {
                                    particle->setName(_selectedJetName);
                                    ++numJets;
                                } else if  (_cleanEvent) {
                                    eventView->removeObject(particle);
                                }
                            }
                        }
                    }
                }
                pxl::Source* sourceSelected=0;
                switch (numJets)
                {
                    case 0:
                        sourceSelected = _sourceSelected_0jets;
                        break;
                    case 1:
                        sourceSelected = _sourceSelected_1jets;
                        break;
                    case 2:
                        sourceSelected = _sourceSelected_2jets;
                        break;
                    case 3:
                        sourceSelected = _sourceSelected_3jets;
                        break;
                    case 4:
                        sourceSelected = _sourceSelected_4jets;
                        break;
                    case 5:
                        sourceSelected = _sourceSelected_5jets;
                        break;
                    case 6:
                        sourceSelected = _sourceSelected_6jets;
                        break;
                    default:
                        sourceSelected = _sourceSelected_otherjets;
                }
                sourceSelected->setTargets(event);
                return sourceSelected->processTargets();
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

PXL_MODULE_INIT(DefaultJetSelection)
PXL_PLUGIN_INIT
