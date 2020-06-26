#include <iostream>
#include <string>
#include <vector>
#include "scanner.h"

/*
 * C++ Starter code for CS241 A3
 * All code requires C++14, so if you're getting compile errors make sure to
 * use -std=c++14.
 *
 * This file contains the main function of your program. By default, it just
 * prints the scanned list of tokens back to standard output.
 */
int main() {
  std::string line;

  try {
    while (getline(std::cin, line)) {
      // For example, just print the scanned tokens
      std::vector<Token> tokenLine = scan(line);
      for (auto &token : tokenLine) {
        std::cout << token << ' ';
      }
      std::cout << std::endl;
    }
  } catch (ScanningFailure &f) {
    std::cerr << f.what() << std::endl;

    return 1;
  }

  return 0;
}
