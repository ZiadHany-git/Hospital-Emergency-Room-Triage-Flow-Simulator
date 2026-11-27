#include <iostream>
#include <cmath>
#include <fstream>
#include <string>
#include <json.hpp>

using namespace std;
using json = nlohmann::json;

struct EventData {      //* we see this struct for the first time in the ERsimulator and we use it to store the events 
    string type;
    int TS = 0;
    int PID = 0;
    int ESI = 0;
    string zone = "";   
    int Maxwait = 0;
    int TD = 0;
    int testduration = 0;
};

class Patient {
public:
    int PID;
    int AT;
    int ESI;
    int currentESI;
    string zone;
    int Maxwait;
    int TD;
    int remainingTD = 0;  
    int remainingDiag = 0;  
    int remainingTriage = 0;   
    int TT = -1; // Triage Time 
    int BT = -1; // Bed/Bay Time  
    int DT = -1; // Departure Time   
    int WT = 0;  // Waiting Time
    int ST = 0;  // Service Time
    bool escalated = false;         
    string status = "Waiting_Triage";   //* by default 

    Patient(int id, int arrival, int acuity, const string& z, int mw, int treatDur)  //! here we pass the values in the constructor to the variables in the class
        : PID(id), AT(arrival), ESI(acuity), currentESI(acuity), zone(z),
          Maxwait(mw), TD(treatDur), remainingTD(treatDur) {}   //* this constructor just for passing the values to the class 

    float getPriorityScore(int currentTime, int escalationThreshold, int alpha = 20) const {   //! why we used const here 
        int escalations = 0;
        if (escalationThreshold > 0 && currentTime > AT) {
            escalations = (currentTime - AT) / escalationThreshold;   //* here we consider the case that the escalationthreshold =0 
        }                                                             //* you should demostrate this part in the instructnions file
        return (6 - currentESI) * 100.0f + alpha * escalations;
    }
};

// FIFO Queue (Circular Array) for Triage / general FIFO usage
class FIFOQueue {
private:
    Patient** arr;  
    int capacity;
    int frontIdx;
    int rearIdx;
    int count;

public:
    FIFOQueue(int cap = 100) {   //* here the 100 is default value if we didn't pass a value to the constructor and not the maximum value 
        capacity = (cap > 0) ? cap : 100;    //* ternary operator
        arr = new Patient*[capacity];
        frontIdx = 0;   
        rearIdx = -1;
        count = 0;
    }

    ~FIFOQueue() {
        delete[] arr;    //* deallocating the pointers 
    }

    bool isFull() const { return count == capacity; }
    bool isEmpty() const { return count == 0; }
    int size() const { return count; }

    void enqueue(Patient* p) {   //* here we must pass an object of the class patient
        if (isFull()) {
            cout << "there is no place\n";
            return;
        }
        rearIdx = (rearIdx + 1) % capacity;
        arr[rearIdx] = p;     //* here we store the address of the first pointer in the arr pointer so we can point to p using arr
        count++;
    }

    Patient* dequeue() {  //* we intialize the fun using patient* because we want to return the pointer which points to an object of type patient
        if (isEmpty()) {  //* in this function we just return the pointer of the patient (we dont delete it physically)
            cout << "the queue is empty\n";
            return nullptr;   //* focus here 
        }
        Patient* p = arr[frontIdx];
        frontIdx = (frontIdx + 1) % capacity;
        count--;
        return p;   //* here we returned the pointer not the object
    }

    Patient* peekAt(int idx) const {   //* peeking the pointer in a specefic index
        if (idx < 0 || idx >= count) return nullptr;
        return arr[(frontIdx + idx) % capacity]; 
    }

    void removePatient(Patient* target) {   //! look for this function in the other lines and you will understand it 
        if (isEmpty() || !target) return;   //* ! target means here the pointer = nullptr
        FIFOQueue temp(capacity);
        while (!isEmpty()) {
            Patient* p = dequeue();
            if (p != target) {
                temp.enqueue(p);
            }
        }
        while (!temp.isEmpty()) {
            enqueue(temp.dequeue());
        }
    }
};

// Priority Queue (Array-based) ordered by PriorityScore, then earlier AT, then smaller PID
class PriorityQueue {         //* it's just an array holds pointer to point to the object of patients and sorting them using the indeces
private:
    Patient** arr;
    int capacity;
    int count;

