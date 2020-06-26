#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include "tree.h"
#include "error.h"
using namespace std;


bool print = false;
bool alloc = false;
int count = 0;
void genTree(Node &node) {
    string str;
    getline(cin, str);
    if (str == "EOF EOF") {
        return;
    }
    node.token = str;
    istringstream iss {str};
    iss >> str;
    if (str.size() != 0 && 'A' <= str[0] && str[0] <= 'Z') {
        node.token = str;
        iss >> str;
        node.lexeme = str;
    }
    int i = 0;
    while (iss >> str) {
        Node newnode {str};
        node.children.push_back(newnode);
        genTree(node.children[i]);
        ++i;
    }
}


void printNode(Node &node) {
    if (node.lexeme != "") {
        cout << node.token << ' ' << node.lexeme << endl;
    } else {
        cout << node.token << endl;
    }
    int len = node.children.size();
    for (int i = 0; i < len; ++i) {
        printNode(node.children[i]);
    }
}


void Node::pushSig(map<string, pair<vector<string>, map<string, pair<int ,string>>>> &symtable, 
                    string &type, string &procname) {
    if (type == "type INT STAR") {
        type = "int*";
    } else {
        type = "int";
    }
    symtable[procname].first.push_back(type);
}


void Node::pushSym(map<string, pair<vector<string>, map<string, pair<int ,string>>>> &symtable, 
                    string &id, string &type, string &procname, int &offset) {
    if (type == "type INT STAR") {
        type = "int*";
    } else {
        type = "int";
    }
    if (symtable[procname].second.find(id) != symtable[procname].second.end()) {
        throw(Error {"ERROR: declared more than once"});
    }
    symtable[procname].second.emplace(id, make_pair(offset, type));
    offset -= 4;
}


int countParams(Node &node, int offset) {
    if (node.token == "params") return 0;
    else if (node.token == "params paramlist") return countParams(node.children[0], offset);
    else if (node.token == "paramlist dcl") return offset += 4;
    else return countParams(node.children[2], offset += 4);
}


void Node::genSymtable(string &procname, map<string, pair<vector<string>,
                       map<string, pair<int ,string>>>> &symtable, int &offset) {
    string str, type, id;
    istringstream iss {token};
    iss >> str;
    if (str == "procedure") {
        procname = children[1].lexeme;
        offset = countParams(children[3], 0);
    } else if (str == "main") {
        procname = "wain";
        type = children[3].children[0].token;
        id = children[3].children[1].lexeme;
        pushSig(symtable, type, procname);

        type = children[5].children[0].token;
        id = children[5].children[1].lexeme;
        pushSig(symtable, type, procname);
        offset = 0;
    } else if (str == "params" && children.size() == 0) {
        symtable[procname];
    } else if (str == "paramlist") {
        type = children[0].children[0].token;
        id = children[0].children[1].lexeme;
        pushSig(symtable, type, procname);
    } else if (str == "dcl") {
        type = children[0].token;
        id = children[1].lexeme;
        pushSym(symtable, id, type, procname, offset);
    } else if (str == "ID") {
        if (symtable[procname].second.find(lexeme) == symtable[procname].second.end() &&
            symtable.find(lexeme) == symtable.end()) {
            throw(Error {"ERROR: used without declaration"});
        }
    } else if (str == "factor") {
        iss >> str >> str;
        if (str == "LPAREN") {
            id = children[0].lexeme;
            if (symtable.find(id) == symtable.end()) {
                throw(Error {"ERROR: func not declared"});
            } else if (symtable[procname].second.find(id) != symtable[procname].second.end()) {
                throw(Error {"ERROR: variable is not a func"});
            }
        }
    }
    int len = children.size();
    for (int i = 0; i < len; ++i) {
        children[i].genSymtable(procname, symtable, offset);
    }
}


