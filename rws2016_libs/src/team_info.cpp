#include <rws2016_libs/team_info.h>
#include <algorithm>


bool team_info(const ros::NodeHandle &nh, vector<string> &myTeam, vector<string> &myHunters, vector<string> &myPreys, string& myTeamId, string& myHuntersId, string& myPreysId)
{
    vector<string> teamA, teamB, teamC;
    bool result=true;
    if(nh.getParam("/teamRed", teamA) && nh.getParam("/teamGreen", teamB) && nh.getParam("/teamBlue", teamC)){
    string myName = (ros::this_node::getName()).substr(1); //To remove the dash

    cout << "myName is " << myName << endl;

        if(find(teamA.begin(), teamA.end(), myName) != teamA.end()){
            myTeam=teamA; myTeamId = "red";
            myPreys=teamB; myPreysId = "green";
            myHunters=teamC; myHuntersId = "blue";
        }
        else if (find(teamB.begin(), teamB.end(), myName) != teamB.end()){
            myTeam=teamB; myTeamId = "green";
            myPreys=teamC; myPreysId = "blue";
            myHunters=teamA; myHuntersId = "red";
        }
        else if (find(teamC.begin(), teamC.end(), myName) != teamC.end()){
            myTeam=teamC; myTeamId = "blue";
            myPreys=teamA; myPreysId = "red";
            myHunters=teamB; myHuntersId = "green";
        }
        else{
            result=false;
        }
    }
    else{
        result=false;
    }

    return result;
}
