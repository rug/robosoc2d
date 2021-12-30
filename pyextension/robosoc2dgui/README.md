robosoc2dgui
============
a package to visualize robosoc2d simulations in realtime inside GUI windows
using Tkinter

it contains only one module: robosoc2dgui.gui

INSTALLATION from pip    
---------------------
Just run:
```
pip install robosoc2dgui
```

INSTALLATION from source code 
-----------------------------

open the terminal or shell and enter:

python3 setup.py install

instead, to uninstall it:

pip uninstall robosoc2dgui

robosoc2dgui.gui
----------------

The only module of the package, it's a GUI library to visualize robosoc2d
simulations inside windows


classes
    
  * GuiPlayer \: it creates the window and manages its updating. It may run the
simulation and visualize it, or let the user write a function to run the
simulation and visualize it step by step.
    
functions
    
  * play_whole_game \: it creates the window, runs the simulation and visualizes
                      it, all in one call, without the need to use the GuiPlayer
                      class above.
    
  * play_routine \: it creates the window and let the user decide how to run and
                   visualize the simulation through a user function. Using this
                   function will save the user from creating a GuiPlayer object
                   manually.
                
  * get_version \: to get the version of the library

Let's see in more detail classes and functions

Classes
-------
### GuiPlayer (class)

It creates a Tkinter gui window to show a robosoc2d simulation in real time.
Let's see its methods in detail, beginning with the constructor.


#### \_\_init\_\_ (self, sim_handle, title='Very Simplified 2D Robotic Soccer Simulator', width=840, height=540, team1color='blue', team2color='red', border1color = "black", border2color = "black", direction1color='white', direction2color='black', ball_color='yellow', pitch_color="green", lines_color="white", goal_color="gray")

sim_handle\: this is the only mandatory parameter and it should contain
                    an integer that is the handle of the simulation.
        
title\: is a string containing the tile of the window

width\: the width of the window

height\: the height of the window

team1color\: tkinter compatible color for team1

team2color\: tkinter compatible color for team2

border1color\: tkinter compatible color for the outline of team1
    
border2color\: tkinter compatible color for the outline of team2

direction1color\: tkinter compatible color used for the first team to draw
                    a line inside the player that indicates player's direction

direction2color\: tkinter compatible color used for the second team to draw
                    a line inside the player that indicates player's direction

ball_color\: tkinter compatible color for the ball

pitch_color\: tkinter compatible color for the pitch

lines_color\: tkinter compatible color for the lines
                 
goal_color\: tkinter compatible color for the goal


#### play_whole_game (self, waiting_time=0.09):
        
This method of the GuiPlayer class plays a whole game and shows it in realtime
inside a Tkinter window. 

Parameter\:

waiting_time\: is a float that will be passed to your callable routine (the second
            parameter) as a suggestion about how much time (in seconds) to wait
            between simulation steps (a good value is the default 0.09). Your
            routine may ignore this value.


#### play_routine (self, routine, waiting_time=0.09)
        
This method of the GuiPlayer class calls a user-defined function or callable
as soon as the gui window is created. It may be useful to create custom inner
loops for the simulation, controlling what to do and how to use data between a
simulation step and the other. If you don't have this necessity, you may want
to use the play_whole_game() method instead.

routine\: it should contain a function or a callable that will be called in a
            new thread as soon as the gui window is created. This function is
            responsible for creating it's own loop running the simulation steps,
            updating the data queue of the gui window with fresh data, and wait
            some millisecond before running a new simulation step. The format of
            this function is required to be the following:

            routine(exit_event, data_queue, sim_handle, waiting_time)
        
            where "exit_event" is an object of type threading.Event() and may be
            used to know if externally the user closed the window and at the same
            time it may be used to pause the execution with
            exit_event.wait(waiting_time). That method will pause execution for as
            many seconds as contained in the float "waiting_time" and will return True
            in case the user closed the window (False otherwise).
            The parameter "data_queue" is a queue used by the gui to update the
            game data and graphics in real time, and the routine should call its
            method "put" passing a tuple containing two objects: the first is the
            tuple of the game state (a 5-objects tuple containing: environment, 
            pitch, settings, tuple of team1 player_info, tuple of team2
            player_info)). 
            The parameter "sim_handle" is the handle of the simulation.
            The parameter "waiting_time" is the same parameter that has been set as
            parameter to "play_routine()"

waiting_time\: is a float that will be passed to your callable routine (the second
            parameter) as a suggestion about how much time (in seconds) to wait
            between simulation steps (a good value is the default 0.09). Your
            routine may ignore this value.

example of a function to be used as second parameter "routine":

def example_routine(exit_event, data_queue, sim_handle, waiting_time):
    not_ended = True
    while (not exit_event.wait(timeout=waiting_time) ) and not_ended:
        #play game here
        not_ended= robosoc2d.simulator_step_if_playing(sim_handle)
        data_queue.put( (robosoc2d.simulator_get_game_state(sim_handle),
                        robosoc2d.simulator_get_state_string(sim_handle)) )

Functions
---------
### play_whole_game (sim_handle, title="Very Simplified 2D Robotic Soccer Simulator", waiting_time=0.09, width=840, height=540,                    team1color="blue", team2color="red", border1color = "black", border2color = "black", direction1color="white", direction2color="black", ball_color="yellow", pitch_color="green", lines_color="white", goal_color="gray")
    
This function plays a whole game and shows it in realtime inside a Tkinter
window, creating internally the GuiPlayer object in an automatic way, hence
saving the user from creating a GuiPlayer object manually.

