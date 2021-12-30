
robosoc2d Python documentation
==============================


Very Simplified 2D Robotic Soccer Simulator (c) 2021 Ruggero Rossi
------------------------------------------------------------------
This is an extremely simplified 2-dimensional robotic soccer simulator, aimed at permitting fast simulation on modest hardware in order to make possible to run a great number of simulations. This is particularly valuable when doing statistical analysis or in reinforcement learning algorithms ("scaling down reinforcement learning").

INSTALLATION from pip    
---------------------
Just run:
```
pip install robosoc2d
```

INSTALLATION from source code    
-----------------------------
Robosoc2d is designed and tested to work on Python 3.6 and greater, but it is possible that it may work also on previous versions.

To install it, open the terminal or shell, set the pyextension folder as the current directory and enter\:
```
python3 setup.py install
```
instead, to uninstall it\:
```
pip uninstall robosoc2d
```

**Note for Windows users**: to install from source code you must have a recent version of Visual C++ compiler installed. You
also have to slightly change the file "setup.py". In that file you have to uncomment the following line (just cancel the first caracter that is #):
```python
#module = Extension('robosoc2d', sources=['robosoc2dmodule.cpp'], language='c++', extra_compile_args=['/std:c++17']) # windows version
```
and delete or comment the line above it.

**Note for Mac users**: depending on your configuration, you may incur in an error similar to the following: 
```
"xcrun: error: invalid active developer path (/Library/Developer/CommandLineTools), missing xcrun at: /Library/Developer/CommandLineTools/usr/bin/xcrun
error: command 'gcc' failed with exit status 1"
```
In that case you may be able to solve it writing the following command:
```
xcode-select --install
```
and then again
```
python3 setup.py install
```
If this still doesn't work, you may try
```
xcode-select --reset
```
and then again
```
python3 setup.py install
```

**Note for Linux users**: if you obtain an error message similar to "fatal error: Python.h: No such file or directory" it means that you have not properly installed the Python developer tools. For instance on Ubuntu it can be done with the command "sudo apt-get install python3-dev".

TUTORIAL    
--------

### SIMULATED ENVIRONMENT

The simulated environment is a 2D world were the pitch extends its length horizontally and its width vertically.
The center has coordinates (x,y) = (0,0) and the horizontal axis (x) increases toward right and decreases (becoming negative) toward left,  and the vertical axis increases toward the top and decreases (becoming negative) toward the bottom.
The size of the pitch can be set at initialization time to any preferred value, otherwise default settings will apply.
Also the number of players can be chosen during initialization (a different number of players for each team is possible). In fact, a lot of environment settings can be set at initialization time, look further in the documentation to have a complete description.
Players are supposed to be small robots moved by wheels, able to turn left/right up to 90 degrees and to move forward and backward. This means that at each command they can turn towards every possible angle (complete 360 degrees choice). They are moved by an engine through a "DASH" command that transmits an acceleration to the robot in the current direction.
A simplified acceleration and friction calculation is computed by the simulation, both for robots and ball. A simplified mechanism for collisions is computed too.
There is a limit to the maximum velocity that robots and ball may achieve.
Robots have a mechanism to kick the ball if it is within a certain distance to the robot and within a certain angle to the robot direction.
The first robot for each team is a goalkeeper and has the special mechanic to try to "catch" the ball when it is within a certain range (the "CATCH" command can be used only inside a goalkeeper own area). The catch mechanism is not deterministic and has a probability of success. When a goalkeeper manages to catch the ball it holds the ball for a limited number of ticks and during that time other players are not able to kick the ball or take it away from the goalkeeper.
Robots do not exhaust their battery so they can always work at full power (this is very different from the popular Robocup2D Soccer Simulator Server).
Robots know the full state of the world, since a complete information state is given to them. This exempts the player agent to the duty of building an internal representation from incomplete and uncertain information and permits the researcher to focus on other aspects of intelligence. But if you want to use this simulation to study partially observable states you still can: just write a function that processes the complete information state and transforms it in a partially observable state before sending it to the rest of your agent computation pipeline.
The rules of the game are quite similar to the ones of real soccer, with the exceptions that, in order to keep the strategy space as basic as possible, there is not the offside rule, and a player can pass the ball to himself during a corner/throw in/goal kick/kick off. Moreover there are not faults nor penalties. Corners, throw-ins, goal-kicks and kick-offs exist, and during their execution there are mandatory distances to be respected by opponents.: each time an event like corner, throw-in, goal kick or kick-off happens, opponent players are required to move in a location that respects the distance, with a special command "MOVE". If the distance is not respected, the simulation will automatically displace the player to the right distance. 
The match is composed by 2 half times (just like real games), the duration is typically 3000 ticks per half time (but may be set to any value), with a total of 6000 ticks. The time duration of a tick is considered to be 1/10th of a second, or in other words 10 ticks happen in a second, a match hence is supposed to have a duration of 600 seconds or 10 minutes. That is just an hypotetical convention, since the simulation is turn-based and not time-based, that implies that the hypotetical time duration of a tick does not really matter.
After the first half time the teams switch sides of the pitch to play the second half.

### Running a simple simulation

To run a simple simulation the first thing to do is to import robosoc2d:
```python
import robosoc2d
```
Then it is possible to run a game even without creating a player agent in Python: a built-in agent called SimplePlayer may be used. The purpose of having a default agent is to have a default opponent to test your agent against, but SimplePlayer can be used in each team so it is possible to have a team formed by SimplePlayer agents playing against another team made of SimplePlayer agents.
To launch a new simulation it is enough to call the function build_simpleplayer_simulator().
The first parameter of the function has to be a (possibly empty) sequence of custom players for the first team. We will see next how to create those players, but for now, since we are using SimplePlayer, we can leave that sequence empty (we may create an empty list). The second parameter is an integer representing how many SimplePlayer agents the simulator has to build for the fist team. We set this number to 4.
The third and fourth parameters are just like the first two but for the second team: third parameter is a sequence containing second team players (and we set it as an empty list), fourth parameters tells the simulator how many SimplePlayers to build for the second team, and we set this number to 4 too (it is possible to run games with a different number of players for each team, a team can even have 0 players, a feature that may be useful to test some opponent agent behaviour).
The function returns an integer that represents a handle to the simulation: it has to be stored by your program and used as a parameter to identify the simulation in subsequent function calls (it is possible to initialize and run an great number of simulations at the same time, limited only by memory).
So the new simulation initialization may be the following\:

```python
sim_handle = robosoc2d.build_simpleplayer_simulator([], 4, [], 4) 
```
The above line of code builds a new simulation, with both teams made of 4 SimplePlayers.
When creating a new simulation, the first player of each team is always a goalkeeper, that means that it can catch the ball through the "CATCH" command inside its own area, while other players can't. In case a team is using one or more SimplePlayers, those SimplePlayers will be the first players in the complete team sequence. That means that if you use one or more SimplePlayers the first of them will be your goalkeeper. If you want to have the role of goalkeeper assigned to a custom player you don't have to use SimplePlayers at all for that team.This is because SimplePlayers aren't really meant to be used as common players in your team but only as opponents or as a temporary goalkeeper replacement while you are developing the behavior of common players and don't want to focus on the goalkeeper for a while.
The build_simpleplayer_simulator() has other optional parameters: the fifth parameter is an integer containing the random seed used to initialize the random number generator of the simulation, se sixth parameter is a reference to a robosoc2d.settings object containing settings to be used to build the simulation (more details inside the documentation).
To know which are the default settings of the simulator (and possibly modify them to use them as an initialization parameter) you can call\:
```python
default_setts=robosoc2d.get_default_settings()
```

Once the simulation is initialized it is possible to run a step of it\:
```python
robosoc2d.simulator_step_if_playing(handle=sim_handle)
```
The function above runs one step of the simulation, that is one tick, sending to each agent in a sequential way all needed information about settings, pitch, state and asking them to take an action. The function returns a boolean: True if the game is not ended, False otherwise.
So it is possible to play a complete game with the following two lines\:

```python
while robosoc2d.simulator_step_if_playing(sim_handle):
    pass
``` 

That is equivalent to the specific function that will play all the steps until the end\:
```python
robosoc2d.simulator_play_game(sim_handle)
``` 

To get a string containing some description of the state of the game you can call the function\:
```python
robosoc2d.simulator_get_state_string(sim_handle)
``` 

So you can run a complete simulation and output the description string for each tick in the following way\:
```python
while robosoc2d.simulator_step_if_playing(sim_handle):
    print(robosoc2d.simulator_get_state_string(sim_handle))
``` 

When the simulation is ended and you don't need it anymore, you can discard its information and release the memory with\:

```python
robosoc2d.simulator_delete(sim_handle)
``` 
After deleting a simulation, the related handle (its identificative integer) is not valid anymore, and the usage of it will result in an error.

It is possible to obtain the whole game state with the following function:
```python
simState = robosoc2d.simulator_get_game_state(sim_handle)
``` 
The returned state is a tuple containing 5 objects:
   * the 1st object is a robosoc2d.environment describing the current state of the game (score, tick, ball position and velocity etc.)
   * the 2nd object is a robosoc2d.pitch describing the playfield (the size of the borders, goals, areas, etc.)
   * the 3rd object is a robosoc2d.settings containing the settings of the simulator
   * the 4th object is a tuple of player_info that contains the position, velocity and other information of first team players (the tuple contains as many player_info objects as the number of players in first team)
   * the 5th object is a tuple of player_info that contains the position, velocity and other information of second team players 

The "documentation" section below describes all the fields of those objects in details. 

Example:
```python
simState = robosoc2d.simulator_get_game_state(sim_handle)
current_environment =simState[0]
``` 
It is not suggested to use simulator_get_game_state() inside a player agent's code, since the player will already have all the information supplied in a step callback (more about this later).


### Creating a player agent

We just saw how to launch and run a simulation using the default SimplePlayer, now it's time to know how to build your own player agent.
What you need to do is just to create a class that contains a metod named "step". This method will be called by the simulator at each tick.

The method has to accept 5 parameters, in addition to the mandatory first parameter "self": environment, pitch, settings, team1, team2 (their actual names don't really matter, the order matters).

The "environment" paramenter contains a robosoc2d.environment describing the current state of the game (score, tick, ball position and velocity etc.). It has to be noted that the field environment.state informs the agent if the game is in active play or in any other stage, such has when interrupted because a goal has just been scored, or when interrupted to kick a corner. The possible values for environment.state are: robosoc2d.STATE_INACTIVE, robosoc2d.STATE_KICKOFF1, robosoc2d.STATE_KICKOFF2, robosoc2d.STATE_PLAY, robosoc2d.STATE_GOALKICK1UP, robosoc2d.STATE_GOALKICK1DOWN, robosoc2d.STATE_GOALKICK2UP, robosoc2d.STATE_GOALKICK2DOWN, robosoc2d.STATE_CORNER1UP, robosoc2d.STATE_CORNER1DOWN, robosoc2d.STATE_CORNER2UP, robosoc2d.STATE_CORNER2DOWN, robosoc2d.STATE_THROWIN1, robosoc2d.STATE_THROWIN2, robosoc2d.STATE_HALFTIME, robosoc2d.STATE_GOAL1, robosoc2d.STATE_GOAL2, robosoc2d.STATE_ENDED

The "pitch" parameter contains a robosoc2d.pitch describing the playfield (the size of the borders, goals, areas, etc.).

The "settings" parameter contains a robosoc2d.settings with the settings of the simulator.

The "team1" parameter is a tuple of player_info that contains the position, velocity and other information of first team players (the tuple will contains as many player_info objects as the number of players in the team). 

The "team2" parameter is a tuple of player_info that contains the position, velocity and other information of second team players. 

In the documentation below those all those classes are described in details.

The step() method of the player class then should contain the logic that decides which action to take. And it has to return the action as a sequence containing an integer and three floats. The integer is the first number of the sequence and indicates which action the player wants to do, choosing among robosoc2d.ACTION_NOOP, robosoc2d.ACTION_MOVE, robosoc2d.ACTION_DASH, robosoc2d.ACTION_KICK, robosoc2d.ACTION_CATCH.
The other three components of the sequence are three floats whose meaning depends on the type of action and are actually action's parameters.

A basic example of a player agent, that doesn't do much: it only counts ho many times is called and it moves continuously in the same direction towards right.
```python
class MyPlayer:
    def __init__(self):
        self.c=0
    def step(self, env, pitch, settings, team1, team2):  
        print("internal variable c="+str(self.c))
        self.c+=1
        action=(robosoc2d.ACTION_DASH, 0.0, 0.06, 0.0)
        return action

my_team=[MyPlayer() for n in range(4) ] # create 4 players
sim_handle = robosoc2d.build_simpleplayer_simulator(my_team, 0, [], 4) # launch a simulation with first Team made of 4 MyPlayers and second team made of 4 SimplePlayers
``` 
In fact the action robosoc2d.ACTION_DASH tells the simulator that the agent wants to accelerate, the first numerical parameter is the angle expressed in radians and is 0.0, that means "right", the second numerical parameter is the acceleration power 0.06, and the third is unused for this kind of action.

The simulator will call the step() method of each agent in random order at each tick. After calling the step() of an agent the simulator will update some vectorial value of the simulation, and call the step() method of another agent. When all agents have acted by their respective step() method the simulator will calculate the effect of the actions and update the environment and players' positions. The order of the agents whose step() method is called is random with two exceptions: (1) when the game state is in a throw-in, corner, goal-kick, or kick-off event, and (2) in the tick subsequent to that, when the game state is back to play.
In case of throw-in, corner, goal-kick, or kick-off, we will have a tick during which the game state is in one of the following states:

robosoc2d.STATE_KICKOFF1, robosoc2d.STATE_KICKOFF2, robosoc2d.STATE_GOALKICK1UP, robosoc2d.STATE_GOALKICK1DOWN, robosoc2d.STATE_GOALKICK2UP, robosoc2d.STATE_GOALKICK2DOWN, robosoc2d.STATE_CORNER1UP, robosoc2d.STATE_CORNER1DOWN, robosoc2d.STATE_CORNER2UP, robosoc2d.STATE_CORNER2DOWN, robosoc2d.STATE_THROWIN1, robosoc2d.STATE_THROWIN2. 

During that tick, the team that has to kick the ball to restart playing is the team to act first. The ball will be automatically positioned in the right location for throw-in/corner/kickoff/goal-kick and the player (of the kicking team) that is closer to the ball will be asked to act first (its step() method will be called). Then, in ascending numerical order, each other player of the kicking team. 
Finally, each player of the other team, in ascending numerical order. 

The only possible actions during that tick are robosoc2d.ACTION_NOOP and robosoc2d.ACTION_MOVE. 

NOOP means "no operations" and is used when an agent doesn't want to do any action at all. 

ACTION_MOVE is an action that automatically positions the agent in a certain well specified location. It may be used only when the game is not in the PLAY state. In case of kick-off each team can ACTION_MOVE players only in their respective half-pitch (the non kicking team has to avoid also its midfield half-circle).  
In case of throw-in, corner and goal-kick the kicking team has the freedom to ACTION_MOVE its agents in whatever position, and the maximum distance between previous and new position of its players will be registered in the field starting_team_max_range of the environment object to be passed to other team's players during their step() call. That value is the maximum distance that non-kicking team player are allowed to ACTION_MOVE. When players are moved by ACTION_MOVE the simulator will check if they are intersecting other players, or if they are too close to the ball during throw-ins and corners (as set in the fields throwin_min_distance and corner_min_distance in the robosoc2d.settings object), or if they are in forbidden zones (e.g. inside the opponent area during opponent goal-kick) and will displace them to the closest acceptable position.

In the tick after throw-in, corner, goal-kick, or kick-off, the game state is back to robosoc2d.STATE_PLAY and the possible actions are ACTION_DASH, ACTION_MOVE, ACTION_KICK, and for goalkeepers ACTION_CATCH. During this tick the player of the kicking team that is closer to the ball is the one whose step() method will be called first, and then all other players of both teams will be called randomly. 

To see an implementation of a more complex, yet still basic, player agent, check the HumblePlayer class in the humble_player.py module, that is a Python conversion of the C++ built-in SimplePlayer agent.

### The Actions

Let's see all the available actions and their parameters.
#### robosoc2d.ACTION_NOOP : 
This action means "no operations" and is used when the agent doesn't want to take action. The parameters are unused.

#### robosoc2d.ACTION_MOVE :
This action is used to place an agent in a certain location of the pitch, determined by coordinates. It is possible to use it only when the game is not in the PLAY state. The simulator will check that the location is valid depending on the rules, and displace the agent in the appropriate position in case it's not. The first parameter is the x coordinate where to locate the player, the second parameter is the y coordinate, and the third parameter is the player direction (where the player will face) expressed in radians.

#### robosoc2d.ACTION_DASH : 
This actions impresses an acceleration to the agent towards a certain direction, in other words it is the way agents move around.
The first parameter is the angle towards which the player will turn before dashing: it will be both the new facing direction of the player and the dash direction. It is expressed in radians. The second parameter is the power and has to be between zero and the field max_dash_power in the robosoc2d.settings object passed as parameter in the step() method. The third parameter is unused

#### robosoc2d.ACTION_KICK : 
This action permits to kick the ball, if it is in reachable range.
The first parameter is the angle indicating the direction of the kick (where the ball would go if it was static at the moment of the kick), expressed in radians. The second parameter is the power of the kick and has to be between zero and the field max_kick_power in the robosoc2d.settings object passed as parameter in the step() method. The third parameter is unused. In the "simplified" modality of simulation ("simplified" flag of "settings" object set to True, as in default mode) the ball can be kicked even if it's behind the player, towards any direction (as if the agent can capture it and move it to the front). With the "simplified" flag set to False instead, the ball can be kicked only if it's in front of the player, that means that the segment between the center of the ball and the center of the player has to form a directional vector with respect to the center of the player whose angle difference with the player direction vector is not bigger than the value contained in the field kickable_angle in the robosoc2d.settings object passed as parameter in the step() method. Another requisite for the ball to be kicked is that the distance between the center of the ball and the center of the player is less than kickable_distance (field of robosoc2d.settings), and this requisite has to be satisfied both with simplified and not simplified modality. In non-simplified play, there is a further requisite to be fulfilled when kicking the ball, except during throw-ins and corners, and it's that the difference between the kick direction and the player direction is less than the kickable_direction_angle field in the settings object.

#### robosoc2d.ACTION_CATCH :
All parameters are unused.
Only the goalkeeper (the first player) can catch the ball, and he can do that only inside its own area. If he manages to catch the ball, the ball will be moving with him and other players can't kick it. The ball will stay caught for as many ticks as specified in the field catch_holding_ticks of robosoc2d.settings object, after which it will be automatically released in front of the goalkeeper. The ball can be catched only if the distance between the center of the ball and the center of the player is less than the value of the catchable_distance field  of robosoc2d.settings object. Another requisite for catching the ball is that the angle between player direction and ball direction is less than the value of catchable_angle field of robosoc2d.settings object. If all requisite are fulfilled, the goalkeeper will have a probability to actually catch the ball equal to the value of the field catch_probability of robosoc2d.settings.

### GUI tutorial
The robosoc2dgui package is a separated package that makes possible to display a robosoc2d game in realtime. It is based on Tkinter so it will not be possible to use it on those Python versions that are natively missing it (in which case we suggest you to use instead the package "robosoc2dplot").
You can install it either via pip or via source code. 
To install via pip just run:
```
pip install robosoc2dgui
```

If instead you downloaded the source code you can set the current folder to ./pyextension/robosoc2d and the run "python3 setup.py install".

Displaying a game in realtime is as simple as calling a function: robosoc2dgui.gui.play_whole_game(sim_handle) 

Example \:
```python
import robosoc2d
import robosoc2dgui.gui as r2gui
sim_handle = robosoc2d.build_simpleplayer_simulator([], 4, [], 4)
r2gui.play_whole_game(sim_handle)
```
For a more detailed introduction, please look at the robosoc2dgui package, that contains its own documentation and examples.

### Saving game history
There are two functions to save the game history\:
```python
robosoc2d.simulator_save_state_history (handle, filename)
robosoc2d.simulator_save_actions_history (handle, filename)
```
The former saves the data about game settings and players' position and velocity at each tick. The latter saves the data about the actions done by each player at each tick.
To understand the file format in detail it is recommended to watch the cpp source code inside the file "simulator.cpp", it is self explanatory.
Alternatively, the file format for the state history is clearly understandable watching the python source code of the GUI log player contained in the folder "logplayer", specifically the file "r2files.py". The log player can be run with "python3 log_player.py", and it does not need to have the robosoc2d package installed.

### Other graphics packages
There is also another package to visualize the game in realtime. It's the package "robosoc2dplot" that may be installed either via source code (if you are downloading the source code you can find it in the folder ./pyextension/robosoc2dplot) or via pip. 
That package makes possibile to show the game inside a Jupyter notebook, both in local and remote servers, also in Kaggle notebook and Google Colab. It is based on Matplotlib and it does not use Tkinter, so it will work also on those Python versions that are natively missing Tkinter. The package contains its own documentation and examples.
There is also an experimental package named robosoc2dplotty whose aim is again to show the game inside Jupyter notebooks, using Plotly istead of Matplotlib, but unfortunately some Jupyter versions create some different behavior for some Plotly functionality in a way that may make robosoc2dplotty not working. For this reason we recommend the usage of robosoc2dplot (the Matplotlib based version) instead of robosoc2dplotty (the Plotly based version).

### Automatic Jupyter notebook generation

If you downloaded the source code of robosoc2d, in the folder "./pyextension/create_notebook" there is the file "create_notebook.py" that, when run (having the current directory set to its own folder), will create a notebook file named "robosoc2d.ipynb" that permits to use robosoc2d inside Jupyter notebooks even without installing robosoc2d with pip: this ipynb file contains all the C++ and Python files needed to compile and install robosoc2d and its side packages. As of the time of writing, the resulting notebook is tested to work on current Google Colab and Kaggle Notebook.


DOCUMENTATION
-------------
### Classes:

settings

pitch

environment

player_info

error

### Functions:

get_default_settings ()

get_seed_by_current_time ()

build_simulator (team1, team2, team1name, team2name, random_seed, game_settings) 

build_simpleplayer_simulator (team1, how_manysimpleplayers_team1, team2, how_manysimpleplayers_team1, team1name, team2name, random_seed, game_settings)

simulator_step_if_playing (handle)

simulator_play_game (handle)

simulator_delete (handle)

simulator_delete_all ()

simulator_is_valid (handle)

simulator_get_state_string (handle)

simulator_get_game_state (handle)

simulator_get_random_seed (handle)

simulator_get_team_names (handle)

simulator_save_state_history (handle, filename)

simulator_save_actions_history (handle, filename)

remainder (dividend, divisor)


### Constants:

STATE_INACTIVE

STATE_READY

STATE_KICKOFF1

STATE_KICKOFF2

STATE_PLAY

STATE_STOPPED

STATE_GOALKICK1UP

STATE_GOALKICK1DOWN

STATE_GOALKICK2UP

STATE_GOALKICK2DOWN

STATE_CORNER1UP

STATE_CORNER1DOWN

STATE_CORNER2UP

STATE_CORNER2DOWN

STATE_THROWIN1

STATE_THROWIN2

STATE_PAUSED

STATE_HALFTIME

STATE_GOAL1

STATE_GOAL2

STATE_ENDED

ACTION_NOOP

ACTION_MOVE

ACTION_DASH

ACTION_TURN

ACTION_KICK

ACTION_CATCH


robosoc2d.settings
------------------

Object containing the settings of the simulation.

The method copy() returns a binary copy of this object.

Converting an object to string and then evaluating that string with eval()
will result in a dictionary containing all the fields of the object.

E.g. my_dict=eval(str(my_settings))

### Attributes\:

simplified \: boolean -> set it to False if you want a more realistic ball control. Set it to True (default value) if you want a simplified simulation that eases the ball control to the agent and relaxes constraints on ball kicking (in this modality agents can kick the ball even behind their back towards any position, as if they can capture the ball from one side and release it from the other)

ticks_per_time \: integer -> duration of each half time, expressed in ticks (a tick is a time step). The total duration of a match is 2*ticks_per_time (since there are 2 half times)

pitch_length \: float -> length of the pitch, that in the simulation extends along horizontal axes. pitch_length is the length of the playable zone, delimited by end lines. If during play the ball exit that zone there will be a goal-kick or a corner kick, but players can anyway move and dash outside that zone in a zone that is larger by the quantity out_pitch_limit (the actual coordinates are calculated in the object robosoc2d.pitch, in pitch.border_left and pitch.border_right)

pitch_width \: float -> width of the pitch, that in the simulation extends along vertical axes. pitch_width is the width of the playable zone, delimited by side lines. If during play the ball exit that zone there will be a throw-in, but players can anyway move and dash outside that zone in a zone that is larger by the quantity out_pitch_limit (the actual coordinates are calculated in the object robosoc2d.pitch, in pitch.border_up and pitch.border_down)

goal_width \: float -> width of goals, in vertical coordinates. It is the internal width, excluding poles. External width including poles instead is equal to (goal_width + (pole_radius\*2)\*2)

center_radius \: float -> radius of center circle of the pitch

pole_radius \: float -> radius of poles

ball_radius \: float -> radius of the ball

player_radius \: float -> radius of the simulated circular player robot

catch_radius \: float -> maximum reach of goalkeepers, that contributes to determine catchable_distance

catch_holding_ticks \: integer -> duration in ticks of the period in which goalkeepers can hold the ball once catched

kick_radius \: float -> maximum length of a kick, that contributes to determine kickable_distance

kickable_distance \: float -> distance to ball within which players can kick the ball. It equals (kick_radius+player_radius+ball_radius)

catchable_distance \: float -> distance within which goalkeepers can catch the ball. It equals (catch_radius+player_radius+ball_radius)

kickable_angle \: float -> maximum angle between player direction and player-ball direct line that permits the player to kick the ball (if the angle is greater the player will not be able to reach the ball for kicking)

kickable_direction_angle \: float -> maximum angle between player direction and the direction of intendend kick to the ball. (The intended direction of a kick is usually expressed as in the first float value in the action tuple, after the integer value robosoc2d.ACTION_KICK). If the angle is greather than kickable_direction_angle, the player will not be able to kick the ball.

catchable_angle \: float -> maximum angle between goalkeeper direction and goalkeeper-ball direct line that permits the goalkeeper to catch the ball (if the angle is greater the goalkeeper will not be able to reach the ball to catch it)

net_length \: float -> the lenght, in horizontal units, of the net structure of goals.

catchable_area_length \: float -> the lenght, in horizontal units, of the catchable area, where goalkeepers can catch and hold the ball

catchable_area_width \: float -> the width, in horizontal units, of the catchable area, where goalkeepers can catch and hold the ball

corner_min_distance \: float -> the minimum distance that opponents have to keep from ball during a corner kick. If opponents are closer, the simulator will automatically move them farther. 

throwin_min_distance \: float -> the minimum distance that opponents have to keep from ball during athrow-in. If opponents are closer, the simulator will automatically move them farther. 

out_pitch_limit \: float -> a distance external from pitch limits within which players can move and dash

max_dash_power \: float -> maximum possible value for dash power (the dash power is usually expressed as second float value in the action tuple, after the integer value robosoc2d.ACTION_DASH and the first float value containing the dash direction)

max_kick_power \: float -> maximum possible value for kick power (the kick power is usually expressed as second float value in the action tuple, after the integer value robosoc2d.ACTION_KICK and the first float value containing the kick direction)

player_velocity_decay \: float -> decay in player velocity at each step. In common usage of the simulator you should not feel the need to either read or set this value. See the simulator source code for greater details

ball_velocity_decay \: float -> decay in ball velocity at each step. In common usage of the simulator you should not feel the need to either read or set this value. See the simulator source code for greater details

max_player_speed\: float -> maximum speed reachable by players

max_ball_speed \: float -> maximum speed reachable by ball

catch_probability \: float -> probability that goalkeepers succeed in catching the ball in case the ball is at appropriate distance and angle

player_random_noise \: float -> constant used to calculate and add some random noise to player dash and kicking power (and to ball movement). In common usage of the simulator you should not feel the need to either read or set this value.  See the simulator source code for greater details

player_direction_noise \: float -> constant used to calculate and add some random noise to player dash and kicking direction. In common usage of the simulator you should not feel the need to either read or set this value. See the simulator source code for greater details

player_velocity_direction_mix \: float -> constant used in the formula to calculate player inertia. In common usage of the simulator you should not feel the need to either read or set this value.  See the simulator source code for greater details

ball_inside_player_velocity_displace \: float -> constant used in the formula to calculate player inertia. In common usage of the simulator you should not feel the need to either read or set this value.  See the simulator source code for greater details

after_catch_distance \: float -> constant used to calculate the position of the ball when the goalkeeper that caught the ball releases the ball after the catch time has terminated. The ball will be approximately at a distance equal to (player_radius+ball_radius+after_catch_distance) from the position of the goalkeeper, along the goalkeeper direction vector

robosoc2d.pitch
---------------

Object containing information about the game pitch.

The method copy() returns a binary copy of this object.

Converting an object to string and then evaluating that string with eval()
will result in a dictionary containing all the fields of the object.

E.g. my_dict=eval(str(my_pitch))

### Attributes\:

x1 \: float -> horizontal coordinate of right pitch boundary (usually a positive number)

x2 \: float -> horizontal coordinate of left pitch boundary (usually a negative number = -x1)

y1 \: float -> vertical coordinate of upper pitch boundary (usually a postiive number)

y2 \: float -> vertical coordinate of lower pitch boundary (usually a negative number = -y1)

x_goal1 \: float -> horizontal coordinate of the right end of the right goal considering the net. The right goal begins at coordinate x1 (at the right pitch boundary, where if the ball surpass the vertical line you score a goal) and it ends at coordinate x_goal1 

x_goal2 \: float -> horizontal coordinate of the left end of the left goal considering the net. The lect goal begins at coordinate x2 (at the left pitch boundary, where if the ball surpass the vertical line you score a goal) and it ends at coordinate x_goal2

y_goal1 \: float -> vertical coordinate of the upper end of the goals, where you have upper poles

y_goal2 \: float -> vertical coordinate of the lower end of the goals, where you have lower poles

area_lx \: float -> right horizontal boundary of the left area (the left area has left horizontal boundary in x2)

area_rx \: float -> left horizontal boundary of the right area (the right area has right horizontal boundary in x1)

area_uy \: float -> upper vertical boundary of areas

area_dy \: float -> lower (down) vertical boundary of areas

goal_kick_lx \: float -> horizontal coordinate of left goal kicks position

goal_kick_rx \: float -> horizontal coordinate of right goal kicks position

goal_kick_uy \: float -> vertical coordinate of upper goal kicks position

goal_kick_dy \: float -> vertical coordinate of lower (down) goal kicks position

pole1x \: float -> horizontal coordinate of left upper pole

pole1y \: float -> vertical coordinate of left upper pole

pole2x \: float -> horizontal coordinate of left lower pole

pole2y \: float -> vertical coordinate of the left lower pole

pole3x \: float -> horizontal coordinate of right upper pole

pole3y \: float -> vertical coordinate of right upper pole

pole4x \: float -> horizontal coordinate of right lower pole

pole4y \: float -> vertical coordinate of right lower pole

border_up \: float -> vertical coordinate of the extreme upper end of the playfield, the more external upper coordinate where a player can be (it's outside the pitch)

border_down \: float -> vertical coordinate of the extreme lower end of the playfield, the more external lower coordinate where a player can be (it's outside the pitch)

border_left \: float -> horizontal coordinate of the extreme left end of the playfield, the more external left coordinate where a player can be (it's outside the pitch)

border_right \: float -> horizontal coordinate of the extreme right end of the playfield, the more external right coordinate where a player can be (it's outside the pitch)


robosoc2d.environment
---------------------

Object containing the state of the simulation environment.

The method copy() returns a binary copy of this object.

Converting an object to string and then evaluating that string with eval()
will result in a dictionary containing all the fields of the object.

E.g. my_dict=eval(str(my_environment))

### Attributes\:

tick \: integer -> the current tick (a tick is a timestep of the game)

score1 \: integer -> the score by first team

score2 \: integer -> the score by second team 

state \: integer -> an integer representing the state of the game, such as robosoc2d.STATE_PLAY . Possible values, all inside the robosoc2d module, are: STATE_INACTIVE, STATE_READY (unused), STATE_KICKOFF1, STATE_KICKOFF2, STATE_PLAY, STATE_STOPPED (unused), STATE_GOALKICK1UP, STATE_GOALKICK1DOWN, STATE_GOALKICK2UP, STATE_GOALKICK2DOWN, STATE_CORNER1UP, STATE_CORNER1DOWN, STATE_CORNER2UP, STATE_CORNER2DOWN, STATE_THROWIN1, STATE_THROWIN2, STATE_PAUSED (unused), STATE_HALFTIME, STATE_GOAL1, STATE_GOAL2, STATE_ENDED

ball_x \: float -> horizontal coordinate of ball position

ball_y \: float -> vertical coordinate of ball position

ball_velocity_x \: float -> horizontal component of ball's velocity vector

ball_velocity_y \: float -> vertical component of ball's velocity vector

last_touched_team2 \: boolean -> set to True if last player to touch or kick the ball was belonging to second team, or set to False if belonging to first team"

starting_team_max_range \: float -> maximum range of movement for players of the non-kicking team in case of throw-in, corner kick, or goal-kick

ball_catched  \: integer -> number of ticks still available to goalkeeper to hold the ball. It's zero if the ball is not held caught by any goalkeeper.

ball_catched_team2 \: boolean -> in case a goalkeeper has catched the ball this attribute is set to True if it's the second team's goalkeeper, or set to False if it's the first team's goalkeeper. In case the ball is not caught, it contains the value for the last time it has been.

halftime_passed \: boolean -> this is set to True after the second half of the match has begun, otherwise it is set to False

n_players1  \: integer -> number of players of first team

n_players2  \: integer -> number of players of second team

robosoc2d.player_info
---------------------

Object representing information about a player.

The method copy() returns a binary copy of this object.

Converting an object to string and then evaluating that string with eval()
will result in a dictionary containing all the fields of the object.

E.g. my_dict=eval(str(my_player_info))

### Attributes\:

x \: float -> horizontal coordinate of player position

y \: float -> vertical coordinate of player position

velocity_x \: float -> horizontal component of player's velocity vector

velocity_y \: float -> vertical component of player's velocity vector

direction \: float -> direction of the player, expressed in radians

acted \: boolean -> set to True if player has alraedy acted in current Tick


functions:
----------

#### get_version()

It returns a string containing the version of the simulator. It uses the format: "major.minor.revision".

#### get_default_settings()

It returns a settings object containing the default settings of simulator. No parameters.

#### get_seed_by_current_time()

It returns a random seed generated by current time. No parameters.

#### build_simulator(team1, team2, team1name, team2name, random_seed, game_settings)

It creates a simulator and returns an integer that represents an handle to it. First and second parameters are mandatory and must be sequences of players, one sequence per team. It is possible to have a different number of players in each team. The other parameters are not mandatory. The third and fourth parameters are strings containing team names. Fifth parameter is an integer containing the random seed to be used to initialize the random engine (if this parameter is missing, a random seed will be generated depending on current time). Sixth parameter is a settings object in case you want to choose you own settings. (a player is an object that implements the method step(self, env, pitch, settings, team1, team2) that receives the information about the game and returns the choosen action as a tuple composed by one integer and three floats)

#### build_simpleplayer_simulator (team1, how_manysimpleplayers_team1, team2, how_manysimpleplayers_team1, team1name, team2name, random_seed, game_settings)

It creates a simulator and returns an integer that represents an handle to it. For each team it is possible to use the built-in class SimplePlayer for some players. The user may decide how many SimplePlayer agent each team may have: the first players of the team will be the SimplePlayers ones (if any), and the subsequent players will be the ones inserted in the team sequences (that may possibly be empty). Since the SimplePlayers will be the first players of the team, and since the first player plays in the goalkeeper role, if a team has at least a SimplePlayer, it means the the goalkeeper will be certainly a SimplePlater. 

Parameters 1-4 are mandatory. First parameter must be a sequence containing the players objects for the first team, and second parameter is a boolean determining how many SimplePlayers have to be added at the beginning of the team. Third and fourth parameters are the same for the second team. It is possible to have a different number of players in each team. The other parameters are not mandatory. The fifth and sixth parameters are strings containing team names. Seventh parameter is an integer containing the random seed to be used to initialize the random engine (if this parameter is missing, a random seed will be generated depending on current time). Eighth parameter is a settings object in case you want to choose you own settings.(a player is an object that implements the method step(self, env, pitch, settings, team1, team2) that receives the information about the game and returns the choosen action as a tuple composed by one integer and three floats)

#### simulator_step_if_playing (handle)

It runs a step of the simulation, if the simulation is still playable and not terminated. It accepts only one parameter: an integer that is an handle to the simulation. It returns a boolean that is false if the game was still playable.

#### simulator_play_game (handle)

It runs all the step of the simulation till the end. It accepts only one parameter: an integer that is an handle to the simulation. It returns a boolean containing True if the game was played, throws an exception otherwise.

#### simulator_delete (handle)

It deletes a simulator. It accepts only one parameter: an integer that is an handle to the simulation. It returns a boolean containing True if the simulation has been deleted, False otherwise.

#### simulator_delete_all ()

It deletes all simulators. No parameters. It returns None

#### simulator_is_valid (handle)

It checks if handle passed as parameter refers to an existing simulator, and returns a boolean accordly"},

#### simulator_get_state_string (handle)

It returns the state string of simulator. It accepts only one parameter: an integer that is an handle to the simulation.

#### simulator_get_game_state (handle)

It returns the game state of the simulator. It accepts only one parameter: an integer that is an handle to the simulation. It returns a tuple containing 5 objects: the first is an environment object, the second is a pitch object, the third is a settings object, the fourth is a tuple of player_info objects containing the infromation about first team players, and the fifth object is a tuple of player_info for the secondo team.

#### simulator_get_random_seed (handle)

It returns the random seed of the simulator. It accepts only one parameter: an integer that is an handle to the simulation.

#### simulator_get_team_names (handle)

It returns the team names as a tuple containing two strings. It accepts only one parameter: an integer that is an handle to the simulation.

#### simulator_save_state_history (handle, filename)

It saves the state history of the simulator. The first parameter is an integer that is an handle to the simulation. The second parameter is the file name. It returns a boolean representing success (True) or failure (False).

#### simulator_save_actions_history (handle, filename)

It saves the actions history of the simulator. The first parameter is an integer that is an handle to the simulation. The second parameter is the file name. It returns a boolean representing success (True) or failure (False).

#### simulator_set_environment(handle, tick, score1, score2, state, ball_x, ball_y, ball_velocity_x, ball_velocity_y, team1, team2, last_touched_team2,  ball_catched,  ball_catched_team2)

It sets the game configuration, making possible to decide things like players' and ball's positions, velocities and so on.
The first parameter is an integer that is an handle to the simulation. The second parameter is an integer that represents the time tick.
Third and fourth parameters are integers representing current score for the two teams. The fith parameter is an integer indicating the state of the game, to be picked up among the constants robosoc2d.STATE_* , for instance robosoc2d.STATE_PLAY . Next 4 parameters are floats containing ball position and velocity. Then the parameter team1 is a sequence containing objects of type robosoc2d.player_info that specify position, velocity and direction of the players of the first team. The parameter team2 is the same for the second team. Then there are 3 optional parameters: last_touched_team2 is a bool that indicates if the last team that touched the ball was team2. The integer ball_catched indicates if the ball is currently owned by a gall keeper: if >0 the ball is catched and only the goalkeeper can kick it and the ball moves with him, thenumber actually indicates for how many ticks the ball is still allowed to be possessed by the goalkeeper. The boolean ball_catched_team2 should be true if the goalkeeper that owns the ball is the one of second team.


#### remainder (dividend, divisor)

Math remainder function following IEEE754 specification. Helper method for people using Python prior to 3.7 because Python<3.7 doesn't have true IEEE754 remainder (Numpy neither). This should be equivalent to Python 3.7: math.remainder(dividend, divisor)  

Simulation mechanics
====================
The simulator inner mechanics details, even if simplified, are easier to be described algorithmically than by words. The C++ distribution with the simulator core is available as open source (as well as the Python extension and packages) and the code is almost self-explaining, so that could be the right place to look.

Citing the Project
---------------------
To cite this repository in publications:

```bibtex
@misc{robosoc2d,
  author = {Ruggero Rossi},
  title = {Very Simplified 2D Robotic Soccer Simulator},
  year = {2021},
  publisher = {GitHub},
  journal = {GitHub repository},
  howpublished = {\url{https://github.com/rug/robosoc2d}},
}
```
