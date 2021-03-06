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

static pxl::Logger logger("Counting");

class Counting : public pxl::Module
{
    private:
    pxl::Source* _output;
    pxl::Sink* _input;

    std::string _name;

    double _seconds;
    int64_t _events;
    int64_t _eventDiff;

    int64_t _count;
    clock_t timeDiff;
    clock_t totalTime;

    bool _onlyFinal;

    public:
    Counting() :
        Module(),
        _seconds(30.0),
        _events(10000),
        _count(0),
        _name(""),
        _onlyFinal(false)

    {
        _input = addSink("input", "input");
        _output = addSource("output", "output");

        addOption("name","name of the counter",_name);
        addOption("seconds","report after the given number of seconds",_seconds);
        addOption("events","report after the given number of events",_events);
        addOption("only final","report only at the end",_onlyFinal);
    }

    ~Counting()
    {
    }

    // every Module needs a unique type
    static const std::string &getStaticType()
    {
        static std::string type ("Counting");
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
        _eventDiff=0;
        timeDiff = clock();
        totalTime = clock();
        getOption("name",_name);
        getOption("seconds",_seconds);
        getOption("events",_events);
        getOption("only final",_onlyFinal);
    }

    void convertTime(char* buf, float seconds)
    {
        int h = ((int)(seconds))/(60*60);
        seconds-=h*60*60;
        int m = ((int)(seconds))/(60);
        seconds-=m*60;
        sprintf(buf,"%2ih %2im %3.1fs",h,m,seconds);
    }

    void endJob()
    {
        float runTime = ((float)clock() -totalTime)/CLOCKS_PER_SEC;
        float eventsPerSecond=1.0*_count/runTime;
        char buf[200];
        char time[50];
        convertTime(time,runTime);
        sprintf(buf,"%s - runtime: %s - processed events: %7.1i [%5.2fk avg. ev./sec.] - Finished!",_name.c_str(),time,(int)_count,eventsPerSecond/1000);
        std::cout<<buf<<std::endl;
    }
    
    void print()
    {
        float runTime = ((float)clock() -totalTime)/CLOCKS_PER_SEC;
        float eventsPerSecond=1.0*(_count-_eventDiff)/((float)(clock()-timeDiff)/CLOCKS_PER_SEC);
        //std::cout<<(_count-_eventDiff)<<", "<<(float)(clock()-timeDiff)/CLOCKS_PER_SEC<<": "<<eventsPerSecond<<std::endl;
        char buf[200];
        char time[50];
        convertTime(time,runTime);
        sprintf(buf,"%s - runtime: %s - processed events: %7.1i [%5.2fk ev./sec.]",_name.c_str(),time,(int)_count,eventsPerSecond/1000);
        std::cout<<buf<<std::endl;
        timeDiff = clock();
        _eventDiff=_count;
    }

    bool analyse(pxl::Sink *sink) throw (std::runtime_error)
    {
        try
        {
            pxl::Event *event  = dynamic_cast<pxl::Event *> (sink->get());
            if (event)
            {

                ++_count;
                if (_count%_events==0 && _events>0 && !_onlyFinal)
                {
                    print();
                }
                float sec = ((float)clock() -timeDiff)/CLOCKS_PER_SEC;
                if (sec>_seconds && _seconds>0 && !_onlyFinal)
                {
                    print();
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

PXL_MODULE_INIT(Counting)
PXL_PLUGIN_INIT
