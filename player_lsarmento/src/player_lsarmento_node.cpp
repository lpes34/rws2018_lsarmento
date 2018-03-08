#include <cmath>
#include <ctime>
#include <iostream>
#include <ros/ros.h>
#include <rws2018_libs/team.h>
#include <rws2018_msgs/GameQuery.h>
#include <rws2018_msgs/MakeAPlay.h>

#include <vector>
#include <visualization_msgs/Marker.h>

#include <tf/transform_broadcaster.h>
#include <tf/transform_listener.h>
// Boost includes
#include <boost/shared_ptr.hpp>

#define DEFAULT_TIME 0.05

using namespace ros;
using namespace tf;
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

  tf::TransformListener listener;

  boost::shared_ptr<ros::Publisher> vis_pub;

  boost::shared_ptr<ros::ServiceServer> game_query_srv;

  // ros::Publisher vis_pub =
  //     n.advertise<visualization_msgs::Marker>("visualization_marker", 0);

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

    vis_pub = boost::shared_ptr<ros::Publisher>(new ros::Publisher());
    *vis_pub = n.advertise<visualization_msgs::Marker>("/bocas", 0);

    game_query_srv =
        boost::shared_ptr<ros::ServiceServer>(new ros::ServiceServer());
    *game_query_srv = n.advertiseService("/" + name + "/game_query",
                                         &MyPlayer::respondToGameQuery, this);

    struct timeval t1;
    gettimeofday(&t1, NULL);
    srand(t1.tv_usec);
    double start_x = ((double)rand() / (double)RAND_MAX) * 10 - 5;
    double start_y = ((double)rand() / (double)RAND_MAX) * 10 - 5;
    warp(start_x, start_y, M_PI / 2);
    // warp(4, 3, M_PI / 2);
  }

  bool respondToGameQuery(rws2018_msgs::GameQuery::Request &req,
                          rws2018_msgs::GameQuery::Response &res) {
    ROS_WARN("I am %s and I am responding to a service request!", name.c_str());

    res.resposta = "banana";
    return true;
  }
  void warp(double x, double y, double alfa) {
    T.setOrigin(tf::Vector3(x, y, 0.0));
    tf::Quaternion q;
    q.setRPY(0, 0, alfa);
    T.setRotation(q);
    br.sendTransform(
        tf::StampedTransform(T, ros::Time::now(), "world", "lsarmento"));
    ROS_INFO("Warping to x=%f y=%f a=%f", x, y, alfa);
  }

  void printReport() {
    ROS_INFO("My name is %s and my team is %s", name.c_str(),
             getTeamName().c_str());
  }

  double getAngleToPlayer(string other_player,
                          double time_to_wait = DEFAULT_TIME) {
    StampedTransform t;
    Time now = Time(0);
    try {
      listener.waitForTransform("lsarmento", other_player, now,
                                Duration(time_to_wait));
      listener.lookupTransform("lsarmento", other_player, now, t);
    } catch (TransformException &ex) {
      ROS_ERROR("%s", ex.what());
      return NAN;
    }

    return atan2(t.getOrigin().y(), t.getOrigin().x());
  }
  double getDistanceToPlayer(string other_player,
                             double time_to_wait = DEFAULT_TIME) {
    StampedTransform t; // The transform object
    // Time now = Time::now(); //get the time
    Time now = Time(0); // get the latest transform received

    try {
      listener.waitForTransform("lsarmento", other_player, now,
                                Duration(time_to_wait));
      listener.lookupTransform("lsarmento", other_player, now, t);
    } catch (TransformException &ex) {
      ROS_ERROR("%s", ex.what());
      return NAN;
    }

    return sqrt(t.getOrigin().y() * t.getOrigin().y() +
                t.getOrigin().x() * t.getOrigin().x());
  }

  void marker(string player_to_hunt, int person) {
    visualization_msgs::Marker marker;
    marker.header.frame_id = "lsarmento";
    marker.header.stamp = ros::Time();
    marker.ns = "lsarmento";
    marker.id = 0;
    marker.type = visualization_msgs::Marker::TEXT_VIEW_FACING;
    marker.action = visualization_msgs::Marker::ADD;

    marker.pose.orientation.w = 1.0;

    marker.scale.z = 0.5;
    marker.color.a = 1.0; // Don't forget to set the alpha!
    marker.color.r = 1.0;
    marker.color.g = 0.0;
    marker.color.b = 0.0;
    if (person == 1) {
      marker.text = "You are going to die " + player_to_hunt + "!!!";
    } else
      marker.text = "Go away " + player_to_hunt + " :(";

    marker.lifetime = ros::Duration(3);
    vis_pub->publish(marker);
  }

  void move(const rws2018_msgs::MakeAPlay::ConstPtr &msg) {

    double x = T.getOrigin().x();
    double y = T.getOrigin().y();
    double a = 0;

    // ......................................
    // .............AI
    // ......................................

    // Finding nearest prey
    double min_distance = 99999;
    string player_to_hunt = "no player"; /// falta alterar para caçar so so
                                         /// vivos
    for (size_t i = 0; i < msg->green_alive.size(); i++) {
      double dist = getDistanceToPlayer(msg->green_alive[i]);
      if (isnan(dist)) {
      } else if (dist < min_distance) {
        min_distance = dist;
        player_to_hunt = msg->green_alive[i];
      }
    }
    // finding nearest hunter
    double min_distance2 = 99999;
    string player_to_flee = "no player";
    for (size_t i = 0; i < msg->blue_alive.size(); i++) {
      double dist = getDistanceToPlayer(msg->blue_alive[i]);
      if (isnan(dist)) {
      } else if (dist < min_distance2) {
        min_distance2 = dist;
        player_to_flee = msg->blue_alive[i];
      }
    }

    double delta_alpha = 0;
    double displacement = 6;

    if (min_distance < min_distance2) {

      delta_alpha = getAngleToPlayer(player_to_hunt);
      if (isnan(delta_alpha))
        delta_alpha = 0;
      marker(player_to_hunt, 1);
    } else {
      delta_alpha = -getAngleToPlayer(player_to_flee);
      if (isnan(delta_alpha))
        delta_alpha = 0;
      marker(player_to_flee, 2);
    }

    // distance to center
    double distance_center = sqrt(x * x + y * y);
    if (distance_center > 7) {
      // double ang_me_in_world = atan2(y, x);
      delta_alpha = getAngleToPlayer("world");
      // ROS_INFO("getting to the edge");
    }

    // ......................................
    // .............CONSTRAINS PART
    // ......................................
    double displacement_max = msg->cheetah;
    double displacement_with_constrains;
    displacement > displacement_max ? displacement = displacement_max
                                    : displacement = displacement;

    double delta_alpha_max = M_PI / 30;
    fabs(delta_alpha) > fabs(delta_alpha_max)
        ? delta_alpha = delta_alpha_max * delta_alpha / fabs(delta_alpha)
        : delta_alpha = delta_alpha;

    static float loop = 0;

    tf::Transform my_move_T; // declare the transformation object (player's pose
                             // wrt world)
    my_move_T.setOrigin(tf::Vector3(displacement, 0.0, 0.0));
    tf::Quaternion q1;
    q1.setRPY(0, 0, delta_alpha);
    my_move_T.setRotation(q1);
    // T.setOrigin(tf::Vector3(5 * sin(loop++ / 20), 5 * cos(loop++ / 20),
    // 0.0));
    T = T * my_move_T;
    br.sendTransform(
        tf::StampedTransform(T, ros::Time::now(), "world", "lsarmento"));

    // ROS_INFO("My name is %s i like to move it move it", name.c_str());
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