    bool isHigherPriority(Patient* a, Patient* b, int currentTime, int escThreshold) const {
        float pa = a->getPriorityScore(currentTime, escThreshold);
        float pb = b->getPriorityScore(currentTime, escThreshold);
        if (pa != pb) return pa > pb;
        if (a->AT != b->AT) return a->AT < b->AT;
        return a->PID < b->PID;
    }

public:
    PriorityQueue(int cap = 100) {
        capacity = (cap > 0) ? cap : 100;
        arr = new Patient*[capacity];
        count = 0;
    }

    ~PriorityQueue() {
        delete[] arr;
    }

    bool isEmpty() const { return count == 0; }
    int size() const { return count; }

    void enqueue(Patient* p) {
        if (count >= capacity) {
            cout << "Priority queue is full!\n";
            return;
        }
        arr[count++] = p;
    }

    Patient* dequeueBest(int currentTime, int escThreshold) {
        if (isEmpty()) return nullptr;
        int bestIdx = 0;  
        for (int i = 1; i < count; i++) { //* we start from one because we put by default bestindex = 0 
            if (isHigherPriority(arr[i], arr[bestIdx], currentTime, escThreshold)) {
                bestIdx = i;
            }
        }
        Patient* best = arr[bestIdx];
        for (int i = bestIdx; i < count - 1; i++) { //* this loop shifts all the items in the priority queue after the bestinex as we want to dequeue the
            arr[i] = arr[i + 1];       //* item in the best index so we can delete it from the queue by just shifting the items in the array
        }
        count--;     
        return best;  //* return the pointer that points to the object that have the higher priority
    }

    Patient* peekAt(int idx) const {
        if (idx < 0 || idx >= count) return nullptr;    //* as the last element = count - 1
        return arr[idx];
    }

    void removePatient(Patient* target) {
        for (int i = 0; i < count; i++) {
            if (arr[i] == target) {
                for (int j = i; j < count - 1; j++) {   //* deleting the pointer from the array by shifting the items 
                    arr[j] = arr[j + 1];
                }
                count--;
                return;
            }
        }
    }
};

class TreatmentBay {
public:
    int id;
    string zone;
    string status = "Free"; // Free, Occupied, Cleaning
    Patient* patient = nullptr;
    int remainingTime = 0;
    int occupiedSteps = 0;

    void assignPatient(Patient* p, int currentTime) {
        patient = p;
        status = "Occupied";
        remainingTime = p->remainingTD;
        p->status = "In_Treatment";
        if (p->BT == -1) {  //* this condition occurs only in the first time the patient enter the bay 
            p->BT = currentTime;
            p->WT = p->BT - p->AT;
        }
    }

    void startCleaning(int cleaningTime = 2) {
        patient = nullptr;
        status = "Cleaning";
        remainingTime = cleaningTime;
    }
};

class Clinician {
public:
    int id;
    string role; // TriageNurse or Physician
    string status = "Available"; // Available, Busy, On_Break
    Patient* currentPatient = nullptr;
    int handledCount = 0;   //* the number of the patients that the clinician dealed with 
    int breakTimer = 0;   //* we track the time of the break using this variable and it equals breakduration in the input file 
    int busySteps = 0;  //* counter to calcutate the utilization of the clinician and know it he needs a break or not 
};

class ERSimulator {
public:
    json config;  //*the name of the input data file 
    json output; //* the name of the output data file 

    int currentTime = 0;  //* the golbal clock
    int nResus = 0, nAcute = 0, nFast = 0;  //* number of bays 
    int nNurses = 0, nPhysicians = 0;  //* we put initial values of these variables in the case there was an error in reading the file so the program 
    int escalationThreshold = 15;       //* don't crash and operate on valid values 
    int breakInterval = 10;
    int breakDuration = 9;
    int alpha = 20;

    EventData* events = nullptr;
    int eventCount = 0;
    int nextEventIdx = 0;

    TreatmentBay* bays = nullptr;
    int bayCount = 0;

    Clinician* nurses = nullptr;
    Clinician* doctors = nullptr;

    Patient** activePatients = nullptr;
    int activeCount = 0;
    int activeCap = 200;

    Patient** completedPatients = nullptr;
    int completedCount = 0;
    int completedCap = 200;

