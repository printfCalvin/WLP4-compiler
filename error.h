#ifndef ERROR_H
#define ERROR_H
#include <string>


class Error {
    const std::string message;
public:
    Error(std::string message) : message{message} {}
    const std::string what() const {
        return message;
    }
};

#endif
