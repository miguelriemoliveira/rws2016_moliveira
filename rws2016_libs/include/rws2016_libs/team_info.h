#ifndef TEAM_INFO_H
#define TEAM_INFO_H

#include <ros/ros.h>

using namespace std;

bool team_info(const ros::NodeHandle &nh, vector<string> &myTeam, vector<string> &myHunters, vector<string> &myPreys, string& myTeamId, string& myHuntersId, string& myPreysId);

#endif // TEAM_INFO_H
