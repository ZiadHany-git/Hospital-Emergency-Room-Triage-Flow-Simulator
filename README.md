# Hospital-Emergency-Room-Triage-Flow-Simulator
This project shows the standard protocol of dealing with patient in triage and ER rooms in hospitals.
--Note this is an educational project not meant to be a professional one.
**if you have any contributes that can add to this project please don't hesitate**
Here you will find instructions and input/output files structure >
some abbriviations that you should know -> AT (Arrival time ) ->TT (triage time )->BT (Bay time )-> DT (Departure time ) ->TS (timestep)-> PID (patient ID)
ESI zone (RESUS , ACUTE , FAST) -> TD (treatment duration) -> ESI = Emergency Severity Index
(KPI)Key Performance Indicators -> (RESUS -> ACUTE -> FAST )
Input File Structure :
 N_Resus N_Acute N_FastTrack N_Nurses N_Physicians
EscalationThreshold BreakInterval BreakDuration
M
<M event lines in chronological order>
exaple :
2 4 3 2 4  
15 5 3
6
A 1 101 3 ACUTE 30 12  
A 2 102 1 RESUS 999 20
A 4 103 4 FAST 20 8
D 7 101 5
A 10 104 2 ACUTE 45 15
L 25 103
output file format : 
DT PID ESI AT WT ST Status
# Example lines:
22 102 1 2 0 20 Discharged
25 103 4 4 21 0 Left_LWBS 
Required Aggregate Summary Statistics (appended at the end of the output file):
Patient Counts: Total patients served, breakdown by ESI level (1–5), total Discharged, and total Left_LWBS.
Waiting & Service Times: Overall average WT, average WT per ESI level, and average Service Time (ST).
Resource Utilization: Average bay utilization (%) per zone and overall physician/nurse utilization (%).
Escalation & LWBS Rate: Number of patients escalated due to wait time and percentage of LWBS walkouts.
