#include <iostream>

class Player  // equivalente ao struct em C
{
public:
  Player(std::string argin_name)
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

class MyPlayer : public Player
{
public:
  MyPlayer(std::string argin_name, std::string argin_team) : Player(argin_name)
  {
    setTeamName(argin_team);
  }
};

int main()
{
  std::string player_name = "lsarmento";
  // Creating an instance of class Player
  Player player(player_name);
  player.setTeamName(1);

  std::cout << "Created an instance of class player with public name " << player.name << std::endl;
  std::cout << "Created an instance of class player with public team name " << player.getTeamName() << std::endl;

  MyPlayer my_player("lsarmento", "green");
  std::cout << "Created an instance of class player with public name " << player.name << std::endl;
}