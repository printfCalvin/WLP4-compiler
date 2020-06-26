#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include "tree.h"
#include "error.h"
using namespace std;


int main() {
    string str;
    Node tree;
    getline(cin, str);
    getline(cin, str);
    genTree(tree);
    map<string, pair<vector<string>, map<string, pair<int, string>>>> symtable;
    string procname;
    int offset = 0;
    try {
        tree.genSymtable(procname, symtable, offset);
        procname = "";
        /*for (auto it1 : symtable) {
            string name = it1.first;
            cerr << name << ' ';
            for (auto it2 : symtable[name].first) {
                cerr << it2 << ' ';
            }
            cerr << endl;
            for (auto it3 : symtable[name].second) {
                cerr << it3.first << ' ' << it3.second.first << ' ' << it3.second.second << endl;
            }
            cerr << endl;
        }*/
        tree.checkType(symtable, procname);
        cout << "lis $4" << endl << ".word 4" << endl;
        cout << "lis $11" << endl << ".word 1" << endl;
        tree.genCode(symtable);
    } catch(Error &e) {
        cerr << e.what() << endl;
    }
}

