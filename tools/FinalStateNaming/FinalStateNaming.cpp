#include "pxl/hep.hh"
#include "pxl/core.hh"
#include "pxl/core/macros.hh"
#include "pxl/core/PluginManager.hh"
#include "pxl/modules/Module.hh"
#include "pxl/modules/ModuleFactory.hh"

static pxl::Logger logger("FinalStateNaming");

class FinalStateNaming : public pxl::Module
{
    private:
    pxl::Source* _output_tHq;
    pxl::Sink* _input_tHq;

    pxl::Source* _output_tt;
    pxl::Sink* _input_tt;

    pxl::Source* _output_fail;

    std::string _inputEventViewName;

    public:
    FinalStateNaming() :
        Module(),
        _inputEventViewName("Generated")
    {
        _input_tHq = addSink("in tHq", "in tHq");
        _output_tHq = addSource("out tHq", "out tHq");

        _input_tt = addSink("in tt", "in tt");
        _output_tt = addSource("out tt", "out tt");

        _output_fail = addSource("out fail", "out fail");


        addOption("generator event view","generator event view",_inputEventViewName);

    }

    ~FinalStateNaming()
    {
    }

    // every Module needs a unique type
    static const std::string &getStaticType()
    {
        static std::string type ("FinalStateNaming");
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
        getOption("generator event view",_inputEventViewName);
    }

    void endJob()
    {
    }

    bool name_simple(std::vector<pxl::Particle*> particles)
    {
        //name FS only by id, e.g. id=1...5 -> jet
    }

    bool name_tHq(std::vector<pxl::Particle*> particles)
    {
        bool foundLepton=false;
        for (unsigned iparticle=0; iparticle<particles.size(); ++iparticle)
        {
            pxl::Particle* particle = particles[iparticle];
            if (abs(particle->getPdgNumber())==11 || abs(particle->getPdgNumber())==13)
            {
                if (abs(dynamic_cast<pxl::Particle*>(particle->getMother()->getMother())->getPdgNumber())==6)
                {
                    foundLepton=true;
                    particle->setName("lepton");
                }
            }
            else if (abs(particle->getPdgNumber())==11 || abs(particle->getPdgNumber())==15)
            {
                if (abs(dynamic_cast<pxl::Particle*>(particle->getMother())->getPdgNumber())==6)
                {
                    //foundLepton=true;
                    //----------------------------------------------------------------------------------------------------------------------------------------------
                    //TODO: handle this case, match the lepton to the gen lepton and not to the tau -> need to keep the information in gen particle pruner
                    //----------------------------------------------------------------------------------------------------------------------------------------------
                    particle->setName("tau_lepton");
                }
            }
            else if (abs(particle->getPdgNumber())==12 || abs(particle->getPdgNumber())==14)
            {
                if (abs(dynamic_cast<pxl::Particle*>(particle->getMother()->getMother())->getPdgNumber())==6)
                {
                    particle->setName("neutrino");
                }
            }
            else if (abs(particle->getPdgNumber())==5)
            {
                if (abs(dynamic_cast<pxl::Particle*>(particle->getMother())->getPdgNumber())==6)
                {
                    particle->setName("b_top");
                }
                else if (abs(dynamic_cast<pxl::Particle*>(particle->getMother())->getPdgNumber())==25)
                {
                    particle->setName("b_higgs");
                }
                else if (particle->getDaughterRelations().size()==0)
                {
                    particle->setName("bspectator");
                }
            }
            else if (abs(particle->getPdgNumber())==6)
            {
                particle->setName("top");
            }
            else if (abs(particle->getPdgNumber())==6)
            {
                particle->setName("top");
            }
            else if (abs(particle->getPdgNumber())<5)
            {
                if (particle->getDaughterRelations().size()==0)
                {
                    particle->setName("spectator");
                }
            }
            else if (abs(particle->getPdgNumber())==5)
            {

            }
        }
        return foundLepton;
    }

    bool name_tt(std::vector<pxl::Particle*>& particles)
    {
        return false;
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

                        std::vector<pxl::Particle*> particles;
                        eventView->getObjectsOfType(particles);
                        if (sink==_input_tHq)
                        {
                            result=name_tHq(particles);
                        }
                        else if (sink==_input_tt)
                        {
                            result=name_tt(particles);
                        }
                    }
                }
                if (!result)
                {
                    _output_fail->setTargets(event);
                    return _output_fail->processTargets();
                }
                else if (sink==_input_tHq)
                {
                    _output_tHq->setTargets(event);
                    return _output_tHq->processTargets();
                }
                else if (sink==_input_tt)
                {
                    _output_tt->setTargets(event);
                    return _output_tt->processTargets();
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

PXL_MODULE_INIT(FinalStateNaming)
PXL_PLUGIN_INIT