    FIFOQueue triageQueue;
    PriorityQueue resusQueue;
    PriorityQueue acuteQueue;
    PriorityQueue fastQueue;

    int escalatedCount = 0;   //* counter to know how many patients has an increased priority score 

    ERSimulator() : triageQueue(200), resusQueue(200), acuteQueue(200), fastQueue(200) {
        activePatients = new Patient*[activeCap];
        completedPatients = new Patient*[completedCap];
    }

    ~ERSimulator() {
        delete[] events;
        delete[] bays;
        delete[] nurses;
        delete[] doctors;
        for (int i = 0; i < activeCount; i++) delete activePatients[i];
        for (int i = 0; i < completedCount; i++) delete completedPatients[i];
        delete[] activePatients;
        delete[] completedPatients;
    }

    void loadAndSortEvents() {
        nResus = config.value("N_RESUS", 0);  //* the program read the value from the file and if it's not there the value = 0 
        nAcute = config.value("N_ACUTE", 0);
        nFast  = config.value("N_FAST", 0);
        nNurses = config.value("N_NURSES", 0);
        nPhysicians = config.value("N_Physicians", 0);

        escalationThreshold = config.value("EscalationThreshold", 15);
        breakInterval = config.value("BreakInterval", 10);   //* break interval is a limit to know if the clinician needs a break or not 
        breakDuration = config.value("BreakDuration", 9);

        bayCount = nResus + nAcute + nFast;
        bays = new TreatmentBay[bayCount];
        int idx = 0;
        for (int i = 0; i < nResus; i++) bays[idx++] = TreatmentBay{i + 1, "RESUS"};  //* Aggregate initialization look at the class variables in order
        for (int i = 0; i < nAcute; i++) bays[idx++] = TreatmentBay{i + 1, "ACUTE"};  //* i +1 is the id 
        for (int i = 0; i < nFast;  i++) bays[idx++] = TreatmentBay{i + 1, "FAST"};

        nurses = new Clinician[nNurses];
        for (int i = 0; i < nNurses; i++) nurses[i] = Clinician{i + 1, "TriageNurse"};

        doctors = new Clinician[nPhysicians];
        for (int i = 0; i < nPhysicians; i++) doctors[i] = Clinician{i + 1, "Physician"};

        eventCount = config.value("M", 0);
        events = new EventData[eventCount];

        for (int i = 0; i < eventCount; i++) {
            const auto& ev = config["events"][i];   //* or we can use -> const json & ev = config ["events"][i]; 
 //* if we didn't refrence to the config["events"][i] so in each line we will use config["events"][i]["type"] for example refrencing made it easier
            events[i].type = ev.value("type", "");   //* here this is a common data between the types of the events we can type them outside the conditions
            events[i].TS   = ev.value("TS", 0);
            events[i].PID  = ev.value("PID", 0);

            if (events[i].type == "A") {    //* A event own data
                events[i].ESI     = ev.value("ESI", 5);
                events[i].zone    = ev.value("zone", "ACUTE");
                events[i].Maxwait = ev.value("Maxwait", 999);
                events[i].TD      = ev.value("TD", 1);
           } else if (events[i].type == "D") {  //* D event own data
            events[i].testduration = ev.value("testduration", 1);
               events[i].TD = ev.value("TD", 1);
                 }
        }

        
        for (int i = 0; i < eventCount - 1; i++) { //* sorting the events in the array with TS 
            for (int j = i + 1; j < eventCount; j++) {
                if (events[i].TS > events[j].TS) {
                    EventData temp = events[i];
                    events[i] = events[j];
                    events[j] = temp;
                }
            }
        }
    }

    Patient* findActivePatient(int pid) {
        for (int i = 0; i < activeCount; i++) {
            if (activePatients[i] && activePatients[i]->PID == pid) {
                return activePatients[i];
            }
        }
        return nullptr;
    }

    void addActivePatient(Patient* p) {
        if (activeCount < activeCap) {
            activePatients[activeCount++] = p;   //* this is the function which increase the activecount number 
        }
    }

