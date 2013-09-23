#include "pxl/hep.hh"
#include "pxl/core.hh"
#include "pxl/core/macros.hh"
#include "pxl/core/PluginManager.hh"
#include "pxl/modules/Module.hh"
#include "pxl/modules/ModuleFactory.hh"

#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

#include "OutputStore.hpp"

static pxl::Logger logger("TTreeFiller");

class AccessorTree
{
    class Node
    {
        private:
            std::string _field;
            std::vector<Node*> _children;
            Node* _parent;

        public:
        Node(std::string field, Node* parent):
            _field(field),
            _parent(parent)
        {
        }

        Node* getParent()
        {
            return _parent;
        }

        /*
        std::string getName()
        {
            std::string name = _field;

            if (_multiple>0)
            {
                char buf[5];
                sprintf(buf,"%i",_multiple);
                name = _field+"__"+std::string(buf);
            }

            Node* parent = getParent();
            while (parent->getField()!="")
            {
                name = parent->getField()+"_"+name;
                parent=parent->getParent();
            }
            return name;
        }
        */

        bool isLeaf()
        {
            return _children.size()==0;
        }

        void addNode(Node* n)
        {
            _children.push_back(n);
        }

        std::string getField()
        {
            return _field;
        }

        Node* insertPath(std::vector<std::string>& path)
        {
            if (path.size()>0)
            {
                for (unsigned i=0;i<_children.size();++i)
                {

                    if (path.back()==_children[i]->getField())
                    {
                        path.pop_back();
                        return _children[i]->insertPath(path);
                    }
                }
                Node* n = new Node(path.back(),this);
                addNode(n);
                path.pop_back();
                return n->insertPath(path);
            } else {
                return this;
            }
        }

        std::string getId(std::string prefix,int multiplicity)
        {
            char buf[4];
            if (multiplicity>0)
            {
                sprintf(buf,"%i",multiplicity);
                if (prefix=="")
                {
                    return _field+"__"+std::string(buf);
                }
                else
                {
                    return prefix+"_"+_field+"__"+std::string(buf);
                }
            }
            else
            {
                if (prefix=="")
                {
                    return _field;
                }
                else
                {
                    return prefix+"_"+_field;
                }
            }
        }

        template<class T>
        void evaluateChildren(T* t, OutputStore* store, std::string prefix)
        {
            for (unsigned ichildren=0;ichildren<_children.size();++ichildren)
            {
                _children[ichildren]->evaluate(t,store,prefix);
            }
        }

        void evaluate(pxl::Event* event, OutputStore* store, std::string prefix)
        {
            if (event->hasUserRecord(_field))
            {
                store->storeValue(event->getUserRecord(_field).toFloat(),getId(prefix,0));
            }
            else
            {
                std::vector<pxl::EventView*> eventViews;
                event->getObjectsOfType(eventViews);

                int multiplicity=0;
                for (unsigned ieventView=0;ieventView<eventViews.size();++ieventView)
                {
                    if (eventViews[ieventView]->getName()==_field)
                    {
                        ++multiplicity;
                        evaluateChildren<pxl::EventView>(eventViews[ieventView],store,getId(prefix,multiplicity));
                    }
                }
            }
        }

        void evaluate(pxl::EventView* eventView, OutputStore* store, std::string prefix)
        {
            if (eventView->hasUserRecord(_field))
            {
                store->storeValue(eventView->getUserRecord(_field).toFloat(),getId(prefix,0));
            }
            else
            {
                std::vector<pxl::Particle*> particles;
                eventView->getObjectsOfType(particles);

                int multiplicity=0;
                for (unsigned iparticle=0;iparticle<particles.size();++iparticle)
                {
                    if (particles[iparticle]->getName()==_field)
                    {
                        ++multiplicity;
                        evaluateChildren<pxl::Particle>(particles[iparticle],store,getId(prefix,multiplicity));
                    }
                }
            }
        }