void checkSig(Node &node, map<string, pair<vector<string>, map<string, pair<int ,string>>>> &symtable,
              string &id, string &procname, int count) {
    string type = node.children[0].checkTypeSubt(symtable, procname);

    int len = symtable[id].first.size() - 1;
    if (count > len) {
        string err = "ERROR: wrong number of arg of ";
        err += id + " in " + procname;
        throw(Error {err});
    } else if (symtable[id].first[count] != type) {
        string err = "ERROR: wrong arg type of ";
        err += id + " in " + procname;
        throw(Error {err});
    } else if (node.children.size() < 3 && count != len) {
        string err = "ERROR: wrong number of arg of ";
        err += id + " in " + procname;
        throw(Error {err});
    }
    
    if (node.children.size() == 1) {
        return;
    }
    checkSig(node.children[2], symtable, id, procname, ++count);
}


void Node::checkType(map<string, pair<vector<string>, map<string, pair<int ,string>>>> &symtable, 
                    string &procname) {
    string str, type, err;
    istringstream iss {token};
    iss >> str;
    if (str == "procedure") {
        procname = children[1].lexeme;
        type = children[9].checkTypeSubt(symtable, procname);
        if (type != "int") {
            err = "ERROR: expect " + procname + " returns an int";
            throw(Error {err});
        }
    } else if (str == "main") {
        procname = "wain";
        type = children[11].checkTypeSubt(symtable, procname);
        if (type != "int") {
            throw(Error {"ERROR: expect wain returns an int"});
        }
        string type = children[5].children[1].checkTypeSubt(symtable, procname);
        if (type != "int") {
            throw(Error {"ERROR: expect an int as second arg in wain"});
        }
    } else if (str == "expr" || str == "lvalue") {
        checkTypeSubt(symtable, procname); 
        return;
    } else if (str == "statement" || str == "test" || str == "dcls") {
        checkTypeStat(symtable, procname); 
        return;
    }
    int len = children.size();
    for (int i = 0; i < len; ++i) {
        children[i].checkType(symtable, procname);
    }
}


string Node::checkTypeSubt(map<string, pair<vector<string>, map<string, pair<int ,string>>>> &symtable, 
                                string &procname) {
    string str, type, id;
    istringstream iss {token};
    iss >> str;

    if(str == "ID") {
        id = lexeme;
        type = symtable[procname].second[id].second;
        return type;
    }
    if (token == "expr expr PLUS term") {
        string left = children[0].checkTypeSubt(symtable, procname);
        string right = children[2].checkTypeSubt(symtable, procname);
        if (left == "int*" && right == "int*") {
            throw(Error {"ERROR: no rule between int* + int*"});
        }
        return left == right? "int" : "int*";
    } else if (token == "expr expr MINUS term") {
        string left = children[0].checkTypeSubt(symtable, procname);
        string right = children[2].checkTypeSubt(symtable, procname);
        if (left == "int" && right == "int*") {
            throw(Error {"ERROR: no rule between int - int*"});
        }
        return (left == "int*" && right == "int")? "int*" : "int";
    } else if (token == "expr term") {
        return children[0].checkTypeSubt(symtable, procname);
    } else if (str == "term") {
        if (children.size() == 3) {
            string left = children[0].checkTypeSubt(symtable, procname);
            string right = children[2].checkTypeSubt(symtable, procname);
            iss >> str >> str;
            if (left != "int" || right != "int") {
                string err = "ERROR: no rule between ";
                err = err + left + " and " + right;
                throw(Error {err});
            }
            return "int";
        } else {
            return children[0].checkTypeSubt(symtable, procname);
        }
    } else if (token == "factor ID") {
        return symtable[procname].second[children[0].lexeme].second;
    } else if (token == "factor NUM") {
        return "int";
    } else if (token == "factor NULL") {
        return "int*";
    } else if (token == "factor LPAREN expr RPAREN") {
        return children[1].checkTypeSubt(symtable, procname);
    } else if (token == "factor AMP lvalue") {
        type = children[1].checkTypeSubt(symtable, procname);
        if (type != "int") {
            string err = "ERROR: no rule for ";
            err = err + "&" + type;
            throw(Error {err});
        }
        return "int*";
    } else if (token == "factor STAR factor") {
        type = children[1].checkTypeSubt(symtable, procname);
        if (type != "int*") {
            string err = "ERROR: no rule for ";
            err = err + "*" + type;
            throw(Error {err});
        }
        return "int"; 
    } else if (token == "factor NEW INT LBRACK expr RBRACK") {
        type = children[3].checkTypeSubt(symtable, procname);
        if (type != "int") {
            throw(Error {"ERROR: expect an int for new"});
        }
        return "int*";
    } else if (token == "factor ID LPAREN RPAREN") {
        id = children[0].lexeme;
        if (!symtable[id].first.empty()) {
            string err = "ERROR: wrong number of atgs of ";
            err += id;
            throw(Error {err});
        }
        return "int";
    } else if (token == "factor ID LPAREN arglist RPAREN") {
        id = children[0].lexeme;
        checkSig(children[2], symtable, id, procname, 0);
        return "int";
    } else if (token == "lvalue ID") {
        return children[0].checkTypeSubt(symtable, procname);
    } else if (token == "lvalue STAR factor") {
        type = children[1].checkTypeSubt(symtable, procname);
        if (type != "int*") {
            string err = "ERROR: no rule for ";
            err = err + " * " + type;
            throw(Error {err});
        }
        return "int";
    } else if (token == "lvalue LPAREN lvalue RPAREN") {
        return children[1].checkTypeSubt(symtable, procname);
    }
    return "";
}


