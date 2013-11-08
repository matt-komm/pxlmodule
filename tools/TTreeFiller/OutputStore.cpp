#include "OutputStore.hpp"

Tree::Tree(TFile* file, std::string name):
    INVALID(-100000),
    _file(file)
{
    _tree = new TTree(name.c_str(),name.c_str());
    _tree->SetDirectory(file);
}

float* Tree::bookVariableAddress(std::string name)
{
    float* address = new float(0);
    _variables[name]=address;
    TBranch* branch = _tree->Branch(name.c_str(),address);
    (*address)=INVALID;
    
    for (int cnt=0;cnt<_count; ++cnt)
    {
        branch->Fill();
    }
    return address;
}

float& Tree::getVariable(std::string name)
{
    std::unordered_map<std::string,float*>::const_iterator elem = _variables.find(name.c_str());
    if (elem==_variables.end()) {
        return *bookVariableAddress(name);
    } else {
        return *elem->second;
    }
}

void Tree::fill()
{
    ++_count;
    _tree->Fill();
}

void Tree::write()
{
    _tree->Write();
}

OutputStore::OutputStore(std::string filename)
{
    _file = new TFile(filename.c_str(),"RECREATE");
}

Tree* OutputStore::getTree(std::string treeName)
{
    std::unordered_map<std::string,Tree*>::const_iterator elem = _treeMap.find(treeName.c_str());
    if (elem==_treeMap.end())
    {
        _treeMap[treeName]=new Tree(_file, treeName);
        return _treeMap[treeName];
    } else {
        return elem->second;
    }
}

void OutputStore::close()
{
    _file->cd();
    for (auto it = _treeMap.begin(); it != _treeMap.end(); ++it )
    {
        it->second->write();
    }
    _file->Close();
}
