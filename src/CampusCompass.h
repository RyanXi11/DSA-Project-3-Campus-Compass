#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <utility>

using namespace std;

class CampusCompass {
private:
    // Think about what member variables you need to initialize
    // perhaps some graph representation?
    struct Edge{
        int to;
        int weight;
        bool isOpen;
    };

    struct ClassInfo{
        int locationId;
        int startMinutes;
        int endMinutes;
    };

    struct Student{
        string name;
        string ufid;
        int residenceId;
        set<string> classes;
    };

    unordered_map<int, vector<Edge>> adjList; // Graph adjacency list: locationId -> list of outgoing edges
    unordered_map<int, string> locationNames; // Location ID -> Location Name
    unordered_map<string, ClassInfo> classMap; // ClassCode -> ClassInfo
    unordered_map<string, Student> students; // UFID -> Student

    // Validation helpers
    bool isValidName(const string &name) const;
    bool isValidUFID(const string &ufid) const;
    bool isValidClassCode(const string &code) const;
    int parseTimeToMinutes(const string &timeStr) const;

    // Student and Class Management
    bool insert(const string &name, const string &ufid, int residenceId, const vector<string> &classes);
    bool remove(const string &ufid);
    bool dropClass(const string &ufid, const string &classCode);
    bool replaceClass(const string &ufid, const string &oldCode, const string &newCode);
    bool removeClass(const string &classCode);

    // Graph and Pathfinding
    bool toggleEdgesClosure(const vector<pair<int, int>> &edges);
    bool checkEdgeStatus(int loc1, int loc2) const;
    bool isConnected(int loc1, int loc2) const;
    void printShortestEdges(const string &ufid) const;
    void printStudentZone(const string &ufid) const;
    void verifySchedule(const string &ufid) const;

public:
    // Think about what helper functions you will need in the algorithm
    CampusCompass(); // constructor
    bool ParseCSV(const string &edges_filepath, const string &classes_filepath);
    bool ParseCommand(const string &command);
};
