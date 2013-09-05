#include "pxl/hep.hh"
#include <string>
#include <unordered_map>
#include <iostream>

class BTaggerAlgorithm
{
    protected:
    std::string _name;
    public:
    BTaggerAlgorithm(): _name("none")
    {
    }
    
    std::string getName()
    {
        return _name;
    }

    virtual bool testBtagged(pxl::Particle* particle)
    {
        return false;
    }

    ~BTaggerAlgorithm()
    {
    }
};

class CSVTAlgorithm:
    public BTaggerAlgorithm
{
    public:
    CSVTAlgorithm():BTaggerAlgorithm()
    {
    }
    
    virtual bool testBtagged(pxl::Particle* particle)
    {
        if (particle->hasUserRecord("combinedSecondaryVertexBJetTags"))
        {
            return particle->getUserRecord("combinedSecondaryVertexBJetTags").toFloat()>0.898;
        } else {
            throw std::runtime_error("The following particle is missing an UR field to apply CSVT btagging\r\n"+particle->toString());
        }
        return false;
    }
    
    void applyScaleFactors(pxl::Particle* particle)
    {
    }
    
    ~CSVTAlgorithm()
    {
        //BTaggerAlgorithm::~BTaggerAlgorithm();
    }
};


static const std::unordered_map<std::string,BTaggerAlgorithm*> BTaggerDataBase=
{
    {"CSVT",new CSVTAlgorithm()}
};
