#include "CampusCompass.h"

#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

CampusCompass::CampusCompass() {
    // initialize your object
}

int CampusCompass::parseTimeToMinutes(const string &timeStr) const{
    size_t colonPos = timeStr.find(':');
    if(colonPos == string::npos){
        return -1;
    }

    int hours = stoi(timeStr.substr(0, colonPos));
    int minutes = stoi(timeStr.substr(colonPos + 1));
    return hours * 60 + minutes;
}

bool CampusCompass::isValidName(const string &name) const{
    if(name.empty()){
        return false;
    }

    for(size_t i = 0; i < name.size(); i++){
        char c = name[i];
        if(!isalpha(static_cast<unsigned char>(c)) && c != ' '){
            return false;
        }
    }
    return true;
}

bool isValidUFID(const string &ufid) const{
    if(ufid.size() != 8){
        return false;
    }

    for(size_t i = 0; i < ufid.size(); i++){
        if(!isdigit(static_cast<unsigned char>(ufid[i]))){
            return false;
        }
    }
    return true;
}

bool CampusCompass::isValidClassCode(const string &code) const{
    if(code.size() != 7){
        return false;
    }

    for(size_t i = 0; i < 3; i++){
        if(code[i] < 'A' || code[i] > 'Z'){
            return false;
        }
    }

    for(size_t i = 3; i < 7; i++){
        if(!isdigit(static_cast<unsigned char>(code[i]))){
            return false;
        }
    }
    return true;
}

bool CampusCompass::ParseCSV(const string &edges_filepath, const string &classes_filepath) {
    // return boolean based on whether parsing was successful or not
    return true;
}

bool CampusCompass::ParseCommand(const string &command) {
    // do whatever regex you need to parse validity
    // hint: return a boolean for validation when testing. For example:
    bool is_valid = true; // replace with your actual validity checking

    return is_valid;
}
