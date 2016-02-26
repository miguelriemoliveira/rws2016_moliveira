#include <iostream>
#include <vector>
#include <boost/shared_ptr.hpp>


using namespace std;

namespace rws2016_moliveira
{

    class Player
    {
        public:

            //Constructor with the same name as the class
            Player(string name) { this->name = name;}

            int setTeamName(int team_index = 0 /*default value*/)
            {
                switch (team_index)
                {
                    case 0: 
                        return setTeamName("green"); break;
                    case 1: 
                        return setTeamName("green"); break;
                    case 2: 
                        return setTeamName("blue");  break;
                    default: 
                        cout << "wrong team index given. Cannot set team" << endl; break;
                }
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

            //Gets team name (accessor)
            string getTeamName(void) {return team;}

            string name; //A public atribute

        private:
            string team;
    };

    //Class myPlayer extends class Player
    class myPlayer: public Player
    {
        public: 

            myPlayer(string name, string team): Player(name)
        {
            setTeamName(team);
        }

    };

    class Team
    {
        public: 

            //Team constructor
            Team(string team, vector<string>& player_names)
            {
                name = team; 

                //Cycle all player names, and create a class player for each
                for (size_t i=0; i < player_names.size(); ++i)
                {
                    //Why? Copy constructable ...
                    boost::shared_ptr<Player> p(new Player(player_names[i]));
                    p->setTeamName(name);
                    players.push_back(p);
                }

            }

            void printTeamInfo(void)
            {
                cout << "Team " << name << " has the following players:" << endl;

                for (size_t i=0; i < players.size(); ++i)
                    cout << players[i]->name << endl;
            }

            string name;
            vector<boost::shared_ptr<Player> > players;
    };

} //end of namespace rws2016_moliveira

int main()
{
    std::vector<string> names;
    names.push_back("moliveira");
    names.push_back("vsantos");
    names.push_back("pdias");

    rws2016_moliveira::Team team("green", names);
    team.printTeamInfo();

    return 1;                                                      
}

