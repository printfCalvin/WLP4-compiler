#ifndef TREE_H
#define TREE_H
#include <string>
#include <map>
#include <vector>


struct Node {
    std::string token, lexeme;
    std::vector<Node> children;

    Node(std::string token, std::string lexeme) : token{token}, lexeme{lexeme} {}
    Node(std::string token) : token{token}, lexeme{""} {}
    Node() : token{""}, lexeme{""} {}
    void printNode();
    void genSymtable(std::string &procname, 
		     std::map<std::string, std::pair<std::vector<std::string>, 
                        std::map<std::string, std::pair<int, std::string>>>> &symtable, int &offset);

    void pushSym(std::map<std::string, std::pair<std::vector<std::string>, 
                 std::map<std::string, std::pair<int, std::string>>>> &symtable, 
		         std::string &id, std::string &type, std::string &procname, int &offset);

    void pushSig(std::map<std::string, std::pair<std::vector<std::string>, 
                 std::map<std::string, std::pair<int, std::string>>>> &symtable, 
		         std::string &type, std::string &procname);

    void checkType(std::map<std::string, std::pair<std::vector<std::string>, 
                   std::map<std::string, std::pair<int, std::string>>>> &symtable,
                   std::string &procname);

    std::string checkTypeSubt(std::map<std::string, std::pair<std::vector<std::string>, 
                              std::map<std::string, std::pair<int, std::string>>>> &symtable,
                              std::string &procname);

    void checkTypeStat(std::map<std::string, std::pair<std::vector<std::string>, 
                       std::map<std::string, std::pair<int, std::string>>>> &symtable,
                       std::string &procname);

    std::string genCode(std::map<std::string, std::pair<std::vector<std::string>, 
                        std::map<std::string, std::pair<int, std::string>>>> &symtable);

};

void genTree(Node &node);
void printNode(Node &node);
void push(int reg);
void pop(int reg);

#endif
