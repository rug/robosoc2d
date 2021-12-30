# robosoc2d
Very Simplified 2D Robotic Soccer Simulator
===========================================

### (c) 2021 Ruggero Rossi

This is an extremely simplified 2-dimensional robotic soccer simulator.
The rationale for its existence is the will to study agents emergent behaviour, hence the need to have a simulation that permits to focus on agents interactions, rather than focusing on how agents sense the world and model the sensory inputs. Within this perspective there is no need to simulate a lot of environmental details to make it as much realistic as possible, and agents are given complete state information about the world.
Moreover it is experiment-oriented rather than competition-oriented: it aims at generating simulations as fast as possible in order to have quickly a good amount of experiments to analyze. To do so, simulation is not in real time but it is turn-based: you don't have to wait a certain amount of time to simulate the next step, and this can give a great speedup. For the same reason, agents do not communicate with simulator by TCP/IP connections: agents are compiled together with simulation code and communicate only by function calls. This means that this simulator is not designed for competitions because of two aspects that are demanded to agents and can't be controlled by simulation: (1) time limits on computations (the simulation is waiting for an action from the agent, without control retrievement), (2) security issues (the simulation may run on the same process of the agents and a malicious agent may easily try to hack it). This does not mean that competitions can't be held, but in case, it is necessary for teams to provide source code to be inspected, and that agents be self-limiting their computation time.
On the other hand those characteristics make the simulator fit for massive amounts of simulations that scale with processing power, so it can be proficiently used to scientifically study the behaviour of agents or to train Reinforcement Learning agents, and it turns out to be very fast even on modest hardware (single core, low frequency).

### SIMULATED ENVIRONMENT

The simulated environment is a 2D world were the pitch extends its length horizontally and its width vertically.
The center has coordinates (x,y) = (0,0) and the horizontal axis (x) increases toward right and decreases (becoming negative) toward left,  and the vertical axis increases toward the top and decreases (becoming negative) toward the bottom.
The size of the pitch can be set at initialization time to any preferred value, otherwise default settings will apply.
Also the number of players can be chosen during initialization (a different number of players for each team is possible). 
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

INSTALLATION
------------
The simulator itself is realized in C++ and may be run easily adding its sources files to your own C++ project. So there is not a real "installation", you rather compile a project containing the simulator code. There is also a Python extension that permits to install the C++ simulator as a Python package and use it in Python (in that case there is an actual installation). To know more about the Python version please refer to the file README.md inside the folder "/pyextension". Here we will describe in more detail the C++ version.

The C++ version is written in C++17 and may be used in different ways:

    * using the default CMake project

    * inserting the source code inside your project in your IDE/build automation system

    * #including the source files in your program file (a special version of the above)


### compiling: C++ using CMake

The first way is to use CMake. The source code contains a CMakeLists.txt file that sets up a CMake project ready to be built and run.
The output of that project are two executables: "robosoc2dc" and "robosoc2dgui".
The former runs a simulation in console/textual mode, in a instantaneous way, outputting only the state message text per each tick. The latter executable runs a simulation launcher inside a window, with a GUI, visualizing graphically the pitch and the players. It pauses a little between a simulation tick and the next in order to let the viewer understand what's going on (it appears as a continuous gameplay). You may want to run the gui version to observe the behaviour of your agents, while you may want to run the console/textual version to run a series of experiments or to use reinforcement learning/machine learning.
You need to have the wxWidgets library installed in your system, otherwise the "robosoc2dgui" will not be built. If you don't need the "robosoc2gui" executable or if you are building your own executable using a different gui system, it's not mandatory to install wxWidgets, since the CMake project will still build "robosoc2d" (the console/textual version).
Keep in mind that if you are using Windows you should set the environment variable "WXWIN" pointing to the installation folder of wxWidgets, to make possible for CMake to find it. Moreover, if you are using Windows, keep in mind that CMake does not copy the wxWidgets dlls aside the built executables, so you either have to set their path as a system path for dlls or to manually copy them in the same folder of your executables.
On Windows, a quick way to compile the project may be to use Visual Studio and select "open folder": it should automatically recognize that its a CMake project and compile it rightfully.
On Linux and Unix, the usual CMake commands should do the job. Alternatively, you can run the scripts: "build.sh","compile.sh", "run.sh" in the base folder.
For those who use Visual Studio Code, opening the project is quite easy, just select from the menu File -> Open Folder and it will automatically understand that it's a CMake project, and then you could just use the usual VSC debug/run icons or menu commands.
When using the CMake project if you want to customize your simulations keep in mind that the file "main.cpp" is the one to modify to customize the console/textual executable "robosoc2dc" while "main_gui.cpp" is the one to modify to customize the gui launcher "robosoc2dgui" (they are good code examples also to understand how to run the simulator). If you want to run the simulator using your own player agent, instead of the default "simple player", you will have to include the source files containing your agent inside the CMake project, or either you can modify the simple_player class files.
Any player agent has to be an object of a class derived from r2s::R2Player and has to implement the virtual method "step()", nothing else is mandatory for a player agent (more on this later).