    void moveToCompleted(Patient* p, const string finalStatus) {
        p->status = finalStatus;
        p->DT = currentTime;    //* DT for departur time
        if (finalStatus == "Left_LWBS") {
            p->WT = p->DT - p->AT;
        }
        if (completedCount < completedCap) {
            completedPatients[completedCount++] = p;
        }
        for (int i = 0; i < activeCount; i++) {    //* shifting the items in the activepatients array because we took one from the array
            if (activePatients[i] == p) {
                for (int j = i; j < activeCount - 1; j++) {
                    activePatients[j] = activePatients[j + 1];
                }
                activeCount--;
                break;
            }
        }
    }

    void pushToZoneQueue(Patient* p) {    //* resus . fast . acute queues and they are priority queues 
        p->status = "Waiting_Bay";
        if (p->zone == "RESUS") resusQueue.enqueue(p);
        else if (p->zone == "ACUTE") acuteQueue.enqueue(p);
        else fastQueue.enqueue(p);
    }

    void processCompletionsAndTimers() {   //* for the clinician 

        for (int i = 0; i < nNurses; i++) {
            if (nurses[i].status == "On_Break") {
                nurses[i].breakTimer--;     
                if (nurses[i].breakTimer <= 0) {
                    nurses[i].status = "Available";
                }
            }
        }
        for (int i = 0; i < nPhysicians; i++) {
            if (doctors[i].status == "On_Break") {
                doctors[i].breakTimer--;
                if (doctors[i].breakTimer <= 0) {
                    doctors[i].status = "Available";
                }
            }
        }

        //* this loop for ending the triage period for the current patients 
        for (int i = 0; i < nNurses; i++) {
            if (nurses[i].status == "Busy" && nurses[i].currentPatient) {   //* current patient here is a pointer look for it in the class 
                Patient* p = nurses[i].currentPatient;
                p->remainingTriage--;   //* here we modifing the time of remainingtriage and the ST for the currentpatient
                p->ST++;
                nurses[i].busySteps++;
                if (p->remainingTriage <= 0) {
                    nurses[i].currentPatient = nullptr;
                    nurses[i].handledCount++;
                    if (nurses[i].handledCount >= breakInterval) {   //* breakinterval is the limit that is used to enter the clinician a break duration
                        nurses[i].status = "On_Break";
                        nurses[i].breakTimer = breakDuration;    //* reset the break timer to start new breakduration
                        nurses[i].handledCount = 0;   //* each breakduration we set handledcount to 0 
                    } else {
                        nurses[i].status = "Available";
                    }
                    pushToZoneQueue(p);
                }
            }
        }

        for (int i = 0; i < bayCount; i++) {
            TreatmentBay& b = bays[i];
            if (b.status == "Cleaning") {
                b.remainingTime--;
                if (b.remainingTime <= 0) {
                    b.status = "Free";
                    b.remainingTime = 0;
                }
            } else if (b.status == "Occupied" && b.patient) {
                Patient* p = b.patient;
                b.occupiedSteps++;

                if (p->status == "In_Diagnostics") {
                    p->remainingDiag--;
                    p->ST++;
                    if (p->remainingDiag <= 0) {
                        p->status = "In_Treatment";
                        b.remainingTime = p->remainingTD;
                    }
                } else if (p->status == "In_Treatment") {
                    p->remainingTD--;
                    b.remainingTime--;
                    p->ST++;

                  
                    for (int d = 0; d < nPhysicians; d++) {
                        if (doctors[d].currentPatient == p) {
                            doctors[d].busySteps++;
                        }
                    }

                    if (p->remainingTD <= 0) {
                        for (int d = 0; d < nPhysicians; d++) {
                            if (doctors[d].currentPatient == p) {
                                doctors[d].currentPatient = nullptr;
                                doctors[d].handledCount++;
                                if (doctors[d].handledCount >= breakInterval) {
                                    doctors[d].status = "On_Break";
                                    doctors[d].breakTimer = breakDuration;
                                    doctors[d].handledCount = 0;
                                } else {
                                    doctors[d].status = "Available";
                                }
                            }
                        }
                        moveToCompleted(p, "Discharged");
                        b.startCleaning(2);
                    }
                }
            }
        }
    }

