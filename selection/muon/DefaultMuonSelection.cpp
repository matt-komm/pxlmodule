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
        pxl::Source* _Source_selected;
        pxl::Source* _Source_veto;

        int64_t _minEventNumber;

public:
        DefaultMuonSelection() : Module(), _minEventNumber(1)
        {
                addOption("minEventNumber", "select only events which have a UserRecord EventNumber > minEventNumber", int64_t(0));

                addSink("input", "Input");
                addSource("veto", "veto");
                addSource("selected", "Selected");
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
                getOption("minEventNumber", _minEventNumber);

                //print the option:
                        //first convert the int into a string
                        std::stringstream str_minEventNumber;
                        str_minEventNumber<<_minEventNumber;

                        //then use the pxl:Logger
                        logger(pxl::LOG_LEVEL_NONE, "Selecting only events which have an UserRecord 'EventNumber' > " + str_minEventNumber.str());

                _Source_selected = getSource("selected");
                _Source_veto = getSource("veto");
        }

        bool analyse(pxl::Sink *sink) throw (std::runtime_error)
        {
                try
                {
                        pxl::Event *event  = dynamic_cast<pxl::Event *> (sink->get());

                        if (event)
                        {
                                if (event->hasUserRecord("Event number"))
                                {
                                        unsigned int eventNumber = (unsigned int)(event->getUserRecord("Event number"));

                                        if (eventNumber > _minEventNumber)
                                        {
                                                _Source_selected->setTargets(event);
                                                return _Source_selected->processTargets();
                                        }//else veto this event
                                }//else veto this event

                                _Source_veto->setTargets(event);
                                return _Source_veto->processTargets();
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

                logger(pxl::LOG_LEVEL_NONE, "Analysed event is not an pxl::Event !");
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
