#include <cmath>
#include <ctime>
#include <iostream>
#include <ros/ros.h>
#include <rws2018_libs/team.h>
#include <rws2018_msgs/MakeAPlay.h>
#include <vector>

#include <tf/transform_broadcaster.h>
// Boost includes
#include <boost/shared_ptr.hpp>

using namespace std;
namespace rws_lsarmento {
class Player // equivalente ao struct em C
{
public:
  Player(string argin_name) { name = argin_name; }

  std::string name;

  std::string getTeamName() { return team; }

  // Accessor for team_name
  int setTeamName(int index) {
    if (index == 0) {
      setTeamName("red");
    } else if (index == 1) {
      setTeamName("green");
    } else if (index == 2) {
      setTeamName("blue");
    } else {
      // std::cout << "cannot set team number to " << index << std::endl;
      ROS_WARN("cannot set team number to %d", index);
      return 0;
    }
  }

  int setTeamName(std::string argin_team) {
    if (argin_team == "red" || argin_team == "green" || argin_team == "blue") {
      team = argin_team;
      return 1;
    } else {
      // std::cout << "cannot set team name to" << argin_team << std::endl;
      ROS_WARN("cannot set team name to %s", argin_team.c_str());
      return 0;
    }
  }

private:
  std::string team;
};

// class Team
// {
// public:
//   Team(string name)
//   {
//   }
// };

class MyPlayer : public Player {
public:
  boost::shared_ptr<Team> red_team;
  boost::shared_ptr<Team> green_team;
  boost::shared_ptr<Team> blue_team;

  boost::shared_ptr<Team> my_team;
  boost::shared_ptr<Team> my_preys;
  boost::shared_ptr<Team> my_hunters;

  tf::TransformBroadcaster br; // declare broadcaster

  ros::NodeHandle n;
  boost::shared_ptr<ros::Subscriber> sub;
  tf::Transform T; // declare the transformation object (player's pose wrt
                   // world)

  MyPlayer(std::string argin_name, std::string argin_team)
      : Player(argin_name) {
    red_team = boost::shared_ptr<Team>(new Team("red"));
    green_team = boost::shared_ptr<Team>(new Team("green"));
    blue_team = boost::shared_ptr<Team>(new Team("blue"));
    if (red_team->playerBelongsToTeam(name)) {
      my_team = red_team;
      my_preys = green_team;
      my_hunters = blue_team;
      setTeamName("red");
    } else if (green_team->playerBelongsToTeam(name)) {
      my_team = green_team;
      my_preys = blue_team;
      my_hunters = red_team;
      setTeamName("green");
    } else if (blue_team->playerBelongsToTeam(name)) {
      my_team = blue_team;
      my_preys = red_team;
      my_hunters = green_team;
      setTeamName("blue");
    }
    sub = boost::shared_ptr<ros::Subscriber>(new ros::Subscriber());
    *sub = n.subscribe("/make_a_play", 100, &MyPlayer::move, this);

    struct timeval t1;
    gettimeofday(&t1, NULL);
    srand(t1.tv_usec);
    double start_x = ((double)rand() / (double)RAND_MAX) * 10 - 5;
    double start_y = ((double)rand() / (double)RAND_MAX) * 10 - 5;
    warp(start_x, start_y, M_PI / 2);
    // warp(4, 3, M_PI / 2);
  }

  void warp(double x, double y, double alfa) {
    T.setOrigin(tf::Vector3(x, y, 0.0));
    tf::Quaternion q;
    q.setRPY(0, 0, alfa);
    T.setRotation(q);
    br.sendTransform(
        tf::StampedTransform(T, ros::Time::now(), "world", "moliveira"));
    ROS_INFO("Warping to x=%f y=%f a=%f", x, y, alfa);
  }

  void printReport() {
    ROS_INFO("My name is %s and my team is %s", name.c_str(),
             getTeamName().c_str());
  }

  void move(const rws2018_msgs::MakeAPlay::ConstPtr &msg) {

    double x = T.getOrigin().x();
    double y = T.getOrigin().y();
    double a = 0;

    // ......................................
    // .............AI
    // ......................................

    // ......................................
    // .............CONSTRAINS PART
    // ......................................

    double dist_max = msg->cheetah;

    static float loop = 0;

    // T.setOrigin(tf::Vector3(5 * sin(loop++ / 20), 5 * cos(loop++ / 20),
    // 0.0));
    T.setOrigin(tf::Vector3(x, y, 0.0));
    tf::Quaternion q;
    q.setRPY(0, 0, a - loop / 6);
    T.setRotation(q);
    br.sendTransform(
        tf::StampedTransform(T, ros::Time::now(), "world", "lsarmento"));

    ROS_INFO("My name is %s i like to move it move it", name.c_str());
  }
};
}

int main(int argc, char **argv) {
  ros::init(argc, argv, "lsarmento");

  rws_lsarmento::MyPlayer my_player("lsarmento", "red");
  // std::cout << "Created an instance of class player with public name " <<
  // my_player.name << std::endl;
  // std::cout << "Created an instance of class player with team name " <<
  // my_player.getTeamName() << std::endl;
  // my_player.printReport();
  ros::NodeHandle n;

  // string test_param_value;
  // n.getParam("test_param", test_param_value);

  // cout << "read test _param with value " << test_param_value << endl;
  my_player.printReport();

  // ros::Rate loop_rate(10);
  // float loop;
  // while (ros::ok()) {
  //   my_player.move(loop++);

  //   ros::spinOnce();
  //   loop_rate.sleep();
  // }
  ros::spin();
}