    void processEventsAtCurrentTime() {
        while (nextEventIdx < eventCount && events[nextEventIdx].TS == currentTime) {
            const EventData& ev = events[nextEventIdx++];

            if (ev.type == "A") {
                Patient* p = new Patient(ev.PID, ev.TS, ev.ESI, ev.zone, ev.Maxwait, ev.TD);
                addActivePatient(p);
                if (p->ESI == 1) {
                    pushToZoneQueue(p);
                } else {
                    p->status = "Waiting_Triage";
                    triageQueue.enqueue(p);
                }
            } else if (ev.type == "D") {
                Patient* p = findActivePatient(ev.PID);
                if (p && p->status == "In_Treatment") {
                    p->status = "In_Diagnostics";
                    p->remainingDiag = ev.testduration;
                }
            } else if (ev.type == "L") {
                Patient* p = findActivePatient(ev.PID);
                if (p && p->ESI >= 3 && (p->status == "Waiting_Triage" || p->status == "Waiting_Bay")) {
                    triageQueue.removePatient(p);
                    resusQueue.removePatient(p);
                    acuteQueue.removePatient(p);
                    fastQueue.removePatient(p);
                    moveToCompleted(p, "Left_LWBS");
                }
            }
        }
    }

    void processEscalations() {
        for (int i = 0; i < activeCount; i++) {
            Patient* p = activePatients[i];
            if (!p) continue;
            if ((p->status == "Waiting_Triage" || p->status == "Waiting_Bay") && p->ESI > 1) {
                int waited = currentTime - p->AT;
                if (escalationThreshold > 0 && waited >= escalationThreshold) {
                    int targetESI = max(2, p->ESI - (waited / escalationThreshold));
                    if (targetESI < p->currentESI) {
                        p->currentESI = targetESI;
                        if (!p->escalated) {
                            p->escalated = true;
                            escalatedCount++;
                        }
                    }
                }
            }
        }
    }

    TreatmentBay* findFreeBay(const string& zone, bool allowAcuteOverflowForEsi1) {
        for (int i = 0; i < bayCount; i++) {
            if (bays[i].status == "Free" && bays[i].zone == zone) {
                return &bays[i];
            }
        }
        if (allowAcuteOverflowForEsi1 && zone == "RESUS") {
            for (int i = 0; i < bayCount; i++) {
                if (bays[i].status == "Free" && bays[i].zone == "ACUTE") {
                    return &bays[i];
                }
            }
        }
        return nullptr;
    }

    Clinician* findAvailableDoctor() {
        for (int i = 0; i < nPhysicians; i++) {
            if (doctors[i].status == "Available") {
                return &doctors[i];
            }
        }
        return nullptr;
    }

    void assignFromZoneQueue(PriorityQueue& q, const string& zone) {
        int n = q.size();
        Patient** deferred = new Patient*[n];
        int defCount = 0;

        while (!q.isEmpty()) {
            Patient* p = q.dequeueBest(currentTime, escalationThreshold);
            TreatmentBay* bay = findFreeBay(zone, p->ESI == 1);
            Clinician* doc = findAvailableDoctor();

            if (bay && doc) {
                doc->status = "Busy";
                doc->currentPatient = p;
                bay->assignPatient(p, currentTime);
            } else {
                deferred[defCount++] = p;
            }
        }

        for (int i = 0; i < defCount; i++) {
            q.enqueue(deferred[i]);
        }
        delete[] deferred;
    }

    void allocateResources() {
    
        for (int i = 0; i < nNurses; i++) {
            if (nurses[i].status == "Available" && !triageQueue.isEmpty()) {
                Patient* p = triageQueue.dequeue();
                if (p) {
                    p->status = "In_Triage";
                    if (p->TT == -1) p->TT = currentTime;
                    p->remainingTriage = 2;
                    nurses[i].status = "Busy";
                    nurses[i].currentPatient = p;
                }
            }
        }

        
        assignFromZoneQueue(resusQueue, "RESUS");
        assignFromZoneQueue(acuteQueue, "ACUTE");
        assignFromZoneQueue(fastQueue, "FAST");
    }

    void processWalkouts() {
        for (int i = activeCount - 1; i >= 0; i--) {
            Patient* p = activePatients[i];
            if (!p) continue;
            if (p->ESI >= 3 && (p->status == "Waiting_Triage" || p->status == "Waiting_Bay")) {
                if ((currentTime - p->AT) > p->Maxwait) {
                    triageQueue.removePatient(p);
                    resusQueue.removePatient(p);
                    acuteQueue.removePatient(p);
                    fastQueue.removePatient(p);
                    moveToCompleted(p, "Left_LWBS");
                }
            }
        }
    }

