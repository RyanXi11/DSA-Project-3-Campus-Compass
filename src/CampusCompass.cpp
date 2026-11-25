#include "CampusCompass.h"

#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <queue>
#include <unordered_set>
#include <functional>
#include <algorithm>

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
        if(!line.empty() && line.back() == '\r'){
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
    if(!isValidName(name) || !isValidUFID(ufid)){
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
                continue;
            }
        }
        ++it;
    }

    if(droppedCount == 0){
        cout << "unsuccessful\n";
        return false;
    }

    cout << droppedCount << "\n";
    return true;
}

bool CampusCompass::toggleEdgesClosure(const vector<pair<int, int>> &edges){
    for(size_t i = 0; i < edges.size(); i++){
        int u = edges[i].first;
        int v = edges[i].second;

        // Toggle u -> v
        unordered_map<int, vector<Edge>>::iterator itU = adjList.find(u);
        if(itU != adjList.end()){
            vector<Edge> &neighborsU = itU->second;
            for(size_t j = 0; j < neighborsU.size(); j++){
                if(neighborsU[j].to == v){
                    neighborsU[j].isOpen = !neighborsU[j].isOpen;
                    break;
                }
            }
        }

        // Toggle v -> u
        unordered_map<int, vector<Edge>>::iterator itV = adjList.find(v);
        if(itV != adjList.end()){
            vector<Edge> &neighborsV = itV->second;
            for(size_t j = 0; j < neighborsV.size(); j++){
                if(neighborsV[j].to == u){
                    neighborsV[j].isOpen = !neighborsV[j].isOpen;
                    break;
                }
            }
        }
    }

    cout << "successful\n";
    return true;
}

bool CampusCompass::checkEdgeStatus(int loc1, int loc2) const{
    unordered_map<int, vector<Edge>>::const_iterator it = adjList.find(loc1);
    if(it != adjList.end()){
        const vector<Edge> &neighbors = it->second;
        for(size_t i = 0; i < neighbors.size(); i++){
            if(neighbors[i].to == loc2){
                if(neighbors[i].isOpen){
                    cout << "open\n";
                }
                else
                {
                    cout << "closed\n";
                }
                return true;
            }
        }
    }

    cout << "DNE\n";
    return true;
}

bool CampusCompass::isConnected(int loc1, int loc2) const{
    if(adjList.find(loc1) == adjList.end() || adjList.find(loc2) == adjList.end()){
        cout << "unsuccessful\n";
        return false;
    }
    if(loc1 == loc2){
        cout << "successful\n";
        return true;
    }

    queue<int> q;
    unordered_set<int> visited;

    q.push(loc1);
    visited.insert(loc1);

    while(!q.empty()){
        int current = q.front();
        q.pop();

        unordered_map<int, vector<Edge>>::const_iterator it = adjList.find(current);
        if(it == adjList.end()){
            continue;
        }

        const vector<Edge> &neighbors = it->second;
        for(size_t i = 0; i < neighbors.size(); i++){
            if(!neighbors[i].isOpen){
                continue;
            }
            int nextNode = neighbors[i].to;
            if(nextNode == loc2){
                cout << "successful\n";
                return true;
            }
            if(visited.find(nextNode) == visited.end()){
                visited.insert(nextNode);
                q.push(nextNode);
            }
        }
    }

    cout << "unsuccessful\n";
    return false;
}

void CampusCompass::runDijkstra(int startNode, unordered_map<int, int> &dist, unordered_map<int, int> &parent) const{
    // Min-heap storing distance {distance, nodeId}
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;

    dist[startNode] = 0;
    pq.push(make_pair(0, startNode));

    while(!pq.empty()){
        int currentDist = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        // Skip stale entries
        if(currentDist > dist[u]){
            continue;
        }

        unordered_map<int, vector<Edge>>::const_iterator it = adjList.find(u);
        if(it == adjList.end()){
            continue;
        }

        const vector<Edge> &neighbors = it->second;
        for(size_t i = 0; i < neighbors.size(); i++){
            if(!neighbors[i].isOpen){
                continue;
            }

            int v = neighbors[i].to;
            int newDist = currentDist + neighbors[i].weight;

            if(dist.find(v) == dist.end() || newDist < dist[v]){
                dist[v] = newDist;
                parent[v] = u;
                pq.push(make_pair(newDist, v));
            }
        }
    }
}

