#include <iostream>
#include <vector>
#include <boost/shared_ptr.hpp>

#include <ros/ros.h>
#include <tf/transform_listener.h>
#include <tf/transform_broadcaster.h>
//#include <tf/transform_broadcaster.h>


using namespace std;

/**
 * @brief The namespace of my player
 */
namespace rws2016_moliveira
{


    /**
     * @brief Contains a description of a game player
     */
    class Player
    {
        public:

            /**
             * @brief The cosntructor
             *
             * @param name the name of the player
             */
            Player(string name) {this->name = name;}

            /**
             * @brief Sets the team to which the player belongs
             *
             * @param team_index 0,1 or 2 for teams red, green and blue respectively
             *
             * @return 
             */
            void setTeamName(int team_index = 0 /*default value*/)
            {
                switch (team_index)
                {
                    case 0: 
                        setTeamName("green"); break;
                    case 1: 
                        setTeamName("green"); break;
                    case 2: 
                        setTeamName("blue");  break;
                    default: 
                        cout << "wrong team index given. Cannot set team" << endl; break;
                }
            }
            /**
             * @brief Overloaded method to set team name
             *
             * @param team a string with the team name
             */
            void setTeamName(string team)
            {
                if (team=="red" || team=="green" || team=="blue")
                {
                    this->team = team;
                }
                else
                {
                    cout << "cannot set team name to " << team << endl;
                }
            }

            double getDistance(Player& p)
            {
                //computing the distance 
                string first_refframe = p.name;
                string second_refframe = name;

                ros::Duration(0.1).sleep(); //To allow the listener to hear messages
                tf::StampedTransform st; //The pose of the player
                try{
                    listener.lookupTransform(first_refframe, second_refframe, ros::Time(0), st);
                }
                catch (tf::TransformException& ex){
                    ROS_ERROR("%s",ex.what());
                    ros::Duration(1.0).sleep();
                }

                tf::Transform t;
                t.setOrigin(st.getOrigin());
                t.setRotation(st.getRotation());

                double x = t.getOrigin().x();
                double y = t.getOrigin().y();

                double norm = sqrt(x*x + y*y);
                return norm;

            }

            /**
             * @brief returns the team to which the player belongs
             *
             * @return a string with the team name
             */
            string getTeamName(void) {return team;}

            /**
             * @brief Gets the pose (calls updatePose first)
             *
             * @return the transform from map to the /moliveira local reference frame
             */
            tf::Transform getPose(void)
            {
                ros::Duration(0.1).sleep(); //To allow the listener to hear messages
                tf::StampedTransform st; //The pose of the player
                try{
                    listener.lookupTransform("/map", name, ros::Time(0), st);
                }
                catch (tf::TransformException& ex){
                    ROS_ERROR("%s",ex.what());
                    ros::Duration(1.0).sleep();
                }

                tf::Transform t;
                t.setOrigin(st.getOrigin());
                t.setRotation(st.getRotation());
                return t;
            }

            /**
             * @brief the name of the player
             */
            string name; 

        private:
            /**
             * @brief The name of the team
             */
            string team;

            /**
             * @brief the transform listener object
             */
            tf::TransformListener listener; //reads tfs from the ros system
    };


    /**
     * @brief Contains a list of all the players on a team
     */
    class Team
    {
        public: 

            /**
             * @brief Constructor
             *
             * @param team the team name
             * @param player_names a list with the name of all the players on the team
             */
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

            /**
             * @brief Prints the name of the team and the names of all its players
             */
            void printTeamInfo(void)
            {
                cout << "Team " << name << " has the following players:" << endl;

                for (size_t i=0; i < players.size(); ++i)
                    cout << players[i]->name << endl;
            }

            /**
             * @brief The team name
             */
            string name;

            /**
             * @brief A list of Players
             */
            vector<boost::shared_ptr<Player> > players;
    };


    /**
     * @brief MyPlayer extends class Player, i.e., there are additional things I can do with MyPlayer and not with any Player, e.g., to order a movement.
     */
    class MyPlayer: public Player
    {
        public: 

