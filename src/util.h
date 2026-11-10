#ifndef FACTORY_UTIL_H
#define FACTORY_UTIL_H

#include <iostream>
#include <ostream>
#include <fstream>
#include <iterator>
#include <string>

using namespace std;

enum LogType {
    INFO,
    WARN,
    ERRO,
};

namespace util {
    inline void log(LogType l, const char* message) {
        const char* prefix = "INFO";

        switch (l) {
            case WARN: prefix = "WARN";
            case ERRO: prefix = "ERRO";
        }

        cout << "[" << prefix << "] " << message << endl;
    }

    inline string readFile(const char* path) {
        std::fstream file(path);

        if (!file.is_open()) {
            log(WARN, ("Failed to open file at location '" + std::string(path) + "'").c_str());
            return "";
        }

        return std::string((istreambuf_iterator<char>(file)), (istreambuf_iterator<char>()));
    }
}

#endif //FACTORY_UTIL_H