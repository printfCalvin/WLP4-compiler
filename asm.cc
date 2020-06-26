#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "scanner.h"
using namespace std;


class Error {
	string message;
	public:
	Error(string message) : message{message} {}
	const string &what() const {
		return message;
	}
};


void printBinary(int64_t instr) {
	unsigned char c = instr >> 24;
	cout << c;
	c = instr >> 16;
	cout << c;
	c = instr >> 8;
	cout << c;
	c = instr;
	cout << c;
}


void printBinary(int opcode, int reg1, int reg2, int reg3, int offset) {
    int instr = (opcode << 26) | (reg1 << 21) | (reg2 << 16) | (reg3 << 11) | (offset & 0xffff);
    unsigned char c = instr >> 24;
	cout << c;
	c = instr >> 16;
	cout << c;
	c = instr >> 8;
	cout << c;
	c = instr;
	cout << c;
}


/*int argnum(vector<Token>::iterator it, vector<Token>::iterator end) {
    int num = 0;
    for (; it != end; ++it) {
        num++;
    }
    return num;
}*/


void word(vector<Token>::iterator &it, vector<Token>::iterator end, map<string, int> &stable) {
    ++it;
    auto it2 = it;
    ++it2;
    if (it->getKind() == Token::INT && it2 == end) {
        int64_t num = it->toLong();
        if (-2147483648 <= num && num <= 4294967295) {
            printBinary(num);
        } else {
            throw(Error("ERROR: int out of range."));
        }
    } else if (it->getKind() == Token::HEXINT && it2 == end) {
        int64_t num = it->toLong();
        if (num <= 0xffffffff) {
            printBinary(num);
        } else {
            throw(Error("ERROR: int out of range."));
        }
    } else if (it->getKind() == Token::ID && it2 == end) {
        auto addr = stable.find(it->getLexeme());
        if ( addr != stable.end()) {
            printBinary(addr->second);
        } else {
            throw(Error("ERROR: label not found"));
        }
    } else {
        throw(Error("ERROR: expect a number or label followied by .word."));
    }
}


void oneArg(vector<Token>::iterator &it, vector<Token>::iterator end, string &op) {
    auto it2 = it;
    ++it2;
    if (it2 != end) {
        throw(Error("ERROR: wrong number of arg"));
    } else if (it->getKind() == Token::REG) {
        int reg = it->toLong();
        if (reg < 0 || reg > 31) {
            throw(Error("ERROR: expect a valid register number"));
        }
        if (op == "jr") printBinary(0, reg, 0, 0, 8);
        if (op == "jalr") printBinary(0, reg, 0, 0, 9);
        if (op == "lis") printBinary(0, 0, 0, reg, 20);
        if (op == "mflo") printBinary(0, 0, 0, reg, 18);
        if (op == "mfhi") printBinary(0, 0, 0, reg, 16);
    } else {
        throw(Error("ERROR: expect a valid register number"));
    }
}


void brancheq(vector<Token>::iterator &it, vector<Token>::iterator end, string &op, map<string, int> &stable,
int count) {
    int regs, regt;
    int64_t val;
    vector<Token>::iterator it2;
    for (int i = 0; i < 5; ++i, ++it) {
        if (it->getKind() == Token::REG && (i == 0 || i == 2)) {
            if (i == 0) regs = it->toLong();
            if (i == 2) regt = it->toLong();
        } else if (i == 4 && it->getKind() == Token::HEXINT) {
            val = it->toLong();
            if (val > 0xffff) {
                throw(Error("ERROR: int out of bound"));
            }
            it2 = it;
        } else if (i == 4 && it->getKind() == Token::INT) {
            val = it->toLong();
            if (-32768 > val || val > 32767) {
                throw(Error("ERROR: int out of bound"));
            }
            it2 = it;
        } else if (i == 4 && it->getKind() == Token::ID) {
            auto addr = stable.find(it->getLexeme());
            if ( addr != stable.end()) {
                val = (addr->second - count - 4) / 4;
                //cerr << addr->first << ' ' << addr->second << ' ' << count << endl;
                if (val < -32768 || val > 32767) {
                    throw(Error("ERROR: out of bound"));
                }
            } else {
                throw(Error("ERROR: label not found"));
            }
            it2 = it;
        } else if (it->getKind() == Token::COMMA && i % 2 == 1) {
            continue;
        } else {
            throw(Error("ERROR: expect 2 registers followed by int with commas in between"));
        }
    }
    if ((0 <= regs && regs <= 31) && (0 <= regt && regt <= 31) && (it == end)) {    
        if (op == "beq") printBinary(4, regs, regt, 0, val);
        if (op == "bne") printBinary(5, regs, regt, 0, val);
    } else {
        throw(Error("ERROR: expect 2 valid register number and an 2byte int"));
    }
    it = it2;
}


void fiveArg(vector<Token>::iterator &it, vector<Token>::iterator end, string &op) {
    int regd, regs, regt;
    auto it2 = it;
    for (int i = 0; i < 5; ++i, ++it) {
        if (it->getKind() == Token::REG && i % 2 == 0) {
            if (i == 0) regd = it->toLong();
            if (i == 2) regs = it->toLong();
            if (i == 4) {
                regt = it->toLong();
                it2 = it;
            }
        } else if (it->getKind() == Token::COMMA && i % 2 == 1) {
            continue;
        } else {
            throw(Error("ERROR: expect 3 registers with commas in between"));
        }
    }
    if ((0 <= regd && regd <= 31) && (0 <= regs && regs <= 31) &&
        (0 <= regt && regt <= 31) && (it == end)) {
        if (op == "add") printBinary(0, regs, regt, regd, 32);
        if (op == "sub") printBinary(0, regs, regt, regd, 34);
        if (op == "slt") printBinary(0, regs, regt, regd, 42);
        if (op == "sltu") printBinary(0, regs, regt, regd, 43);
    } else {
        throw(Error("ERROR: expect 3 valid register numbers"));
    }
    it = it2;
}


