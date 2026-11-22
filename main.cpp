
#include <iostream>
#include <cmath>
#include <fstream>
#include <json.hpp>

using namespace std;
using json = nlohmann::json;

json config;
json output;
int currenttime = 0;
int counterarrive= 0;
void loadconfig()
{
    ifstream file("G:\\projects vs code\\hospital triage\\input.json");

    if (!file.is_open())
    {
        cout << "JSON FILE NOT FOUND!" << endl;
        return;
    }

    cout << "FILE OPENED SUCCESSFULLY" << endl;

    file >> config;

    file.close();
}
void saveoutput()
{
    ofstream file("G:\\projects vs code\\hospital triage\\output.json");

    if (!file.is_open())
    {
        cout << "CANNOT CREATE OUTPUT FILE!" << endl;
        return;
    }

    file << output;

    file.close();

    cout << "OUTPUT SAVED SUCCESSFULLY" << endl;
}

class ERsimulator
{
public:
int  RESUS = config["N_RESUS"];
       int  ACUTE = config["N_ACUTE"];
    int     FAST = config["N_FAST"];
    void updatetime() { currenttime++; }
    int gettime() { return currenttime; }
    int getarrivalnum () {
           int size = config["M"];
        
        for (int i = 0 ; i < size;i++){
            if (config["events"][i]["type"]== "A"){counterarrive ++;}
    }
    return counterarrive;
}
};

class patient : public ERsimulator
{
public:
    int A = 20 ,esi, AT, maxwait, pid, TD, DT, WT;
    float priorityscore;
     string zone;
     
     
    patient()
    {
        
        int *ptr = new int [getarrivalnum()];
        int j = 0;
        for(int i = 0 ; i <counterarrive ;i ++){
         j++ ;
       if (config["events"][i]["type"]== "A"){
        ptr[j] = i;
        cout<<ptr[j];
        
         esi = config["events"][ptr[j]]["ESI"];
        AT = config["events"][ptr[j]]["TS"];
        maxwait = config["events"][ptr[j]]["Maxwait"];
        pid = config ["events"][ptr[j]]["PID"];
        TD = config["events"][ptr[j]]["TD"];
         zone =config["events"][ptr[j]]["zone"];
        priorityscore = (6 - esi) * 100 +
            A * floor((ERsimulator::gettime() - AT) /
            (float)config["EscalationThreshold"]);
        
  if (zone == "RESUS"){RESUS --;
    updatetime(); 
      WT = currenttime - AT;
    DT= TD + AT +WT ;}
     if (zone == "ACUTE"){ACUTE --;
    updatetime();
    WT = currenttime - AT;
DT= TD + AT +WT ;}
     if (zone == "FAST"){FAST -- ;
    updatetime();
     WT = currenttime - AT;
    DT= TD + AT +WT ;}
    output["patient"].push_back(
        {
        {"DT",DT},
        {"PID",pid},
        {"ESI",esi},
        {"WT",WT},
        {"AT",AT},
        {"TD",TD},
        {"priorety",priorityscore}
        }
        );

    } 
   continue;
  }
    }  
};
class clician : public ERsimulator
{
public:
    char state;
    int N_nurse;
    int N_doctor;

    clician()
    {
        N_nurse = config["N_NURSES"];
        N_doctor = config["N_Physicians"];
    }
};

class Event : public ERsimulator 
{
public:
    virtual void execute() = 0;
};
class Arrival : public Event  {
     public:
     
  void execute ()override {
   { 
    int arrivalnum =ERsimulator::getarrivalnum();
            patient();
       
        
    }

  }
};
class Diagnostic : public Event  {
     public:
     
};
class Leave: public Event  {
     public:
     
};
class IOmanager : public ERsimulator
  {
    public:
    string statues;
    IOmanager() {
        Arrival p1;
        p1.execute();
     }   
};
class UI : public ERsimulator
{
    UI()
    {
        
    }
};

int main()
{
    loadconfig();
    IOmanager();
   saveoutput();

    return 0;
}