### compiling: Inserting the source files inside your project

If you don't want to use CMake or if you feel that you want a different project structure, you can just insert the source code files inside your project in your favourite IDE/build automation system and they will most probably compile without any problem.
I'll write below a quick explanation of what every source file is about and its dependencies, so you will know how to insert them in your project. Make sure that all the files imported from robosoc2d stay in the same folder, or they will not be able to #include their headers.
Just remember to set the compiler on C++17 version, link the math library, and install and link wxWidgets if you are using the gui part.
Let's see each individual file:

#### vec2.h
This is a small include-only 2d vector class. It is needed by the simulator and you have to copy it in the same folder of the other robosoc2d source code files that you insert in your project

#### simulator.h
This is the header of the simulator itself. You have to #include this if you want to use the simulator, and it has to stay in the same folder of the other robosoc2d source files that you insert in your project.

#### simulator.cpp
This is the simulator core. If you insert this file in your project, copying also the two includes above ("vec2.h" and "simulator.h") you already have the whole complete simulator in your project. You only need to write your own player agent deriving it from the virtual class r2s::R2Player and implementing its "step()" method. More on this later.
_
#### simple_player.cpp
If you don't know yet how to write your own player agent, you can use the "r2s::SimpePlayer" class contained in this file. It's a simple reactive agent with no planning. You may also look up at the source code as an example about how to write agents.

#### simple_player.h
It's the necessary header for the previous file (SimplePlayer class)

#### main.cpp
It's a small file that creates an example of console/textual simulation and runs it. You don't need it in your project if you know how to run robosoc2d simulation. Anyway you may want to insert it in your project as a starting point and modify it according to your needs. This file needs the file "debug_print.h" to be in the same folder.

#### debug_print.h
It's just a small include that contains a couple of macros useful for debugging. It's used by the file above (main.cpp). If you don't insert "main.cpp" in your project, you won't need it.

#### r2gui.h and r2gui.cpp
You may or may not want to insert those files in your project. They contain the code to be able to create a gui simulation launcher that runs simulations in a window and visualize graphically the pitch and the players. These files are not strictly necessary for the simulator itself, so they may be discarded, for instance if you are running a series of simulation on a server. If you insert them in your project, it's your duty to set up your project as a wxWidgets project, including library linking and compiler switches (and you have to assure that wxWidgets is installed in your system). 

#### main_gui.cpp
It's a small file that creates an example of gui launcher to run simulations inside a window. It needs that your project contains "r2gui.cpp" and it #includes "r2gui.h".
You don't need it in your project if you already know how to create a gui through r2gui.cpp (or if you don't want to visualize graphically the games). 

### compiling: #including the source files inside your program

A special case of inserting the robosoc2d files in your project may be the quick'n'dirty #include .
With this method, you just #include the files that you need inside your main source code file.
Robosoc2d source code files are written in a way that makes them compile without problems if #included (provided that you put their header files in the same folder).
The drawbacks of this method are that if you want to use the simulator classes inside more than a file of yours, you may have duplicated code at best, naming conflicts at worst. So this method is suggested only if you are using the simulator classes inside only one of your source files, or if you know exactly what you are doing.

As an example, think of creating a file named "my_simulation.cpp", that contains the following lines of code\:

```cpp
#include "simulator.cpp"      // this will include also "vec2.h" and "simulator.h"
#include "simple_player.cpp"  // this will include also "simple_player.h"
#include "main.cpp"           // this will include also "debug_print.h"
```
on Linux you may compile it with:
```console
g++ my_simulation.cpp -std=c++17 -lm
```

That's it ! If you compile it now you will have an executable that runs a console/textual simulation.
More realistically, in your program you would not have the last #include "main.cpp", instead you would have your code that defines your player agent and that runs a series of simulations with the settings that you decide.

TUTORIAL
--------
In this section I will present a brief tutorial to learn quickly how to write your own player agent and use it in a simulation. I assume the reader already read the previous sections and understands the file dependencies, the needed #include files, knows how to compile the project and so on.

The class r2s::R2Simulator is the class that runs the simulation and contains all necessary data.
It is possible to set up a new simulation creating a new object of type R2Simulator\:
```cpp
std::unique_ptr<R2Simulator> simulator= std::make_unique<R2Simulator>(team1, team2, team1name, team2name, random_seed, settings);
```

If you look at the constructor in "simulator.h" you will know the types of parameters\: 
```cpp
R2Simulator(std::vector<std::shared_ptr<R2Player>> _team1, std::vector<std::shared_ptr<R2Player>> _team2, std::string _team1name, std::string _team2name,  unsigned int _random_seed, R2EnvSettings _settings) 
```

I copied a simplified method declaration omitting the default arguments: keep in mind that only the first 2 parameters are mandatory, the others have default values.
So what we need to create a new simulation are two vectors of shared pointers to R2Player (or derived class) objects.
Those two vectors contain pointers to actual player agents, that have to be created before creating the new simulation.
Player agents are just objects of any class derived from r2s::R2Player that implements the "step()" method, a method that the simulator will call at each tick to ask every agent which action to take.
We will see later how to write your own player class, but for now we will focus just on the simulation initialization so we will use a player class that is already available: SimplePlayer (from the files simple_player.h and simple_player.cpp).
SimplePlayer derives from R2Player and implements the method\:

```cpp
virtual R2Action SimplePlayer::step(const R2GameState gameState) override;
```

so it has all it needs to be used as a player.
It has the following constructor, that requires to pass as parameters the number of the player inside its team (the "shirt number": 0 if it is the first player of the team, 1 if it is the second player of the team, and so on), and the number that indicates if it is of the first team (=0) or of the second team (=1).
This is the constructor signature:

```cpp
SimplePlayer::SimplePlayer(int index, int _whichTeam=0);
```
So, let's try to build two teams composed by 4 SimplePlayer agents each, and create a new simulation \:

```cpp
std::vector<std::shared_ptr<R2Player>> team1, team2;

for (int i=0 ; i <4; i++){
    team1.push_back(std::static_pointer_cast<R2Player>(std::make_shared<SimplePlayer>(i, 0)));
}

for (int i=0 ; i <4; i++){
    team2.push_back(std::static_pointer_cast<R2Player>(std::make_shared<SimplePlayer>(i, 1)));
}

std::unique_ptr<R2Simulator> simulator= std::make_unique<R2Simulator>(team1, team2);
```
That's it !
You just created a new simulation.
Keep in mind that in each team the first player of the vector is considered the goalkeeper and it's the only one that is able to catch the ball through the "catch" action.
Now, if you want to run a step of simulation (provided that the game is not ended), you have just to call a method:
```cpp
simulator->stepIfPlaying()
```
That method returns a bool: true if the game is not ended, and false if the game is ended. In that way it is possible to run all steps of a game until the end. We can make a loop displaying a game information string obtained with the method R2Simulator::getStateString()
```cpp
while(simulator->stepIfPlaying()){
    std::cout << simulator->getStateString() << endl;
}
```

If you don't need to process the game data at each tick and you want just to play a complete game without interruptions, you can do that with just one method call:
```cpp
simulator->playMatch();
```

It is also possible to obtain full game data with getStateString()::getGameState()

```cpp
R2GameState gameState=simulator->getGameState();
```