        void evaluate(pxl::Particle* particle, OutputStore* store, std::string prefix)
        {
            if (particle->hasUserRecord(_field))
            {
                store->storeValue(particle->getUserRecord(_field),getId(prefix,0));
            }
            else if (_field=="Pt")
            {
                store->storeValue(particle->getPt(),getId(prefix,0));
            }
            else if (_field=="Eta")
            {
                store->storeValue(particle->getEta(),getId(prefix,0));
            }
            else if (_field=="Phi")
            {
                store->storeValue(particle->getPhi(),getId(prefix,0));
            }
            else if (_field=="Mass")
            {
                store->storeValue(particle->getMass(),getId(prefix,0));
            }
            else if (_field=="Px")
            {
                store->storeValue(particle->getPx(),getId(prefix,0));
            }
            else if (_field=="Py")
            {
                store->storeValue(particle->getPy(),getId(prefix,0));
            }
            else if (_field=="Pz")
            {
                store->storeValue(particle->getPz(),getId(prefix,0));
            }
            else if (_field=="Charge")
            {
                store->storeValue(particle->getPz(),getId(prefix,0));
            }
            else if (_field=="Mother")
            {
                int multiplicity=0;
                std::vector<pxl::Particle*> particles;
                particle->getMotherRelations().getObjectsOfType(particles);
                for (unsigned iparticle=0;iparticle<particles.size();++iparticle)
                {
                    ++multiplicity;
                    evaluateChildren<pxl::Particle>(particles[iparticle],store,getId(prefix,multiplicity));
                }
            }
            else if (_field=="Daughter")
            {
                int multiplicity=0;
                std::vector<pxl::Particle*> particles;
                particle->getDaughterRelations().getObjectsOfType(particles);
                for (unsigned iparticle=0;iparticle<particles.size();++iparticle)
                {
                    ++multiplicity;
                    evaluateChildren<pxl::Particle>(particles[iparticle],store,getId(prefix,multiplicity));
                }
            }
        }

        bool operator==(const Node& rhs)
        {
            return _field==rhs._field;
        }

        void toString(int depth)
        {
            for (int i=0;i<depth;++i)
            {
                std::cout<<" -  ";
            }
            std::cout<<_field<<std::endl;
            for (unsigned i=0;i<_children.size();++i)
            {
                _children[i]->toString(depth+1);
            }
        }
    };
    private:
    Node* _root;
    public:
    AccessorTree():
        _root(new Node("",0))
    {
    }

    template<class T>
    void evaluate(T* t, OutputStore* store)
    {
        _root->evaluateChildren(t,store,"");
    }

    Node* insert(std::vector<std::string> path)
    {
        std::reverse(path.begin(),path.end());
        return _root->insertPath(path);
    }

    void toString()
    {
        _root->toString(0);
    }

};

class TTreeFiller : public pxl::Module
{
    private:
    pxl::Source* _output;
    std::string _outFileName;
    OutputStore* _outputStore;
    AccessorTree* _accessorTree;

    std::vector<std::string> _fields;

    public:
    TTreeFiller() :
        Module(),
        _outFileName("out.root"),
        _outputStore(0),
        _accessorTree(new AccessorTree())
    {
        addSink("input", "Input");
        _output = addSource("output", "output");

        _fields.push_back("Reconstructed:TightMuon:Pt");

        addOption("output file","name of the root output file",_outFileName,pxl::OptionDescription::USAGE_FILE_SAVE);
        addOption("fields","fields to write out",_fields);
    }

    ~TTreeFiller()
    {
    }

    // every Module needs a unique type
    static const std::string &getStaticType()
    {
        static std::string type ("TTreeFiller");
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
        _outputStore = new OutputStore(_outFileName);


        getOption("fields",_fields);
        for (unsigned i=0;i<_fields.size();++i)
        {
            std::vector<std::string> elem = split(_fields[i],':');
            _accessorTree->insert(elem);
        }
        _accessorTree->toString();


    }

    void endJob()
    {
        _outputStore->close();
    }

    std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems)
    {
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            elems.push_back(item);
        }
        return elems;
    }

    std::vector<std::string> split(const std::string &s, char deliminator)
    {
        std::vector<std::string> elems;
        split(s, deliminator, elems);
        return elems;
    }

    bool analyse(pxl::Sink *sink) throw (std::runtime_error)
    {
        try
        {
            pxl::Event *event  = dynamic_cast<pxl::Event *> (sink->get());
            if (event)
            {
                _outputStore->setDefaultTreeName(event->getUserRecord("Process"));
                _accessorTree->evaluate(event,_outputStore);
                _outputStore->fill();
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

PXL_MODULE_INIT(TTreeFiller)
PXL_PLUGIN_INIT
