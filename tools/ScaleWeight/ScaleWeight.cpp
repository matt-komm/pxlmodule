#include "pxl/hep.hh"
#include "pxl/core.hh"
#include "pxl/core/macros.hh"
#include "pxl/core/PluginManager.hh"
#include "pxl/modules/Module.hh"
#include "pxl/modules/ModuleFactory.hh"

#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <stdio.h>

#include "LHAPDF/LHAPDF.h"

static pxl::Logger logger("ScaleWeight");

class ScaleWeight : public pxl::Module
{
    private:
    pxl::Source* _output;
    pxl::Source* _error;
    pxl::Sink* _input;

    std::string _scaleEventView;

    double _factorization;

    public:
    ScaleWeight() :
        Module(),
        _scaleEventView("Generated"),
        _factorization(1.0)

    {
        _input = addSink("input", "input");
        _output = addSource("output", "output");
        _error = addSource("error", "error");

        addOption("generator event view","",_scaleEventView);
        addOption("factorization","multiplies the qscale by this number to get the factorization scale",_factorization);
    }

    ~ScaleWeight()
    {
    }

    // every Module needs a unique type
    static const std::string &getStaticType()
    {
        static std::string type ("ScaleWeight");
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
        getOption("generator event view",_scaleEventView);
        getOption("factorization",_factorization);

        LHAPDF::initPDFSet("cteq6ll.LHpdf");
        LHAPDF::usePDFMember(1,0);
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
                double qscale = -1;
                double x1 = -1;
                double x2 = -1;
                double id1 = -1000;
                double id2 = -1000;

                std::vector<pxl::EventView*> eventViews;
                event->getObjectsOfType(eventViews);
                for (unsigned ieventView=0; ieventView<eventViews.size();++ieventView)
                {
                    pxl::EventView* eventView = eventViews[ieventView];
                    if (eventView->getName()==_scaleEventView)
                    {
                        eventView->getUserRecord("qscale",qscale);
                        eventView->getUserRecord("x1",x1);
                        eventView->getUserRecord("x2",x2);
                        eventView->getUserRecord("id1",id1);
                        eventView->getUserRecord("id2",id2);
                        if (qscale<0)
                        {
                            logger(pxl::LOG_LEVEL_ERROR , "qscale not properly set");
                            _error->setTargets(event);
                            return _error->processTargets();
                        }
                        if (x1<0 || x2<0)
                        {
                            logger(pxl::LOG_LEVEL_ERROR , "x1,x2 not properly set");
                            _error->setTargets(event);
                            return _error->processTargets();
                        }

                        if (id1<-10 || id2<-10)
                        {
                            logger(pxl::LOG_LEVEL_ERROR , "id1,id2 not properly set");
                            _error->setTargets(event);
                            return _error->processTargets();
                        }
                        double alpha = LHAPDF::alphasPDF(qscale);
                        double alpha_down = LHAPDF::alphasPDF(qscale*0.5);
                        double alpha_up = LHAPDF::alphasPDF(qscale*2.);
                        double factorization_scale = qscale*_factorization;

                        eventView->setUserRecord("alpha_s",alpha);
                        eventView->setUserRecord("alpha_s_down",alpha_down);
                        eventView->setUserRecord("alpha_s_up",alpha_up);

                        double pdf1 = LHAPDF::xfx(1,  x1, factorization_scale,  id1);
                        double pdf2 = LHAPDF::xfx(1,  x2, factorization_scale,  id2);
                        double pdf1_up = LHAPDF::xfx(1,  x1, factorization_scale*2.,  id1);
                        double pdf2_up = LHAPDF::xfx(1,  x2, factorization_scale*2.,  id2);
                        double pdf1_down = LHAPDF::xfx(1,  x1, factorization_scale*.5,  id1);
                        double pdf2_down = LHAPDF::xfx(1,  x2, factorization_scale*.5,  id2);

                        eventView->setUserRecord("pdf1",pdf1);
                        eventView->setUserRecord("pdf2",pdf2);
                        eventView->setUserRecord("pdf1_up",pdf1_up);
                        eventView->setUserRecord("pdf2_up",pdf2_up);
                        eventView->setUserRecord("pdf1_down",pdf1_down);
                        eventView->setUserRecord("pdf2_down",pdf2_down);

                        break;
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

PXL_MODULE_INIT(ScaleWeight)
PXL_PLUGIN_INIT
