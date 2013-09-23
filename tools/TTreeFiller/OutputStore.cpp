#include "OutputStore.hpp"

OutputStore::OutputStore(std::string filename):
INVALID(-999999),
defaultTreeName("default")
{
    file = new TFile(filename.c_str(),"RECREATE");

    count=0;
}

void OutputStore::setDefaultTreeName(std::string treeName)
{
    defaultTreeName=treeName;
}


float* OutputStore::bookVariableAddress(std::string name, std::string treeName)
{
    variables[name+"__"+treeName]=new float(0);
    float* address = getVariableAddress(name, treeName);
    
    TBranch* branch = getTree(treeName)->Branch(name.c_str(),address);
    (*address)=INVALID;
    
    for (int cnt=0;cnt<count; ++cnt)
    {
        branch->Fill();
    }
    return address;
}

float* OutputStore::getVariableAddress(std::string name, std::string treeName)
{
    std::unordered_map<std::string,float*>::const_iterator elem = variables.find((name+"__"+treeName).c_str());
    if (elem==variables.end()) {
        return NULL;
    } else {
        return elem->second;
    }
}

TTree* OutputStore::getTree(std::string treeName)
{
    if (treeMap.find(treeName)==treeMap.end())
    {
        TTree* tree = new TTree(treeName.c_str(),treeName.c_str());
        tree->SetDirectory(file);
        treeMap[treeName]=tree;
    }
    return treeMap[treeName];

}
void OutputStore::storeValue(const float& value, std::string name)
{
    storeValue(value,name,defaultTreeName);
}

void OutputStore::storeValue(const float& value, std::string name, std::string treeName)
{
    if (getVariableAddress(name, treeName)==NULL) {
        (*bookVariableAddress(name, treeName))=value;
    } else {
        (*getVariableAddress(name, treeName))=value;
    }
}

void OutputStore::fill()
{
    //std::cout<<(*getVariableAddress("muon_reliso"))<<std::endl;
    file->cd();
    //tree->SetDirectory(file);
    for (auto it = treeMap.begin(); it != treeMap.end(); ++it )
    {
        it->second->Fill();
    }
    for ( auto it = variables.begin(); it != variables.end(); ++it ) {
        (*it->second)=INVALID;
    }
    ++count;
}

void OutputStore::close()
{
    file->cd();
    //tree->SetDirectory(file);
    for (auto it = treeMap.begin(); it != treeMap.end(); ++it )
    {
        it->second->Write();
    }
    file->Close();
}
