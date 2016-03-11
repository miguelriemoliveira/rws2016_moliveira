#include <iostream>
#include <vector>
#include <boost/shared_ptr.hpp>

#include <ros/ros.h>
#include <tf/transform_listener.h>
#include <tf/transform_broadcaster.h>
//#include <tf/transform_broadcaster.h>

#include <rws2016_libs/team_info.h>
#include <rws2016_msgs/GameMove.h>

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
                        setTeamName("red"); break;
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
                string first_refframe = name;
                string second_refframe = p.name;

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

            double getAngle(string player_name)
            {
                //computing the distance 
                string first_refframe = name;
                string second_refframe = player_name;

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

                double angle = atan2(y,x);
                return angle;

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

            /**
             * @brief The teams
             */
            boost::shared_ptr<Team> my_team;
            boost::shared_ptr<Team> hunter_team;
            boost::shared_ptr<Team> prey_team;

            boost::shared_ptr<ros::Subscriber> _sub; 

            /**
             * @brief Constructor
             *
             * @param name player name
             * @param team team name
             */
            MyPlayer(string name, string team): Player(name)
        {
            setTeamName(team);
            ros::NodeHandle node;

            //Initialize teams
            vector<string> myTeam_names, myHunters_names, myPreys_names;
            string myTeamId, myHuntersId, myPreysId;

            if (!team_info(node, myTeam_names, myHunters_names, myPreys_names, myTeamId, myHuntersId, myPreysId))
                ROS_ERROR("Something went wrong reading teams");

            my_team = (boost::shared_ptr<Team>) new Team(myTeamId, myTeam_names);
            hunter_team = (boost::shared_ptr<Team>) new Team(myHuntersId, myHunters_names);
            prey_team = (boost::shared_ptr<Team>) new Team(myPreysId, myPreys_names);

            my_team->printTeamInfo();
            hunter_team->printTeamInfo();
            prey_team->printTeamInfo();

            //Initialize position according to team
            ros::Duration(0.5).sleep(); //sleep to make sure the time is correct
            tf::Transform t;
            //srand((unsigned)time(NULL)); // To start the player in a random location
            struct timeval t1;      
            gettimeofday(&t1, NULL);
            srand(t1.tv_usec);
            double X=((((double)rand()/(double)RAND_MAX) ) * 2 -1) * 5 ;
            double Y=((((double)rand()/(double)RAND_MAX) ) * 2 -1) * 5 ;
            t.setOrigin( tf::Vector3(X, Y, 0.0) );
            tf::Quaternion q; q.setRPY(0, 0, 0);
            t.setRotation(q);
            br.sendTransform(tf::StampedTransform(t, ros::Time::now(), "/map", name));

            //initialize the subscriber
            _sub = (boost::shared_ptr<ros::Subscriber>) new ros::Subscriber;
            *_sub = node.subscribe("/game_move", 1, &MyPlayer::moveCallback, this);

        }

            /**
             * @brief Moves MyPlayer
             *
             * @param displacement the liner movement of the player, bounded by [-0.1, 1]
             * @param turn_angle the turn angle of the player, bounded by  [-M_PI/30, M_PI/30]
             */
            void move(double displacement, double turn_angle)
            {
                //Put arguments withing authorized boundaries
                double max_d =  1; 
                displacement = (displacement > max_d ? max_d : displacement);

                double min_d =  -0.1; 
                displacement = (displacement < min_d ? min_d : displacement);

                double max_t =  (M_PI/30);
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

            string getNameOfClosestPrey(void)
            {
                double prey_dist = getDistance(*prey_team->players[0]);
                string prey_name = prey_team->players[0]->name;

                for (size_t i = 1; i < prey_team->players.size(); ++i)
                {
                    double d = getDistance(*prey_team->players[i]);

                    if (d < prey_dist) //A new minimum
                    {
                        prey_dist = d;
                        prey_name = prey_team->players[i]->name;
                    }
                }

                return prey_name;
            }


            /**
             * @brief called whenever a /game_move msg is received
             *
             * @param msg the msg with the animal values
             */
            void moveCallback(const rws2016_msgs::GameMove& msg)
            {
                ROS_INFO("player %s received game_move msg", name.c_str()); 

                //I will encode a very simple hunting behaviour:
                //
                //1. Get closest prey name
                //2. Get angle to closest prey
                //3. Compute maximum displacement
                //4. Move maximum displacement towards angle to prey (limited by min, max)

                //Step 1
                string closest_prey = getNameOfClosestPrey();
                ROS_INFO("Closest prey is %s", closest_prey.c_str());

                //Step 2
                double angle = getAngle(closest_prey);

                //Step 3
                double displacement = msg.cat; //I am a cat, others may choose another animal

                //Step 4
                move(displacement, angle);

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
    ros::init(argc, argv, "moliveira");
    ros::NodeHandle node;

    //Creating an instance of class MyPlayer
    rws2016_moliveira::MyPlayer my_player("moliveira", "red");

    //Infinite loop
    ros::spin();
}