    bool hasRemainingWork() const {
        if (nextEventIdx < eventCount) return true;
        if (activeCount > 0) return true;
        for (int i = 0; i < bayCount; i++) {
            if (bays[i].status != "Free") return true;
        }
        for (int i = 0; i < nNurses; i++) {
            if (nurses[i].status != "Available") return true;
        }
        for (int i = 0; i < nPhysicians; i++) {
            if (doctors[i].status != "Available") return true;
        }
        return false;
    }

    void runSimulation(bool interactive = false) {
        currentTime = 0;
        while (true) {
            currentTime++;
            processCompletionsAndTimers();
            processEventsAtCurrentTime();
            processEscalations();
            allocateResources();
            processWalkouts();

            if (interactive) {
                cout << "[TS " << currentTime << "] Active: " << activeCount
                     << " | Completed: " << completedCount << "\n";
                cin.get();
            }

            if (!hasRemainingWork() && currentTime > 0) break;
            if (currentTime > 100000) break; 
        }
        for (int i = 0; i < completedCount - 1; i++) {
            for (int j = i + 1; j < completedCount; j++) {
                if (completedPatients[i]->DT > completedPatients[j]->DT) {
                    Patient* tmp = completedPatients[i];
                    completedPatients[i] = completedPatients[j];
                    completedPatients[j] = tmp;
                }
            }
        }

        int dischargedCount = 0;
        int lwbsCount = 0;
        double totalWT = 0.0;
        double totalST = 0.0;

        output["patients"] = json::array();
        for (int i = 0; i < completedCount; i++) {
            Patient* p = completedPatients[i];
            if (p->status == "Discharged") dischargedCount++;
            else if (p->status == "Left_LWBS") lwbsCount++;

            totalWT += p->WT;
            totalST += p->ST;

            output["patients"].push_back({
                {"DT", p->DT},
                {"PID", p->PID},
                {"ESI", p->ESI},
                {"AT", p->AT},
                {"WT", p->WT},
                {"ST", p->ST},
                {"Status", p->status}
            });
        }

        double totalBaySteps = (bayCount > 0 && currentTime > 0) ? (double)(bayCount * currentTime) : 1.0;
        double occupiedBaySteps = 0.0;
        for (int i = 0; i < bayCount; i++) occupiedBaySteps += bays[i].occupiedSteps;

        output["summary"] = {
            {"total_served", completedCount},
            {"discharged", dischargedCount},
            {"left_lwbs", lwbsCount},
            {"avg_wt", (completedCount > 0) ? (totalWT / completedCount) : 0.0},
            {"avg_st", (completedCount > 0) ? (totalST / completedCount) : 0.0},
            {"bay_utilization_pct", (occupiedBaySteps / totalBaySteps) * 100.0},
            {"escalated_patients", escalatedCount},
            {"lwbs_rate_pct", (completedCount > 0) ? ((double)lwbsCount / completedCount) * 100.0 : 0.0}
        };
    }
};

class IOManager {
public:
    bool loadConfig(const string& path, json& cfg) {
        ifstream file(path);
        if (!file.is_open()) {
            cout << "JSON FILE NOT FOUND!\n";
            return false;
        }
        file >> cfg;
        file.close();
        return true;
    }

    bool saveOutput(const string& path, const json& out) {
        ofstream file(path);
        if (!file.is_open()) {
            cout << "CANNOT CREATE OUTPUT FILE!\n";
            return false;
        }
        file << out.dump(4);
        file.close();
        cout << "OUTPUT SAVED SUCCESSFULLY\n";
        return true;
    }
};

class UI {
public:
    void printBanner() {
        cout << "=== Hospital ER & Triage Flow Simulator ===\n";
    }
};

int main() {
    UI ui;
    ui.printBanner();

    IOManager io;
    ERSimulator sim;

    if (!io.loadConfig("G:\\projects vs code\\hospital triage\\input.json", sim.config)) {
        return 1;
    }

    sim.loadAndSortEvents();
    sim.runSimulation(false);
    io.saveOutput("G:\\projects vs code\\hospital triage\\output.json", sim.output);

    return 0;
}
