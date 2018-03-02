#include <iostream>
#include <ros/ros.h>
#include <rws2018_libs/team.h>
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
  }
  void printReport() {
    ROS_INFO("My name is %s and my team is %s", name.c_str(),
             getTeamName().c_str());
  }

  void move(void) {
    tf::Transform transform; //
    transform.setOrigin(tf::Vector3(3, 4, 0.0));
    tf::Quaternion q;
    q.setRPY(0, 0, 0.5);
    transform.setRotation(q);
    br.sendTransform(tf::StampedTransform(transform, ros::Time::now(), "world",
                                          "lsarmento"));
  }
};
}

int main(int argc, char **argv) {
  ros::init(argc, argv, "lsarmento");

  rws_lsarmento::MyPlayer my_player("lsarmento", "green");
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
  ros::Rate loop_rate(10);
  while (ros::ok()) {
    my_player.move();

    ros::spinOnce();
    loop_rate.sleep();
  }
  ros::spin();
}