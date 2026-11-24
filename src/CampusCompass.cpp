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

bool CampusCompass::isValidUFID(const string &ufid) const{
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
    // 1. Parse edges.csv
    ifstream edgesFile(edges_filepath);
    if(!edgesFile.is_open()){
        return false;
    }

    string line;
    getline(edgesFile, line); // Skip header

    while(getline(edgesFile, line)){
        if(!line.empty() && line.back() == '\r'){
            line.pop_back();
        }
        if(line.empty()){
            continue;
        }

        stringstream ss(line);
        string id1Str, id2Str, name1, name2, timeStr;
        if(getline(ss, id1Str, ',') &&
           getline(ss, id2Str, ',') &&
           getline(ss, name1, ',') &&
           getline(ss, name2, ',') &&
           getline(ss, timeStr)){
            
            int id1 = stoi(id1Str);
            int id2 = stoi(id2Str);
            int timeWeight = stoi(timeStr);

            locationNames[id1] = name1;
            locationNames[id2] = name2;

            // Undirected graph: store both directions
            adjList[id1].push_back({id2, timeWeight, true});
            adjList[id2].push_back({id1, timeWeight, true});
        }
    }

    edgesFile.close();

    // 2. Parse classes.csv
    ifstream classesFile(classes_filepath);
    if(!classesFile.is_open()){
        return false;
    }

    getline(classesFile, line); //skip header row

    while(getline(classesFile, line)){
        if(!line.empty() && line.back() == 'r'){
            line.pop_back();
        }

        if(line.empty()){
            continue;
        }

        stringstream ss(line);
        string classCode, locIdStr, startTimeStr, endTimeStr;

        if(getline(ss, classCode, ',') &&
           getline(ss, locIdStr, ',') &&
           getline(ss, startTimeStr, ',') &&
           getline(ss, endTimeStr)){
            
            ClassInfo info;
            info.locationId = stoi(locIdStr);
            info.startMinutes = parseTimeToMinutes(startTimeStr);
            info.endMinutes = parseTimeToMinutes(endTimeStr);

            classMap[classCode] = info;
        }
    }
    classesFile.close();

    return true;
}

bool CampusCompass::insert(const string &name, const string &ufid, int residenceId, const vector<string> &classes) {
    if(!isValidName(name) || isValidUFID(ufid)){
        cout << "unsuccessful\n";
        return false;
    }

    if(students.find(ufid) != students.end()){
        cout << "unsuccessful\n";
        return false;
    }

    if(locationNames.find(residenceId) == locationNames.end()){
        cout << "unsuccessful\n";
        return false;
    }

    if(classes.size() < 1 || classes.size() > 6){
        cout << "unsuccessful\n";
        return false;
    }

    set<string> uniqueClasses;
    for(size_t i = 0; i < classes.size(); i++){
        const string &code = classes[i];
        if(!isValidClassCode(code) || classMap.find(code) == classMap.end()){
            cout << "unsuccessful\n";
            return false;
        }
        uniqueClasses.insert(code);
    }

    if(uniqueClasses.size() != classes.size()){
        cout << "unsuccessful\n";
        return false;
    }

    Student newStudent;
    newStudent.name = name;
    newStudent.ufid = ufid;
    newStudent.residenceId = residenceId;
    newStudent.classes = uniqueClasses;

    students[ufid] = newStudent;
    cout << "successful\n";
    return true;
}

bool CampusCompass::remove(const string &ufid){
    if(!isValidUFID(ufid)){
        cout << "unsuccessful\n";
        return false;
    }

    unordered_map<string, Student>::iterator it = students.find(ufid);
    if(it == students.end()){
        cout << "unsuccessful\n";
        return false;
    }

    students.erase(it);
    cout << "successful\n";
    return true;
}

bool CampusCompass::dropClass(const string &ufid, const string &classCode){
    if(!isValidUFID(ufid) || !isValidClassCode(classCode) || classMap.find(classCode) == classMap.end()){
        cout << "unsuccessful\n";
        return false;
    }

    unordered_map<string, Student>::iterator it = students.find(ufid);
    if(it == students.end()){
        cout << "unsuccessful\n";
        return false;
    }

    if(it->second.classes.erase(classCode) == 0){
        cout << "unsuccessful\n";
        return false;
    }

    // Drop student completely if they have 0 classes left
    if(it->second.classes.empty()){
        students.erase(it);
    }

    cout << "successful\n";
    return true;
}

bool CampusCompass::replaceClass(const string &ufid, const string &oldCode, const string &newCode){
    if(!isValidUFID(ufid) || !isValidClassCode(oldCode) || !isValidClassCode(newCode)){
        cout << "unsuccessful\n";
        return false;
    }

    if(classMap.find(newCode) == classMap.end()){
        cout << "unsuccessful\n";
        return false;
    }

    unordered_map<string, Student>::iterator it = students.find(ufid);
    if(it == students.end()){
        cout << "unsuccessful\n";
        return false;
    }

    set<string> &studentClasses = it->second.classes;
    if(studentClasses.find(oldCode) == studentClasses.end() ||
       studentClasses.find(newCode) != studentClasses.end()){
        cout << "unsuccessful\n";
        return false;
    }

    studentClasses.erase(oldCode);
    studentClasses.insert(newCode);
    cout << "successful\n";
    return true;
}

bool CampusCompass::removeClass(const string &classCode){
    if(!isValidClassCode(classCode) || classMap.find(classCode) == classMap.end()){
        cout << "unsuccessful\n";
        return false;
    }

    int droppedCount = 0;
    unordered_map<string, Student>::iterator it = students.begin();
    while(it != students.end()){
        if(it->second.classes.erase(classCode) > 0){
            droppedCount++;
            if(it->second.classes.empty()){
                it = students.erase(it);
                continue
            }
        }
        ++it;
    }

    if(droppedCount == 0){
        cout << "unsuccessful\n";
        return false;
    }

    cout << "successful\n";
    return true;
}

bool CampusCompass::ParseCommand(const string &command) {
    // do whatever regex you need to parse validity
    // hint: return a boolean for validation when testing. For example:
    bool is_valid = true; // replace with your actual validity checking

    return is_valid;
}