void CampusCompass::printShortestEdges(const string &ufid) const{
    unordered_map<string, Student>::const_iterator sIt = students.find(ufid);
    if(sIt == students.end()){
        return;
    }

    const Student &student = sIt->second;
    unordered_map<int, int> dist;
    unordered_map<int, int> parent;
    runDijkstra(student.residenceId, dist, parent);

    cout << "Time For Shortest Edges: " << student.name << "\n";

    // student.classes is a set, so it is already sorted lexicographically
    for(set<string>::const_iterator cIt = student.classes.begin(); cIt != student.classes.end(); ++cIt){
        const string &code = *cIt;
        int classLoc = classMap.at(code).locationId;

        unordered_map<int, int>::const_iterator dIt = dist.find(classLoc);
        if(dIt == dist.end()){
            cout << code << ": -1\n";
        }else{
            cout << code << ": " << dIt->second << "\n";
        }
    }
}

void CampusCompass::printStudentZone(const string &ufid) const{
    unordered_map<string, Student>::const_iterator sIt = students.find(ufid);
    if(sIt == students.end()){
        return;
    }

    const Student &student = sIt->second;
    unordered_map<int, int> dist;
    unordered_map<int, int> parent;
    runDijkstra(student.residenceId, dist, parent);

    // 1. Collect all vertices along the shortest paths from residence to reachable classes
    unordered_set<int> zoneNodes;
    zoneNodes.insert(student.residenceId);
    
    for(set<string>::const_iterator cIt = student.classes.begin(); cIt != student.classes.end(); ++cIt){
        int classLoc = classMap.at(*cIt).locationId;
        if(dist.find(classLoc) == dist.end()){
            continue; // Unreachable class
        }

        int curr = classLoc;
        while(curr != student.residenceId){
            zoneNodes.insert(curr);
            curr = parent[curr];
        }
    }

    // 2. Compute MST of the induced subgraph using Prim's algorithm
    // Min-heap storing {edgeWeight, nodeId}
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    unordered_set<int> inMST;
    int totalCost = 0;

    pq.push(make_pair(0, student.residenceId));

    while(!pq.empty() && inMST.size() < zoneNodes.size()){
        int weight = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        if(inMST.find(u) != inMST.end()){
            continue;
        }

        inMST.insert(u);
        totalCost += weight;

        unordered_map<int, vector<Edge>>::const_iterator it = adjList.find(u);
        if(it == adjList.end()){
            continue;
        }

        const vector<Edge> &neighbors = it->second;
        for(size_t i = 0; i < neighbors.size(); i++){
            int v = neighbors[i].to;
            // Only use open edges that connect two nodes inside zoneNodes
            if(neighbors[i].isOpen &&
                zoneNodes.find(v) != zoneNodes.end() &&
                inMST.find(v) == inMST.end()){
                    pq.push(make_pair(neighbors[i].weight, v));
            }
        }
    }

    cout << "Student Zone Cost For " << student.name << ": " << totalCost << "\n";
}

void CampusCompass::verifySchedule(const string &ufid) const{
    unordered_map<string, Student>::const_iterator sIt = students.find(ufid);
    if(sIt == students.end()){
        cout << "unsuccessful\n";
        return;
    }

    const Student &student = sIt->second;
    if(student.classes.size() <= 1){
        cout << "unsuccessful\n";
        return;
    }

    // Collect the student's classes into a vector so we can sort by start time
    vector<string> sortedClasses(student.classes.begin(), student.classes.end());
    sort(sortedClasses.begin(), sortedClasses.end(),
        [this](const string &a, const string &b){
            int startA = classMap.at(a).startMinutes;
            int startB = classMap.at(b).startMinutes;
            if(startA != startB){
                return startA < startB;
            }
            return a < b; // Tie-breaker: lexicographical order
        });

    cout << "Schedule Check for " << student.name << ":\n";

    for(size_t i = 0; i + 1 < sortedClasses.size(); i++){
        const string &code1 = sortedClasses[i];
        const string &code2 = sortedClasses[i + 1];

        const ClassInfo &info1 = classMap.at(code1);
        const ClassInfo &info2 = classMap.at(code2);
        
        int timeGap = info2.startMinutes - info1.endMinutes;

        // Find shortest path from class 1's location to class 2's location using open edges
        unordered_map<int, int> dist;
        unordered_map<int, int> parent;
        runDijkstra(info1.locationId, dist, parent);

        unordered_map<int, int>::const_iterator dIt = dist.find(info2.locationId);
        bool canMakeIt = (dIt != dist.end() && timeGap >= dIt->second);

        cout << code1 << " - " << code2 << ": "
             << (canMakeIt ? "successful" : "unsuccessful") << "\n";
    }
}