The object gameState is of type R2GameState: it's a struct that has three fields\:
   * "sett" of type R2EnvSettings, containing the settings of the simulator
   * "env" of type R2Environment  a struct containing all the current information about the game: score, tick, ball position and velocity (as an object of type R2ObjectInfo), players position and velocity (as vectors of type R2PlayerInfo)
   * "pitch" of type R2Pitch, containing information about the playfield

You may want to peek into "simulator.h" and check all the fields of the 3 structs R2EnvSettings, R2Environment, R2Pitch.

Let's go back for a moment to the R2Simulator constructor to talk about the reamaining parameters that we omitted.
The _random_seed parameter is an integer used to initialize the seed of the random number generator: every time you run a simulatio using the same seed will end up producing exactly the same outcome. In other words the game will be played in exactly the same manner, since the simulator is deterministic (given a random seed), provided that also player agents choose actions in a deterministic way given their possible random seed. Unfortunately it seems that different platforms (e.g. gcc on Linux vs Visual C++ in Windows) use different random generators so you don't have to expect the same outcomes if you run the simulation with the same seed on different platforms, you will have the same results only if you repeat the simulation only on the same platform.

The last parameter in the R2Simulator constructor is _settings, an object of type R2EnvSettings. Through this parameter it is possible to customize almost every aspect of the simulation, such as the maximum velocity of ball, the pitch size and so on. You can look at that struct into "simulator.h" and examine its fields.

Now it's time to finally learn how to create your own player. As anticipated, a player agent is an object of a class that derives from R2Player and implements the method\:

```cpp
virtual R2Action step(const R2GameState gameState) override;
```

Doing that is enough to be a valid player agent ! 
At each tick the method step() of each player will be called by the simulator, passing all the relevant game information in the parameter gameState of type R2GameState, that is exactly the same type returned by the method R2Simulator::getGameState() that we discussed a little above. 
It has to be noted that the field gameState.env.state is an enum of type R2State that informs the agent if the game is in active play (R2State::Play) or in any other stage, such has when interrupted because a goal has just been scored (R2State::Goal1 or R2State::Goal2), or when interrupted to kick a corner (check the enum class R2State in "simulator.h" to see all possible values).

Inside the step() method you should put the logic of behaviour of your player (or call other methods that implement the logic). The method must return an object of type R2Action that is a struct describing the action to take.

Let's focus on the struct R2Action, which is the type of object that the step() method has to return. It has a field "action" of type R2ActionType that determines the kind of actions, and then a field named "data" that is an array of 3 doubles containing parameters for that action (for each kind of action the data parameters may have different meaning). Valid kind of actions are R2ActionType::NoOp (if the agent doesn't want to act), R2ActionType::Move (to place a player in a certain pitch location when the game is not in play state) , R2ActionType::Dash (to accelerate the player agent in a certain direction), R2ActionType::Kick (to kick the ball if reachable), R2ActionType::Catch (to catch the ball, only if you are a goalkeeper).

A very minimalistic example of a player agent, that does nothing but going to the right:
```cpp
class PoorPlayer : public R2Player {
private:
    int shirtNumber;
    int team;
public:

    PoorPlayer(int index, int _whichTeam=0):shirtNumber(index),team(_whichTeam){}
    ~PoorPlayer(){}; 

    virtual R2Action step(const R2GameState gameState) override{
         return R2Action(R2ActionType::Dash, 0.0, 0.03, 0.0); 
    }
};
```
In fact the action R2ActionType::Dash tells the simulator that the agent wants to accelerate, the first numerical parameter is the angle expressed in radians and is 0.0, that means "right", the second numerical parameter is the acceleration power 0.03, and the third is unused for this kind of action.

The simulator will call the step() method of each agent in random order at each tick. After calling the step() of an agent the simulator will update some vectorial value of the simulation, and call the step() method of another agent. When all agents have acted by their respective step() method the simulator will calculate the effect of the actions and update the environment and players' positions. The order of the agents whose step() method is called is random with two exceptions: (1) when the game state is in a throw-in, corner, goal-kick, or kick-off event, and (2) in the tick subsequent to that, when the game state is back to play.
In case of throw-in, corner, goal-kick, or kick-off, we will have a tick during which the game state is in one of the following states:

R2State::Kickoff1, R2State::Goalkick1up, R2State::Goalkick1down, R2State::Corner1up, R2State::Corner1down, R2State::Throwin1, R2State::Kickoff2, R2State::Goalkick2up, R2State::Goalkick2down, R2State::Corner2up, R2State::Corner2down, R2State::Throwin2

During that tick, the team that has to kick the ball to restart playing is the team to act first. The ball will be automatically positioned in the right location for throw-in/corner/kickoff/goal-kick and the player (of the kicking team) that is closer to the ball will be asked to act first (its step() method will be called). Then, in ascending numerical order, each other player of the kicking team. 
Finally, each player of the other team, in ascending numerical order. 

The only possible actions during that tick are R2ActionType::NoOp and R2ActionType::Move . 

NoOp means "no operations" and is used when an agent doesn't want to do any action at all. 

R2ActionType::Move is an action that automatically positions the agent in a certain well specified location. It may be used only when the game is not in the R2State::Play state. In case of kick-off each team can R2ActionType::Move players only in their respective half-pitch (the non kicking team has to avoid also its midfield half-circle).  
In case of throw-in, corner and goal-kick the kicking team has the freedom to R2ActionType::Move its agents in whatever position, and the maximum distance between previous and new position of its players will be registered in the field startingTeamMaxRange of the R2Environment object to be passed to other team's players during their step() call. That value is the maximum distance that non-kicking team player are allowed to R2ActionType::Move. When players are moved by R2ActionType::Move the simulator will check if they are intersecting other players, or if they are too close to the ball during throw-ins and corners (as set in the fields throwinMinDistance and cornerMinDistance in the R2EnvSettings object), or if they are in forbidden zones (e.g. inside the opponent area during opponent goal-kick) and will displace them to the closest acceptable position.

In the tick after throw-in, corner, goal-kick, or kick-off, the game state is back to R2State::Play and the possible actions are R2ActionType::Dash, R2ActionType::Move, R2ActionType::Kick, and for goalkeepers R2ActionType::Catch. During this tick the player of the kicking team that is closer to the ball is the one whose step() method will be called first, and then all other players of both teams will be called randomly. 

### The Actions

Let's see all the available actions and their parameters.
#### R2ActionType::NoOp : 
This action means "no operations" and is used when the agent doesn't want to take action. The parameters are unused.

#### R2ActionType::Move :
This action is used to place an agent in a certain location of the pitch, determined by coordinates. It is possible to use it only when the game is not in the R2State::Play state. The simulator will check that the location is valid depending on the rules, and displace the agent in the appropriate position in case it's not. The first parameter is the x coordinate where to locate the player, the second parameter is the y coordinate, and the third parameter is the player direction (where the player will face) expressed in radians.

#### R2ActionType::Dash : 
This actions impresses an acceleration to the agent towards a certain direction, in other words it is the way agents move around.
The first parameter is the angle towards which the player will turn before dashing: it will be both the new facing direction of the player and the dash direction. It is expressed in radians. The second parameter is the power and has to be between zero and the field maxDashPower in the R2EnvSettings object passed as parameter in the step() method. The third parameter is unused

#### R2ActionType::Kick : 
This action permits to kick the ball, if it is in reachable range.
The first parameter is the angle indicating the direction of the kick (where the ball would go if it was static at the moment of the kick), expressed in radians. The second parameter is the power of the kick and has to be between zero and the field maxKickPower in the R2EnvSettings object passed as parameter in the step() method. The third parameter is unused. The ball can be kicked only if it's in front of the player, that means that the segment between the center of the ball and the center of the player has to form a directional vector with respect to the center of the player whose angle difference with the player direction vector is not bigger than the value contained in the field kickableAngle in the R2EnvSettings passed as parameter in the step() method. Another requisite for the ball to be kicked is that the distance between the center of the ball and the center of the player is less than kickableDistance (field of R2EnvSettings). In non-simplified play, there is a further requisite to be fulfilled when kicking the ball, except during throw-ins and corners, and it's that the difference between the kick direction and the player direction is less than the kickableDirectionAngle field in the R2EnvSettings object.

