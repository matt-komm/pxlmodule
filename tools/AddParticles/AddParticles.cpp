#include "pxl/hep.hh"
#include "pxl/core.hh"
#include "pxl/core/macros.hh"
#include "pxl/core/PluginManager.hh"
#include "pxl/modules/Module.hh"
#include "pxl/modules/ModuleFactory.hh"

static pxl::Logger logger("AddParticles");

class AddParticles : public pxl::Module
{
    private:
    pxl::Source* _output;
    pxl::Sink* _input;

    std::string _inputEventViewName;
    
    std::string _particleName;
    int64_t _num;
    int64_t _pdgId;
    int64_t _charge;
    double _eta;
    double _pt;
    double _phi;

    public:
    AddParticles() :
        Module(),
        _inputEventViewName("Generated")
    {
        _input = addSink("input", "input");
        _output = addSource("output", "output");


        addOption("event view","event view",_inputEventViewName);
        
        addOption("num","num",_num);
        addOption("particleName","particleName",_particleName);
        addOption("pdgID","pdgID",_pdgId);
        addOption("charge","charge",_charge);
        addOption("eta","eta",_eta);
        addOption("pt","pt",_pt);
        addOption("phi","phi",_phi);
        

    }

    ~AddParticles()
    {
    }

    // every Module needs a unique type
    static const std::string &getStaticType()
    {
        static std::string type ("AddParticles");
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
        
        getOption("num",_num);
        getOption("particleName",_particleName);
        getOption("pdgID",_pdgId);
        getOption("charge",_charge);
        getOption("pt",_pt);
        getOption("eta",_eta);
        getOption("phi",_phi);
    }

    void endJob()
    {
    }
    
    bool analyse(pxl::Sink *sink) throw (std::runtime_error)
    {
        try
        {
            pxl::Event *event  = dynamic_cast<pxl::Event *> (sink->get());
            if (event)
            {
                bool result=false;

                std::vector<pxl::EventView*> eventViews;
                event->getObjectsOfType(eventViews);
                for (unsigned ieventView=0; ieventView<eventViews.size();++ieventView)
                {
                    pxl::EventView* eventView = eventViews[ieventView];
                    if (eventView->getName()==_inputEventViewName)
                    {
                        for (unsigned iparticle=0;iparticle<_num;++iparticle)
                        {
                            pxl::Particle* particle = eventView->create<pxl::Particle>();
                            particle->setName(_particleName);
                            particle->setCharge(_charge);
                            particle->getVector().setRThetaPhi(_pt,_eta,_phi);
                        }
                        
                    }
                }
                
            }
            _output->setTargets(event);
            return _output->processTargets();
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

PXL_MODULE_INIT(AddParticles)
PXL_PLUGIN_INIT