Parameters:

sim_handle\: this is the only mandatory parameter and it should contain
            an integer that is the handle of the simulation.

title: is a string containing the tile of the window

waiting_time\: is a float that will be passed to your callable routine (the second
            parameter) as a suggestion about how much time (in seconds) to wait
            between simulation steps (a good value is the default 0.09). Your
            routine may ignore this value.

width\: the width of the window

height\: the height of the window

team1color\: tkinter compatible color for team1

team2color\: tkinter compatible color for team2

border1color\: tkinter compatible color for the outline of team1
    
border2color\: tkinter compatible color for the outline of team2

direction1color\: tkinter compatible color used for the first team to draw
                    a line inside the player that indicates player's direction

direction2color\: tkinter compatible color used for the second team to draw
                    a line inside the player that indicates player's direction

ball_color\: tkinter compatible color for the ball

pitch_color\: tkinter compatible color for the pitch

lines_color\: tkinter compatible color for the lines
                 
goal_color\: tkinter compatible color for the goal


### play_routine (sim_handle, routine, title="Very Simplified 2D Robotic Soccer Simulator", waiting_time=0.09, width=840, height=540,               team1color="blue", team2color="red", border1color = "black", border2color = "black", direction1color="white", direction2color="black", ball_color="yellow", pitch_color="green", lines_color="white", goal_color="gray")

This function creates a gui window with Tkinter inside which it is possible to
show the game in realtime. You have to provide a function or callable (the
second parameter "routine") that will provide for the inner loop.
If you don't want to write your own function but you still want to show the
game in a windows, you may call "play_whole_game()" instead of "play_routine()".
Only the first two parameters are mandatory.

Parameters\:

sim_handle\: it should contain an integer that is the handle of the simulation. 

routine\: it should contain a function or a callable that will be called in a
            new thread as soon as the gui window is created. This function is
            responsible for creating it's own loop running the simulation steps,
            updating the data queue of the gui window with fresh data, and wait
            some millisecond before running a new simulation step. The format of
            this function is required to be the following:

            routine(exit_event, data_queue, sim_handle, waiting_time)
            
            where "exit_event" is an object of type threading.Event() and may be
            used to know if externally the user closed the window and at the same
            time it may be used to pause the execution with
            exit_event.wait(waiting_time). That method will pause execution for as
            many seconds as contained in the float "waiting_time" and will return True
            in case the user closed the window (False otherwise).
            The parameter "data_queue" is a queue used by the gui to update the
            game data and graphics in real time, and the routine should call its
            method "put" passing a tuple containing two objects: the first is the
            tuple of the game state (a 5-objects tuple containing: environment, 
            pitch, settings, tuple of team1 player_info, tuple of team2
            player_info)). 
            The parameter "sim_handle" is the handle of the simulation.
            The parameter "waiting_time" is the same parameter that has been set as
            parameter to "play_routine()"

title\: is a string containing the tile of the window

waiting_time\: is a float that will be passed to your callable routine (the second
            parameter) as a suggestion about how much time (in seconds) to wait
            between simulation steps (a good value is the default 0.09). Your
            routine may ignore this value.

width\: the width of the window

height\: the height of the window

team1color\: tkinter compatible color for team1

team2color\: tkinter compatible color for team2

direction1color\: tkinter compatible color used for the first team to draw
                    a line inside the player that indicates player's direction

direction2color\: tkinter compatible color used for the second team to draw
                    a line inside the player that indicates player's direction

ball_color\: tkinter compatible color for the ball

pitch_color\: tkinter compatible color for the pitch

lines_color\: tkinter compatible color for the lines
                 
goal_color\: tkinter compatible color for the goal

example of a function to be used as second parameter "routine":

def example_routine(exit_event, data_queue, sim_handle, waiting_time):
    not_ended = True
    while (not exit_event.wait(timeout=waiting_time) ) and not_ended:
        #play game here
        not_ended= robosoc2d.simulator_step_if_playing(sim_handle)
        data_queue.put( (robosoc2d.simulator_get_game_state(sim_handle),
                            robosoc2d.simulator_get_state_string(sim_handle)) )


### get_version()

It returns a string containing the version of the library.

It uses the format: "major.minor.revision".

Examples:
---------

### Example 1 A (with functions)
```python
import robosoc2d
import robosoc2dgui.gui

sim_handle = robosoc2d.build_simpleplayer_simulator([], 4, [], 4)
robosoc2dgui.gui.play_whole_game(sim_handle)
```
### Example 1 B (with class)
```python
import robosoc2d
import robosoc2dgui.gui

sim_handle = robosoc2d.build_simpleplayer_simulator([], 4, [], 4)
gui = robosoc2dgui.gui.GuiPlayer(sim_handle)
gui.play_whole_game()
```
### Example 2 (with routine)
```python
import robosoc2d
import robosoc2dgui.gui

def my_routine(exit_event, data_queue, sim_handle, waiting_time):
    not_ended = True
    while (not exit_event.wait(timeout=waiting_time) ) and not_ended:
        not_ended= robosoc2d.simulator_step_if_playing(sim_handle)
        data_queue.put( (robosoc2d.simulator_get_game_state(sim_handle),
                        robosoc2d.simulator_get_state_string(sim_handle)) )

sim_handle = robosoc2d.build_simpleplayer_simulator([], 4, [], 4)
robosoc2dgui.gui.play_routine(sim_handle, my_routine)
```