#### R2ActionType::Catch :
All parameters are unused.
Only the goalkeeper (the first player) can catch the ball, and he can do that only inside its own area. If he manages to catch the ball, the ball will be moving with him and other players can't kick it. The ball will stay caught for as many ticks as specified in the field catchHoldingTicks of R2EnvSettings object, after which it will be automatically released in front of the goalkeeper. The ball can be catched only if the distance between the center of the ball and the center of the player is less than the value of the catchableDistance field  of R2EnvSettings object. Another requisite for catching the ball is that the angle between player direction and ball direction is less than the value of catchableAngle field of R2EnvSettings object. If all requisite are fulfilled, the goalkeeper will have a probability to actually catch the ball equal to the value of the field catchProbability of R2EnvSettings.

Now you should examine SimplePlayer code in "simple_player.h" and "simple_player.cpp" files, to have an example of a simple player agent.

### Initialization templates

There are some ready-to-use template functions in "simulator.h" that simplify the creation of a simulation. They require that the player agents class has a constructor that accepts only two parameters, two integers, the first of which is the shirt number (that is also the order of the player in the team, with 0 = goalkeeper), and the second of which is the team number (0 for the first team, 1 for the second). The PoorPlayer class seen before and the SimplePlayer class satisfy that requisite and can be used with those initialization templates.
Let's see an example that creates a simulator with 4 PoorPlayers in team 1 and 4 SimplePlayers in team 2, and a random seed=18, using only one template function, without the need to create the players before:

```cpp
std::unique_ptr<R2Simulator> simulator=  buildSimulator<PoorPlayer,SimplePlayer>(4, 4, "Poor Team", "Simple Team", 18);
```

Peeking into "simulator.h" you can see in details those template functions, some of them make possible to use a different class for the goalkeeper, for instance:
```cpp
template<typename team1Goalkeeper, typename team1Player, typename team2goalkeeper, typename team2Player>
std::unique_ptr<R2Simulator> buildSimulator(int nPlayers1, int nPlayers2, std::string team1name, std::string team2name, unsigned int random_seed, R2EnvSettings settings)
```
(the template signature above is shortened wrt the original one, I omitted the default parameters values. In fact, only the first two parameters nPlayer1 and nPlayers2 are mandatory, together with class types).

### GUI tutorial

If you are using the gui part of robosoc2d you have the possibility to run the game inside a window and see it happen in real time.
That works anyway only if you set up your application as a wxWidgets app: as already discussed above you have to install wxWidgets in your system, follow the wxWidgets programming paradigm, and link the wxWidgets library.
If you are using the robosoc2d CMAKE project, the wxWidgets library linking is already set up and the file main_gui.cpp is following the wxWidgets paradigm, you only need to assure that you have the wxWidgets library installed.
Looking the content of main_gui.cpp should be self-explaining with respect to what to do to create a gui launcher.
The whole main_gui.cpp content is the following:
```cpp
#include "r2gui.h"
#include "simulator.h"
#include "simple_player.h"

using namespace std;
using namespace r2s;

class R2App: public wxApp{
    bool OnInit();
};

wxIMPLEMENT_APP(R2App);

bool R2App::OnInit(){
    r2s_gui::R2Launcher<SimplePlayer,SimplePlayer> *launcher=new r2s_gui::R2Launcher<SimplePlayer,SimplePlayer>();
    return true;
}
```
apart from boilerplate code, there is really only one line that matters:
```cpp
r2s_gui::R2Launcher<SimplePlayer,SimplePlayer> *launcher=new r2s_gui::R2Launcher<SimplePlayer,SimplePlayer>();
```
The line above creates a R2Launcher object, a template object that needs only to have specified the classes of the players for each team, and automagically it will create a gui launcher.
The gui launcher will let you decide how many players per team to have and what random seed to use, and with just a click it will automatically create the 2 teams, crate the simulation, and create a window where the simulation will be displayed graphically.
The simulation window will permit you to start the imulation through the menu "File | Start Game" or pressing Ctrl-s . Then you can toggle shirt numbers and change game speed with menu or keyboard shortcuts.
The gui launcher will let you create as many simulations as you want.