void Node::checkTypeStat(map<string, pair<vector<string>, map<string, pair<int, string>>>> &symtable, 
                            string &procname) {
    string str, err, type, left, right;
    istringstream iss {token};
    iss >> str;

    if (token == "statement lvalue BECOMES expr SEMI" || str == "test") {
        left = children[0].checkTypeSubt(symtable, procname);
        right = children[2].checkTypeSubt(symtable, procname);
        if (left != right) {
            err = "ERROR: cannot assign or compair " + right + " to " + left;
            throw (Error {err});
        }
    } else if (token == 
                "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {
        children[2].checkTypeStat(symtable, procname);
        children[5].checkType(symtable, procname);
        children[9].checkType(symtable, procname);
    } else if (token == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
        children[2].checkType(symtable, procname);
        children[5].checkType(symtable, procname);
    } else if (token == "statement PRINTLN LPAREN expr RPAREN SEMI") {
        type = children[2].checkTypeSubt(symtable, procname);
        if (type != "int") {
            throw(Error {"ERROR: println can only be used with int"});
        }
    } else if (token == "statement DELETE LBRACK RBRACK expr SEMI") {
        type = children[3].checkTypeSubt(symtable, procname);
        if (type != "int*") {
            throw(Error {"ERROR: delete [] can only be used with int*"});
        }
    } else if (token == "dcls dcls dcl BECOMES NUM SEMI") {
        type = children[1].children[1].checkTypeSubt(symtable, procname);
        if (type != "int") {
            throw(Error {"ERROR: cannot assign num to int*"});
        }
        children[0].checkType(symtable, procname);
    } else if (token == "dcls dcls dcl BECOMES NULL SEMI") {
        type = children[1].children[1].checkTypeSubt(symtable, procname);
        if (type != "int*") {
            throw(Error {"ERROR: cannot assign NULL to int"});
        } 
        children[0].checkType(symtable, procname);
    }
}


int num = 0;
void push(int reg) {
    cout << "sw $" << reg << ", -4($30)" << endl << "sub $30, $30, $4" << endl;
}


void pop(int reg) {
    cout << "add $30, $30, $4" << endl << "lw $" << reg << ", -4($30)" << endl;
}


string procname;
string code(string id, map<string, pair<vector<string>, map<string, pair<int, string>>>> &symtable) {
    auto val = symtable[procname].second[id];
    cout << "lw $3, " << val.first << "($29)" << endl;
    return val.second;
}


