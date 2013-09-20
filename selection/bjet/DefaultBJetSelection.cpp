#include "pxl/hep.hh"
#include "pxl/core.hh"
#include "pxl/core/macros.hh"
#include "pxl/core/PluginManager.hh"
#include "pxl/modules/Module.hh"
#include "pxl/modules/ModuleFactory.hh"

#include <vector>
#include <string>

#include "BTaggerAlgorithm.h"

static pxl::Logger logger("DefaultBJetSelection");

class DefaultBJetSelection : public pxl::Module
{
    private:
    pxl::Source* _sourceSelected_0bjets;
    pxl::Source* _sourceSelected_1bjets;
    pxl::Source* _sourceSelected_2bjets;
    pxl::Source* _sourceSelected_3bjets;
    pxl::Source* _sourceSelected_4bjets;
    pxl::Source* _sourceSelected_5bjets;
    pxl::Source* _sourceSelected_6bjets;
    pxl::Source* _sourceSelected_otherbjets;
    pxl::Source* _sourceVeto;

    std::string _inputJetName;
    std::string _inputEventViewName;
    std::string _selectedJetName;
    
    std::string _btaggerAlgorithmName;

    BTaggerAlgorithm* _btaggerAlgorithm;

    public:
    DefaultBJetSelection() :
        Module(),
        _inputJetName("SelectedJet"),
        _inputEventViewName("Reconstructed"),
        _selectedJetName("SelectedBJet"),
        _btaggerAlgorithmName("CSVT")
    {
        addSink("input", "Input");
        _sourceSelected_otherbjets = addSource(">6 bjets", ">6 bjets");
        _sourceSelected_6bjets = addSource("6 bjets", "6 bjets");
        _sourceSelected_5bjets = addSource("5 bjets", "5 bjets");
        _sourceSelected_4bjets = addSource("4 bjets", "4 bjets");
        _sourceSelected_3bjets = addSource("3 bjets", "3 bjets");
        _sourceSelected_2bjets = addSource("2 bjets", "2 bjets");
        _sourceSelected_1bjets = addSource("1 bjets", "1 bjets");
        _sourceSelected_0bjets = addSource("0 bjets", "0 bjets");

        addOption("event view","name of the event view where jets are selected",_inputEventViewName);
        addOption("input jet name","name of particles to consider for selection",_inputJetName);
        addOption("name of selected bjets","",_selectedJetName);
        
        addOption("algorithm","used btagging algorithm",_btaggerAlgorithmName);
    }

    ~DefaultBJetSelection()
    {
    }

    // every Module needs a unique type
    static const std::string &getStaticType()
    {
        static std::string type ("DefaultBJetSelection");
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
        getOption("name of selected bjets",_selectedJetName);
        getOption("algorithm",_btaggerAlgorithmName);

        if (BTaggerDataBase.find(_btaggerAlgorithmName)!=BTaggerDataBase.end())
        {
            _btaggerAlgorithm = BTaggerDataBase.at(_btaggerAlgorithmName);
        } else {
            std::string message("");
            message+="specified btagging algorithm '";
            message+=_btaggerAlgorithmName;
            message+="' not in database";
            throw std::runtime_error(message);
        }
    }

    bool passSelection(pxl::Particle* particle) throw (std::runtime_error)
    {
        if (_btaggerAlgorithm->testBtagged(particle))
        {
            //-------------------------------------------------------------------------TODO: apply SF
            return true;
        }
        return false;
    }

    bool analyse(pxl::Sink *sink) throw (std::runtime_error)
    {
        try
        {
            pxl::Event *event  = dynamic_cast<pxl::Event *> (sink->get());
            if (event)
            {
                int numBJets=0;
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
                                    ++numBJets;
                                }
                            }
                        }
                        eventView->setUserRecord("numBJets",numBJets);
                    }
                }
                pxl::Source* sourceSelected=0;
                switch (numBJets)
                {
                    case 0:
                        sourceSelected = _sourceSelected_0bjets;
                        break;
                    case 1:
                        sourceSelected = _sourceSelected_1bjets;
                        break;
                    case 2:
                        sourceSelected = _sourceSelected_2bjets;
                        break;
                    case 3:
                        sourceSelected = _sourceSelected_3bjets;
                        break;
                    case 4:
                        sourceSelected = _sourceSelected_4bjets;
                        break;
                    case 5:
                        sourceSelected = _sourceSelected_5bjets;
                        break;
                    case 6:
                        sourceSelected = _sourceSelected_6bjets;
                        break;
                    default:
                        sourceSelected = _sourceSelected_otherbjets;
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

PXL_MODULE_INIT(DefaultBJetSelection)
PXL_PLUGIN_INIT