If instead of the gui launcher you want to decide inside your program when and how to launch a simulation graphical window, but you want to use the same wxWidgets window type used by the gui launcher, you can!
You just have to set up your program as a wxWidgets program (as wrote before) and #included r2gui.h, in your code once the simulator is created and it's time to show it inside a window, you can do that with just two lines of code:
```cpp
r2s_gui::R2Frame *frame = new r2s_gui::R2Frame( TITLE_TEXT, wxPoint(10,10), wxSize(960,700), simulator);
frame->SetDoubleBuffered(true);
```

### Saving games

The class r2s::R2Simulator has 3 methods to save the current game information in textual format (this can be useful if you want to analyze the match.)

```cpp
bool R2Simulator::saveStatesHistory(std::string filename);
bool R2Simulator::saveActionsHistory(std::string filename);
bool R2Simulator::saveHistory()
```
The first method saves the data about game settings and players' position and velocity at each tick. The second method saves the data about the actions done by each player at each tick. To understand the file formats it is enough to watch the methods' source code, it is self explanatory. In both methods the filename parameter is optional: if omitted a default name will be used.
The third method will call both the first and second methods, using default file names.
In the source code, inside the "logplayer" folder there is an application named "log_player.py" that is a player for the states history saved files (it requires python 3 to be run): launching it with "python3 log_player.py" permits to watch the saved game with a GUI.

### Setting a certain configuration

In case you need to artificially set players and ball positions, or more in general you want to control a game state, there is a function that permits to set the environment. For instance this may be useful if you are doing Reinforcement Learning and you want to make agents learn from a certain configuration of the game.

```cpp
void R2Simulator::setEnvironment(int _tick, int _score1, int _score2, R2State _state, R2ObjectInfo _ball, 
    std::vector<R2PlayerInfo> _team1, std::vector<R2PlayerInfo> _team2,
    bool _lastTouchedTeam2, int _ballCatched, bool _ballCatchedTeam2)
```

example:
```cpp
auto env=simulator->getGameState().env;
std::vector<R2PlayerInfo> _team1(4);
std::vector<R2PlayerInfo> _team2(4);
_team1[0].pos.x=5.0;
_team1[0].pos.y=5.0;
_team1[0].velocity.x=5.0;
_team1[0].velocity.y=5.0;
_team1[1].pos.x=-5.0;
_team1[1].pos.y=-5.0;
_team1[1].velocity.x=-5.0;
_team1[1].velocity.y=-5.0;
_team1[2].pos.x=5.0;
_team1[2].pos.y=-5.0;
_team1[2].velocity.x=5.0;
_team1[2].velocity.y=-5.0;
_team1[3].pos.x=-5.0;
_team1[3].pos.y=5.0;
_team1[3].velocity.x=-5.0;
_team1[3].velocity.y=5.0;

_team2[0].pos.x=1.0;
_team2[0].pos.y=1.0;
_team2[0].velocity.x=5.0;
_team2[0].velocity.y=5.0;
_team2[1].pos.x=-1.0;
_team2[1].pos.y=-1.0;
_team2[1].velocity.x=-5.0;
_team2[1].velocity.y=-5.0;
_team2[2].pos.x=1.0;
_team2[2].pos.y=-1.0;
_team2[2].velocity.x=5.0;
_team2[2].velocity.y=-5.0;
_team2[3].pos.x=-1.0;
_team2[3].pos.y=5.0;
_team2[3].velocity.x=-1.0;
_team2[3].velocity.y=5.0;

R2ObjectInfo _ball(7.0, 0.0,-5.0,-1.0);
simulator->setEnvironment(122, 4, 0, R2State::Play, _ball, 
_team1, _team2, false, 0, false) ;
```
        

    

PYTHON VERSION
--------------
The simulator is also available as a Python extension package named "robosoc2d" with two side packages for visualization named "robosoc2dgui" and "robosoc2dplot". This makes possible to write player agents and run simulations in Python without writing a single line of C++. If you downloaded the source code you will find the code for the Python extension, as well as the installation instructions, documentation and examples in the folder "/pyextension". 

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