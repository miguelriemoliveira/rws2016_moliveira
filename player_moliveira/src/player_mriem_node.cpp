#include <iostream>
#include <vector>
#include <boost/shared_ptr.hpp>

using namespace std;

namespace rws2016_moliveira
{

    class Player
    {
        public:
            Player(string name)
            {
                this->name = name;
            }

            string name;

            string getTeamName() {return team;};
            int setTeamName(int team_index = 1)
            {
                if(team_index == 0) setTeamName("red");
                else if(team_index == 1) setTeamName("green");
                else if(team_index == 2) setTeamName("blue");
            }

            //Set team name, if given a correct team name (accessor)
            int setTeamName(string team)
            {
                if (team=="red" || team=="green" || team=="blue")
                {
                    this->team = team;
                    return 1;
                }
                else
                {
                    cout << "cannot set team name to " << team << endl;
                    return 0;
                }
            }

        private:
            string team;
    };

    //Class myPlayer extends class Player
    class myPlayer: public Player
    {
        public: 

            myPlayer(string name, string team = "blue"): Player(name)
            {
                setTeamName(team);
            }

            int varexample;
    };

    class Team
    {
        public:
            Team(vector<string> player_names)
            {
                for (size_t i=0; i < player_names.size(); ++i)
                {
                    Player player_temp(player_names[i]);
                    players.push_back(player_temp);
                }
            }

            void printTeamInfo(void)
            {
                cout << "Team has the following players:" << endl;

                for (size_t i=0; i < players.size(); ++i)
                    cout << players[i].name << endl;
            }
            vector<Player> players;
    };
}//end of rws2016_moliveira namspace

int main()
{
    std::vector<string> names;
    names.push_back("moliveira");
    names.push_back("vsantos");
    names.push_back("pdias");

    rws2016_moliveira::Team team(names);
    team.printTeamInfo();



    return 1;                                                      
}

