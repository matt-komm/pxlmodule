#include "pxl/hep.hh"
#include "pxl/core.hh"
#include "pxl/core/macros.hh"
#include "pxl/core/PluginManager.hh"
#include "pxl/modules/Module.hh"
#include "pxl/modules/ModuleFactory.hh"

static pxl::Logger logger("DefaultMuonSelection");

class DefaultMuonSelection : public pxl::Module
{
    private:
    pxl::Source* _sourceSelected_0muons;
    pxl::Source* _sourceSelected_1muons;
    pxl::Source* _sourceSelected_2muons;
    pxl::Source* _sourceSelected_othermuons;

    pxl::Source* _sourceVeto;

    std::string _inputMuonName;
    std::string _inputEventViewName;
    std::string _tightMuonName;
    std::string _looseMuonName;
    bool _cleanEvent;

    public:
    DefaultMuonSelection() :
        Module(),
        _inputMuonName("Muon"),
        _inputEventViewName("Reconstructed"),
        _tightMuonName("TightMuon"),
        _looseMuonName("LooseMuon"),
        _cleanEvent(true)
    {
        addSink("input", "Input");
        _sourceVeto = addSource("veto", "veto");
        _sourceSelected_othermuons = addSource(">2 muons", ">2 muons");
        _sourceSelected_2muons = addSource("2 muons", "2 muons");
        _sourceSelected_1muons = addSource("1 muons", "1 muons");
        _sourceSelected_0muons = addSource("0 muons", "0 muons");


        addOption("event view","name of the event view where muons are selected",_inputEventViewName);
        addOption("input muon name","name of particles to consider for selection",_inputMuonName);
        addOption("name of selected tight muons","",_tightMuonName);
        addOption("name of selected loose muons","",_looseMuonName);
        addOption("clean event","this option will clean the event of all muons falling tight or loose criteria",_cleanEvent);
    }

    ~DefaultMuonSelection()
    {
    }

    // every Module needs a unique type
    static const std::string &getStaticType()
    {
        static std::string type ("DefaultMuonSelection");
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
        getOption("input muon name",_inputMuonName);
        getOption("name of selected tight muons",_tightMuonName);
        getOption("name of selected loose muons",_looseMuonName);
        getOption("clean event",_cleanEvent);
    }

    bool passTightCriteria(pxl::Particle* particle)
    {
        if (not (particle->getPt()>26.0)) {
            return false;
        }
        if (not (fabs(particle->getEta())<2.1)) {
            return false;
        }
        if (not particle->getUserRecord("isPFMuon").toBool()) {
            return false;
        }
        if (not particle->getUserRecord("isGlobalMuon").toBool()) {
            return false;
        }
        if (not (particle->getUserRecord("chi2").toFloat()<10.0)) {
            return false;
        }
        if (not (particle->getUserRecord("numberOfValidMuonHits").toInt32()>0)) {
            return false;
        }
        if (not (particle->getUserRecord("numberOfMatchedStations").toInt32()>1)) {
            return false;
        }
        if (not (fabs(particle->getUserRecord("dxy").toFloat())<0.2)) {
            return false;
        }
        if (not (fabs(particle->getUserRecord("dz").toFloat())<0.5)) {
            return false;
        }
        if (not (particle->getUserRecord("numberOfValidPixelHits").toInt32()>0)) {
            return false;
        }
        if (not (particle->getUserRecord("trackerLayersWithMeasurement").toInt32()>5)) {
            return false;
        }
        if (not (particle->getUserRecord("relIso").toFloat()<0.12)) {
            return false;
        }
        return true;
    }

    bool passLooseCriteria(pxl::Particle* particle)
    {
        if (not (particle->getPt()>10.0)) {
            return false;
        }
        if (not (fabs(particle->getEta())<2.5)) {
            return false;
        }
        if (not (particle->getPt()>10.0)) {
            return false;
        }
        if (not (particle->getUserRecord("relIso").toFloat()<0.2)) {
            return false;
        }
        if (not (fabs(particle->getUserRecord("dxy").toFloat())<0.2)) {
            return false;
        }
        if (not (fabs(particle->getUserRecord("dz").toFloat())<0.5)) {
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
                int numTightMuons=0;
                int numLooseMuons=0;
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

                            if (particle->getName()==_inputMuonName)
                            {
                                if (passTightCriteria(particle))
                                {
                                    particle->setName(_tightMuonName);
                                    ++numTightMuons;
                                } else if (passLooseCriteria(particle)) {
                                    particle->setName(_looseMuonName);
                                    ++numLooseMuons;
                                } else if (_cleanEvent) {
                                    eventView->removeObject(particle);
                                }

                            }
                        }

                    }
                }
                if (numLooseMuons>0)
                {
                    _sourceVeto->setTargets(event);
                    return _sourceVeto->processTargets();
                }
                pxl::Source* sourceSelected=0;
                switch (numTightMuons)
                {
                    case 0:
                        sourceSelected = _sourceSelected_0muons;
                        break;
                    case 1:
                        sourceSelected = _sourceSelected_1muons;
                        break;
                    case 2:
                        sourceSelected = _sourceSelected_2muons;
                        break;
                    default:
                        sourceSelected = _sourceSelected_othermuons;
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

PXL_MODULE_INIT(DefaultMuonSelection)
PXL_PLUGIN_INIT