bool CampusCompass::ParseCommand(const string &command) {
    string cleanCmd = command;
    if (!cleanCmd.empty() && cleanCmd.back() == '\r')
    {
        cleanCmd.pop_back();
    }

    istringstream ss(cleanCmd);
    string action;
    if (!(ss >> action))
    {
        cout << "unsuccessful\n";
        return false;
    }

    if (action == "insert")
    {
        ss >> ws;
        if (ss.peek() != '"')
        {
            cout << "unsuccessful\n";
            return false;
        }
        ss.get(); // consume opening quote

        string name;
        if (!getline(ss, name, '"'))
        {
            cout << "unsuccessful\n";
            return false;
        }

        string ufid;
        int residenceId;
        int numClasses;
        if (!(ss >> ufid >> residenceId >> numClasses) || numClasses < 1 || numClasses > 6)
        {
            cout << "unsuccessful\n";
            return false;
        }

        vector<string> classes;
        string code;
        for (int i = 0; i < numClasses; i++)
        {
            if (!(ss >> code))
            {
                cout << "unsuccessful\n";
                return false;
            }
            classes.push_back(code);
        }

        string extra;
        if (ss >> extra)
        {
            cout << "unsuccessful\n";
            return false;
        }

        return insert(name, ufid, residenceId, classes);
    }
    else if (action == "remove")
    {
        string ufid, extra;
        if (!(ss >> ufid) || (ss >> extra))
        {
            cout << "unsuccessful\n";
            return false;
        }
        return remove(ufid);
    }
    else if (action == "dropClass")
    {
        string ufid, classCode, extra;
        if (!(ss >> ufid >> classCode) || (ss >> extra))
        {
            cout << "unsuccessful\n";
            return false;
        }
        return dropClass(ufid, classCode);
    }
    else if (action == "replaceClass")
    {
        string ufid, oldCode, newCode, extra;
        if (!(ss >> ufid >> oldCode >> newCode) || (ss >> extra))
        {
            cout << "unsuccessful\n";
            return false;
        }
        return replaceClass(ufid, oldCode, newCode);
    }
    else if (action == "removeClass")
    {
        string classCode, extra;
        if (!(ss >> classCode) || (ss >> extra))
        {
            cout << "unsuccessful\n";
            return false;
        }
        return removeClass(classCode);
    }
    else if (action == "toggleEdgesClosure")
    {
        int n;
        if (!(ss >> n) || n <= 0)
        {
            cout << "unsuccessful\n";
            return false;
        }

        vector<pair<int, int>> edges;
        for (int i = 0; i < n; i++)
        {
            int u, v;
            if (!(ss >> u >> v))
            {
                cout << "unsuccessful\n";
                return false;
            }
            edges.push_back(make_pair(u, v));
        }

        string extra;
        if (ss >> extra)
        {
            cout << "unsuccessful\n";
            return false;
        }

        return toggleEdgesClosure(edges);
    }
    else if (action == "checkEdgeStatus")
    {
        int loc1, loc2;
        string extra;
        if (!(ss >> loc1 >> loc2) || (ss >> extra))
        {
            cout << "unsuccessful\n";
            return false;
        }
        return checkEdgeStatus(loc1, loc2);
    }
    else if (action == "isConnected")
    {
        int loc1, loc2;
        string extra;
        if (!(ss >> loc1 >> loc2) || (ss >> extra))
        {
            cout << "unsuccessful\n";
            return false;
        }
        return isConnected(loc1, loc2);
    }
    else if (action == "printShortestEdges")
    {
        string ufid, extra;
        if (!(ss >> ufid) || (ss >> extra))
        {
            cout << "unsuccessful\n";
            return false;
        }
        printShortestEdges(ufid);
        return true;
    }
    else if (action == "printStudentZone")
    {
        string ufid, extra;
        if (!(ss >> ufid) || (ss >> extra))
        {
            cout << "unsuccessful\n";
            return false;
        }
        printStudentZone(ufid);
        return true;
    }
    else if (action == "verifySchedule")
    {
        string ufid, extra;
        if (!(ss >> ufid) || (ss >> extra))
        {
            cout << "unsuccessful\n";
            return false;
        }
        verifySchedule(ufid);
        return true;
    }

    cout << "unsuccessful\n";
    return false;
}
