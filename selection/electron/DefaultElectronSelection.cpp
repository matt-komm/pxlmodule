#include "pxl/hep.hh"
#include "pxl/core.hh"
#include "pxl/core/macros.hh"
#include "pxl/core/PluginManager.hh"
#include "pxl/modules/Module.hh"
#include "pxl/modules/ModuleFactory.hh"

static pxl::Logger logger("DefaultElectronSelection");

class DefaultElectronSelection : public pxl::Module
{
    private:
    pxl::Source* _sourceSelected;
    pxl::Source* _sourceVeto;

    std::string _inputElectronName;
    std::string _inputEventViewName;
    std::string _tightElectronName;
    std::string _looseElectronName;
    bool _cleanEvent;
    int64_t _maxTightElectrons;
    int64_t _minTightElectrons;

    public:
    DefaultElectronSelection() :
        Module(),
        _inputElectronName("Electron"),
        _inputEventViewName("Reconstructed"),
        _tightElectronName("TightElectron"),
        _looseElectronName("LooseElectron"),
        _cleanEvent(true),
        _maxTightElectrons(1),
        _minTightElectrons(1)
    {
        addSink("input", "Input");
        _sourceVeto = addSource("veto", "veto");
        _sourceSelected = addSource("selected", "Selected");

        addOption("event view","name of the event view where electrons are selected",_inputEventViewName);
        addOption("input electron name","name of particles to consider for selection",_inputElectronName);
        addOption("name of selected tight electrons","",_tightElectronName);
        addOption("name of selected loose electrons","",_looseElectronName);
        addOption("clean event","this option will clean the event of all electrons falling tight or loose criteria",_cleanEvent);
        addOption("max tight electrons","veto events which have more tight electrons",_maxTightElectrons);
        addOption("min tight electrons","veto events which have less tight electrons",_minTightElectrons);

    }

    ~DefaultElectronSelection()
    {
    }

    // every Module needs a unique type
    static const std::string &getStaticType()
    {
        static std::string type ("DefaultElectronSelection");
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
        getOption("input electron name",_inputElectronName);
        getOption("name of selected tight electrons",_tightElectronName);
        getOption("name of selected loose electrons",_looseElectronName);
        getOption("clean event",_cleanEvent);
        getOption("max tight electrons",_maxTightElectrons);
        getOption("min tight electrons",_minTightElectrons);
    }

    bool passTriggerPreselection(pxl::Particle* particle)
    {
        if (fabs(particle->getUserRecord("superClusterEta").toFloat())<1.479)
        {
            if (not (particle->getUserRecord("sigmaIetaIeta").toFloat()<0.014))
            {
                return false;
            }
            if (not (particle->getUserRecord("hadronicOverEm").toFloat()<0.15))
            {
                return false;
            }
//---------------------------------------------------------------------------------------------------------------------------
//remove if else branch when dr03TkSumP changed to dr03TkSumPt with next reprocessing
            if (particle->hasUserRecord("dr03TkSumP")) {
                if (not (particle->getUserRecord("dr03TkSumP").toFloat()/particle->getPt()<0.2))
                {
                    return false;
                }
            } else {
                if (not (particle->getUserRecord("dr03TkSumPt").toFloat()/particle->getPt()<0.2))
                {
                    return false;
                }
            }
//---------------------------------------------------------------------------------------------------------------------------
            if (not (particle->getUserRecord("dr03EcalRecHitSumEt").toFloat()/particle->getPt()<0.2))
            {
                return false;
            }
            if (not (particle->getUserRecord("dr03HcalTowerSumEt").toFloat()/particle->getPt()<0.2))
            {
                return false;
            }
            if (not (particle->getUserRecord("numberOfLostHits").toInt32()==0))
            {
                return false;
            }
            return true;
        } else {
            if (not (particle->getUserRecord("sigmaIetaIeta").toFloat()<0.035))
            {
                return false;
            }
            if (not (particle->getUserRecord("hadronicOverEm").toFloat()<0.10))
            {
                return false;
            }
//---------------------------------------------------------------------------------------------------------------------------
//remove if else branch when dr03TkSumP changed to dr03TkSumPt with next reprocessing
            if (particle->hasUserRecord("dr03TkSumP")) {
                if (not (particle->getUserRecord("dr03TkSumP").toFloat()/particle->getPt()<0.2))
                {
                    return false;
                }
            } else {
                if (not (particle->getUserRecord("dr03TkSumPt").toFloat()/particle->getPt()<0.2))
                {
                    return false;
                }
            }
//---------------------------------------------------------------------------------------------------------------------------
            if (not (particle->getUserRecord("dr03EcalRecHitSumEt").toFloat()/particle->getPt()<0.2))
            {
                return false;
            }
            if (not (particle->getUserRecord("dr03HcalTowerSumEt").toFloat()/particle->getPt()<0.2))
            {
                return false;
            }
            if (not (particle->getUserRecord("numberOfLostHits").toInt32()==0))
            {
                return false;
            }
            return true;
        }
    }

    bool passTightCriteria(pxl::Particle* particle)
    {
        if (not (particle->getPt()>30.0)) {
            return false;
        }
        if (not (fabs(particle->getEta())>2.5)) {
            return false;
        }
        if (not (particle->getUserRecord("isInEB-EE").toBool())) {
            return false;
        }
        if (not (particle->getUserRecord("passConversionVeto").toBool())) {
            return false;
        }
        if (not (particle->getUserRecord("mva").toFloat()>0.5)) {
            return false;
        }
        if (not passTriggerPreselection(particle)) {
            return false;
        }
        if (not (particle->getUserRecord("numberOfHits").toInt32()<=0)) {
            return false;
        }
        if (not (particle->getUserRecord("relIso").toFloat()<0.1)) {
            return false;
        }
        return true;
    }

    bool passLooseCriteria(pxl::Particle* particle)
    {
        if (not (particle->getPt()>10.0)) {
            return false;
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
                int numTightElectrons=0;
                int numLooseElectrons=0;
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

                            if (particle->getName()==_inputElectronName)
                            {
                                if (passTightCriteria(particle))
                                {
                                    particle->setName(_tightElectronName);
                                    ++numTightElectrons;
                                } else if (passLooseCriteria(particle)) {
                                    particle->setName(_looseElectronName);
                                    ++numLooseElectrons;
                                } else if (_cleanEvent) {
                                    eventView->removeObject(particle);
                                }

                            }
                        }

                    }
                }
                if (numLooseElectrons>0)
                {
                    _sourceVeto->setTargets(event);
                    return _sourceVeto->processTargets();
                }

                if (numTightElectrons<=_maxTightElectrons && numTightElectrons>=_minTightElectrons)
                {
                    _sourceSelected->setTargets(event);
                    return _sourceSelected->processTargets();
                } else {
                    _sourceVeto->setTargets(event);
                    return _sourceVeto->processTargets();
                }
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

PXL_MODULE_INIT(DefaultElectronSelection)
PXL_PLUGIN_INIT
