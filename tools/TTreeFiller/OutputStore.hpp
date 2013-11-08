#ifndef _OUTPUTSTORE_H_#define _OUTPUTSTORE_H_#include <unordered_map>#include <TTree.h>#include <TFile.h>#include <TObject.h>#include <TBranch.h>#include <string>#include <iostream>class Tree{    private:        const int INVALID;        int _count;        std::unordered_map<std::string,float*> _variables;        TTree* _tree;        TFile* _file;    public:        Tree(TFile* file, std::string name);        float& getVariable(std::string name);        float* bookVariableAddress(std::string name);        void fill();        void write();};class OutputStore{    private:        TFile* _file;        std::unordered_map<std::string,Tree*> _treeMap;    public:        OutputStore(std::string filename);        Tree* getTree(std::string treeName);        void close();};#endif