            /**
             * @brief The transform publisher object
             */
            tf::TransformBroadcaster br;

            //vector<string> prey_names;
            //prey_names.push_back("lalmeida");
            //rws2016_moliveira::Team prey_team("green", prey_names);
            //prey_team.printTeamInfo();

            boost::shared_ptr<Team> prey_team;


            /**
             * @brief Constructor
             *
             * @param name player name
             * @param team team name
             */
            MyPlayer(string name, string team): Player(name)
        {
            setTeamName(team);

            //Initialize position to 0,0,0
            tf::Transform t;
            t.setOrigin( tf::Vector3(0.0, 0.0, 0.0) );
            tf::Quaternion q; q.setRPY(0, 0, 0);
            t.setRotation(q);
            br.sendTransform(tf::StampedTransform(t, ros::Time::now(), "/map", name));

            vector<string> prey_names;
            prey_names.push_back("lalmeida");
            prey_team = (boost::shared_ptr<Team>) new Team("green", prey_names);

        }

            /**
             * @brief Moves MyPlayer
             *
             * @param displacement the liner movement of the player, bounded by [-0.1, 1]
             * @param turn_angle the turn angle of the player, bounded by  [-M_PI/60, M_PI/60]
             */
            void move(double displacement, double turn_angle)
            {
                //Put arguments withing authorized boundaries
                double max_d =  1; 
                displacement = (displacement > max_d ? max_d : displacement);

                double min_d =  -0.1; 
                displacement = (displacement < min_d ? min_d : displacement);

                double max_t =  (M_PI/60);
                if (turn_angle > max_t)
                    turn_angle = max_t;
                else if (turn_angle < -max_t)
                    turn_angle = -max_t;

                //Compute the new reference frame
                tf::Transform t_mov;
                t_mov.setOrigin( tf::Vector3(displacement , 0, 0.0) );
                tf::Quaternion q;
                q.setRPY(0, 0, turn_angle);
                t_mov.setRotation(q);

                tf::Transform t = getPose();
                t = t  * t_mov;

                //Send the new transform to ROS
                br.sendTransform(tf::StampedTransform(t, ros::Time::now(), "/map", name));
            }

            void getDistanceToAllPreys(void)
            {

                for (size_t i = 0; i < prey_team->players.size(); ++i )
                {
                    double dist_me_to_ith_player = getDistance(*prey_team->players[i]);
        //cout << "dist_from_my_player_to_lalmeida = " << dist_from_my_player_to_lalmeida << endl;
                }

                //for hunter in hunters 

            }


    };


} //end of namespace rws2016_moliveira

/**
 * @brief The main function
 *
 * @param argc number of command line arguments
 * @param argv values of command line arguments
 *
 * @return result
 */
int main(int argc, char** argv)
{
    //initialize ROS stuff
    ros::init(argc, argv, "player_moliveira_node");
    ros::NodeHandle node;

    //Creating an instance of class MyPlayer
    rws2016_moliveira::MyPlayer my_player("moliveira", "red");


    //vector<string> myteam_names;
    //myteam_names.push_back("moliveira");
    //rws2016_moliveira::Team my_team("red", myteam_names);
    //my_team.printTeamInfo();

    //vector<string> prey_names;
    //prey_names.push_back("lalmeida");
    //rws2016_moliveira::Team prey_team("green", prey_names);
    //prey_team.printTeamInfo();

    rws2016_moliveira::Player lalmeida_player("lalmeida");


    //Infinite loop
    ros::Rate loop_rate(10);
    while (ros::ok())
    {
        //Test the get pose method
        tf::Transform t = my_player.getPose();
        cout << "x = " << t.getOrigin().x() << " y = " << t.getOrigin().y() << endl;

        double dist_from_my_player_to_lalmeida = my_player.getDistance(lalmeida_player);
        cout << "dist_from_my_player_to_lalmeida = " << dist_from_my_player_to_lalmeida << endl;

        //Test the move method
        my_player.move(0.1, -M_PI/6);

        ros::spinOnce();
        loop_rate.sleep();
    }



}
