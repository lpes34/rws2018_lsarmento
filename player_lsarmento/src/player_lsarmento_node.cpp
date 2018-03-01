#include <ros/ros.h>
#include <iostream>
#include <vector>

// Boost includes
#include <boost/shared_ptr.hpp>

using namespace std;
namespace rws_lsarmento
{
class Player  // equivalente ao struct em C
{
public:
  Player(string argin_name)
  {
    name = argin_name;
  }

  std::string name;

  std::string getTeamName()
  {
    return team;
  }

  // Accessor for team_name
  int setTeamName(int index)
  {
    if (index == 0)
    {
      setTeamName("red");
    }
    else if (index == 1)
    {
      setTeamName("green");
    }
    else if (index == 2)
    {
      setTeamName("blue");
    }
    else
    {
      std::cout << "cannot set team number to " << index << std::endl;
      return 0;
    }
  }

  int setTeamName(std::string argin_team)
  {
    if (argin_team == "red" || argin_team == "green" || argin_team == "blue")
    {
      team = argin_team;
      return 1;
    }
    else
    {
      std::cout << "cannot set team name to" << argin_team << std::endl;
      return 0;
    }
  }

private:
  std::string team;
};

class Team
{
public:
  Team(string name)
  {
  }
};

class MyPlayer : public Player
{
public:
  boost::shared_ptr<Team> red_team;
  boost::shared_ptr<Team> green_team;
  boost::shared_ptr<Team> blue_team;

  MyPlayer(std::string argin_name, std::string argin_team) : Player(argin_name)
  {
    red_team = boost::shared_ptr<Team>(new Team("red"));
    green_team = boost::shared_ptr<Team>(new Team("green"));
    blue_team = boost::shared_ptr<Team>(new Team("blue"));
    setTeamName(argin_team);
  }
  void printReport()
  {
    cout << "My name is " << name << "and my team name is " << getTeamName() << endl;
  }
};
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "lsarmento");

  rws_lsarmento::MyPlayer my_player("lsarmento", "green");
  // std::cout << "Created an instance of class player with public name " << my_player.name << std::endl;
  // std::cout << "Created an instance of class player with team name " << my_player.getTeamName() << std::endl;
  // my_player.printReport();
  ros::NodeHandle n;

  string test_param_value;
  n.getParam("test_param", test_param_value);

  cout << "read test _param with value " << test_param_value << endl;

  ros::spin();
}