string Node::genCode(map<string, pair<vector<string>, map<string, pair<int, string>>>> &symtable) {
    string str, id;
    istringstream iss {token};
    iss >> str; 
    int len = children.size();
    if (token == "procedures procedure procedures") {
        children[1].genCode(symtable);
        children[0].genCode(symtable);
    } else if (str == "procedure") {
        procname = children[1].lexeme;
        cout << 'f' << procname << ':' << endl;
        cout << "sub $29, $30, $4" << endl;
        children[6].genCode(symtable);
        children[7].genCode(symtable);
        children[9].genCode(symtable);
        cout << "add $30, $29, $4" << endl << "jr $31" << endl;
    } else if (str == "main") {
        procname = "wain";
        cout << "sub $29, $30, $4" << endl;
        cout << ".import init" << endl;
        push(2);
        push(31);
        string type = symtable[procname].first[0];
        if (type == "int") cout << "add $2, $0, $0" << endl;
        cout << "lis $5" << endl << ".word init" << endl;
        cout << "jalr $5" << endl;
        pop(31);
        pop(2);
        push(1);
        push(2);
        children[8].genCode(symtable);
        children[9].genCode(symtable);
        children[11].genCode(symtable);
        cout << "jr $31" << endl;
    } else if (token == "factor ID LPAREN RPAREN" || token == "factor ID LPAREN arglist RPAREN") {
        push(29);
        push(31);
        if (children.size() == 4) children[2].genCode(symtable);
        cout << "lis $5" << endl << ".word " << 'f' << children[0].lexeme << endl;
        cout << "jalr $5" << endl;
        for(int i = 0; i < num; ++i) pop(5);
        pop(31);
        pop(29);
        num = 0;
        return "int";
    } else if (token == "factor ID") {
        id = children[0].lexeme;
        return code(id, symtable);
    } else if ((str == "expr" || str == "term") && len == 3) {
        iss >> str >> str;
        string left = children[0].genCode(symtable);
        push(3);
        string right = children[2].genCode(symtable);
        pop(5);
        //$3 <- right
        //$5 <- left
        if (left == "int*" && right == "int") cout << "mult $3, $4" << endl << "mflo $3" << endl;
        else if (right == "int*" && str == "PLUS") cout << "mult $5, $4" << endl << "mflo $5" << endl;

        if (str == "PLUS") cout << "add $3, $5, $3" << endl;
        else if (str == "MINUS") cout <<  "sub $3, $5, $3" << endl;
        else if (str == "STAR") cout << "mult $3, $5" << endl << "mflo $3" << endl;
        else if (str == "SLASH") cout << "div $5, $3" << endl << "mflo $3" << endl;
        else if (str == "PCT") cout << "div $5, $3" << endl << "mfhi $3" << endl;
        
        if (left == "int*" && right == "int*") 
        cout << "div $3, $4" << endl << "mflo $3" << endl;

        return left == "int*" || right == "int*"? "int*" : "int";
    } else if (token == "factor NUM") {
        cout << "lis $3" << endl << ".word " << children[0].lexeme << endl;
        return "int";
    } else if (token == "NUM") {
        cout << "lis $5" << endl << ".word " << lexeme << endl;
        push(5);
        return "int";
    } else if (token == "statement PRINTLN LPAREN expr RPAREN SEMI") {
        if (!print) cout << ".import print" << endl;
        print = true;
        push(1);
        children[2].genCode(symtable);
        cout << "add $1, $3, $0" << endl;
        push(31);
        cout << "lis $5" << endl << ".word print" << endl;
        cout << "jalr $5" << endl;
        pop(31);
        pop(1);
    } else if (token == "statement lvalue BECOMES expr SEMI") {
        if (children[0].token == "lvalue LPAREN lvalue RPAREN") {
            children[0].genCode(symtable);
        } else if (children[0].token == "lvalue ID") {
            children[2].genCode(symtable);
            int offset = symtable[procname].second[children[0].children[0].lexeme].first;
            cout << "sw $3, " << offset << "($29)" << endl;
        } else if (children[0].token == "lvalue STAR factor") {
            children[2].genCode(symtable);
            push(3);
            children[0].children[1].genCode(symtable);
            pop(5);
            cout << "sw $5, 0($3)" << endl;
            return "int";
        }
    } else if (str == "test") {
        string comp = "slt";
        string left = children[0].genCode(symtable);
        push(3);
        children[2].genCode(symtable);
        pop(5);
        iss >> str >> str;
        if (left == "int*") comp = "sltu";
        if (str == "EQ") {
            cout << comp << " $6, $5, $3" << endl << comp << " $7, $3, $5" << endl;
            cout << "add $3, $6, $7" << endl << "sub $3, $11, $3" << endl;
        } else if (str == "NE") {
            cout << comp << " $6, $5, $3" << endl << comp << " $7, $3, $5" << endl;
            cout << "add $3, $6, $7" << endl;
        } else if (str == "LT") {
            cout << comp << " $3, $5, $3" << endl;
        } else if (str == "LE") {
            cout << comp << " $3, $3, $5" << endl << "sub $3, $11, $3" << endl;
        } else if (str == "GE") {
            cout << comp << " $3, $5, $3" << endl << "sub $3, $11, $3" << endl;
        } else {
            cout << comp << " $3, $3, $5" << endl;
        }
    } else if (token == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
        int temp = count;
        count++;
        cout << "while" << temp << ':' << endl;
        children[2].genCode(symtable);
        cout << "beq $3, $0, endofWhile" << temp << endl;
        children[5].genCode(symtable);
        cout << "beq $0, $0, while" << temp << endl; 
        cout << "endofWhile" << temp << ':' << endl;
    } else if (token == 
                "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {
        int temp = count;
        count++;
        children[2].genCode(symtable);
        cout << "beq $3, $0, else" << temp << endl;
        children[5].genCode(symtable);
        cout << "beq $0, $0, end" << temp << endl;
        cout << "else" << temp << ':' << endl;
        children[9].genCode(symtable);
        cout << "end" << temp << ':' << endl;
    } else if (token == "factor NULL") {
        cout << "add $3, $0, $11" << endl;
        return "int*";
    } else if (token == "factor AMP lvalue") {
        if (children[1].token == "lvalue STAR factor") children[1].genCode(symtable);
        else if (children[1].token == "lavlue LPAREN lvalue RPAREN") {
            children[1].genCode(symtable);
        } else {
            int offset = symtable[procname].second[children[1].children[0].lexeme].first;
            cout << "lis $3" << endl << ".word " << offset << endl;
            cout << "add $3, $3, $29" << endl;
        }
        return "int*";
    } else if (token == "factor STAR factor") {
        children[1].genCode(symtable);
        cout << "lw $3, 0($3)" << endl;
        return "int";
    } else if (token == "NULL") {
        cout << "add $3, $0, $11" << endl;
        push(3);
        return "int*";
    } else if (token == "factor NEW INT LBRACK expr RBRACK" || 
               token == "statement DELETE LBRACK RBRACK expr SEMI") {
        if (!alloc) {
            cout << ".import new" << endl << ".import delete" << endl;
            alloc = true;
        }
        iss >> str;
        push(1);
        push(31);
        children[3].genCode(symtable);
        if (str == "NEW") {
            cout << "add $1, $3, $0" << endl;
            cout << "lis $5" << endl << ".word new" << endl;
            cout << "jalr $5" << endl;
            cout << "bne $3, $0, 1" << endl << "add $3, $11, $0" << endl;
        } else {
            int temp = count;
            count++;
            cout << "beq $3, $11, skipDelete" << temp << endl << "add $1, $3, $0" << endl;
            cout << "lis $5" << endl << ".word delete" << endl;
            cout << "jalr $5" << endl << "skipDelete" << temp << ':' << endl;
        }
        pop(31);
        pop(1);
        return "int*";
    } else if (str == "arglist") {
        children[0].genCode(symtable);
        push(3);
        if (children.size() == 3) children[2].genCode(symtable);
        num++;
    }
    else {
        str = "";
        for (int i = 0; i < len; ++i) {
            str += children[i].genCode(symtable);
        }
        return str;
    }
    return "";
}