void twoArg(vector<Token>::iterator &it, vector<Token>::iterator end, string &op) {
    int regs, regt;
    auto it2 = it;
    for (int i = 0; i < 3; ++i, ++it) {
        if (it->getKind() == Token::REG && i % 2 == 0) {
            if (i == 0) regs = it->toLong();
            if (i == 2) {
                regt = it->toLong();
                it2 = it;
            }
        } else if (it->getKind() == Token::COMMA && i == 1) {
            continue;
        } else {
            throw(Error("ERROR: expect 2 registers with comma in between"));
        }
    }
    if ((0 <= regs && regs <= 31) && (0 <= regt && regt <= 31) && (it == end)) {
        if (op == "mult") printBinary(0, regs, regt, 0, 24);
        if (op == "multu") printBinary(0, regs, regt, 0, 25);
        if (op == "div") printBinary(0, regs, regt, 0, 26);
        if (op == "divu") printBinary(0, regs, regt, 0, 27);
    } else {
        throw(Error("ERROR: expect 2 valid register number"));
    }
    it = it2;
}


void loadOrStoreWord(vector<Token>::iterator &it, vector<Token>::iterator end, string &op) {
    int64_t regs, regt, val;
    auto it2 = it;
    for (int i = 0; i < 6; ++i, ++it) {
        if (it->getKind() == Token::REG && i == 0) regt = it->toLong();
        else if (it->getKind() == Token::REG && i == 4) regs = it->toLong(); 
        else if ((it->getKind() == Token::COMMA && i == 1) ||
            (it->getKind() == Token::LPAREN && i == 3)) {
            continue;
        } else if (it->getKind() == Token::RPAREN && i == 5) {
            it2 = it;
        } else if (it->getKind() == Token::INT && i == 2) {
            val = it->toLong();
            if (-32768 > val || val > 32767) {
                throw(Error("ERROR: int out of bound"));
            }
        } else if (it->getKind() == Token::HEXINT && i == 2) {
            val = it->toLong();
            if (val > 0xffff) {
                throw(Error("ERROR: int out of bound"));
            }
        } else {
            throw(Error("ERROR: expected syntax: lw $t, i($s) or sw $t, i($s)"));
        }
    }
    if ((0 <= regs && regs <= 31) && (0 <= regt && regt <= 31) && (it == end)) {
        if (op == "lw") printBinary(35, regs, regt, 0, val);
        if (op == "sw") printBinary(43, regs, regt, 0, val);
    } else {
        throw(Error("ERROR: expect 2 valid registers"));
    }
    it = it2;
}


int main() {
	string line;
	int count = 0;
	map<string, int> stable;
    vector<vector<Token>> code;
    bool allLabel = true;

	try {
		while (getline(std::cin, line)) {
            vector<Token> tokenLine = scan(line);
			for (auto it = tokenLine.begin(); it != tokenLine.end(); ++it) {
                if (it->getKind() != Token::LABEL) allLabel = false;
				if (it->getKind() == Token::LABEL) {
                    string label {it->getLexeme()};
                    label.erase(label.size() - 1);
                    if (stable.count(label) == 0) {
					    stable[label] = count;
                    } else {
                        throw(Error("ERROR: duplicated label."));
                    }
                }
            }
            if (line != "" && !allLabel) count += 4;
            allLabel = true;
            code.push_back(tokenLine);
		}
        count = 0;
        for(auto token = code.begin(); token != code.end(); ++token) {
            allLabel = true;
            for (auto it = token->begin(); it != token->end(); ++it) {
                Token::Kind kind{ it->getKind() };
                if (kind != Token::LABEL) {
                    allLabel = false;
                }
				if (kind == Token::WORD) {
                    word(it, token->end(), stable);    
                } else if (kind == Token::LABEL) {
                    continue;
                } else if (kind == Token::ID) {
                    string op = it->getLexeme();
                    ++it;
                    if (op == "jr" || op == "jalr") {
                        oneArg(it, token->end(), op);
                    } else if (op == "lis" || op == "mflo" || op == "mfhi") {
                        oneArg(it, token->end(), op);
                    } else if (op == "add" || op == "sub" || op == "slt" || op == "sltu") {
                        fiveArg(it, token->end(), op);
                    } else if (op == "beq" || op == "bne") {
                        brancheq(it, token->end(), op, stable, count);
                    } else if (op == "mult" || op == "multu" || op == "div" || op == "divu") {
                        twoArg(it, token->end(), op);
                    } else if (op == "lw" || op == "sw") {
                        loadOrStoreWord(it, token->end(), op);
                    } else {
                        throw(Error("ERROR: invalid code"));
                    }
                } else {
                    throw(Error("ERROR: invalid code"));
                }
            }
            count += 4;
            if (allLabel) count -= 4;
        }
    } catch (ScanningFailure &f) {
		cerr << f.what() << endl;
		return 1;
	} catch (Error &e) {
        cerr << e.what() << endl;
        return 1;
    }
    /*for (auto &label : stable) {
        cerr << label.first << ' ' << label.second << endl;
    }*/
	return 0;
}

