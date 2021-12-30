robosoc2dplot
=============
It's a package to plot robosoc2d simulations with matplotlib.

It contains only one module: robosoc2dplot.plot .

INSTALLATION from pip    
---------------------
Just run:
```
pip install robosoc2dplot
```

INSTALLATION from source code 
------------

open the terminal or shell and enter:

python3 setup.py install

instead, to uninstall it:

pip uninstall robosoc2dplot

robosoc2dplot.plot
------------------
It's the only module of the package. It contains 7 functions\:

draw() to actually draw the simulation state

show() to show up the figure previously generate with draw()

get_version() to get the version of the library

play_whole_game_in_notebook () to play a whole game in a Jupyter notebook

play_steps_in_notebook () to advance and display the simulation step by step inside a Jupyter notebook

play_whole_game_in_notebook_inline () to play a whole game in a Jupyter notebook if there is not the magic function "%matplotlib notebook"

play_steps_in_notebook_inline () to advance and display the simulation step by step inside a Jupyter notebook if there is not the magic function "%matplotlib notebook"

### on Jupyter and notebooks:

if you want to just display a single picture, at the beginning of the notebook you have to write the "magic function"\:
```python
%matplotlib inline
```

if you want to display the game during each steps, at the beginning of the notebook you have to write the "magic function"\:
```python
%matplotlib notebook
```

On Jupyter notebooks there is no need to call "show()".

Let's see the two functions in detail.

### draw (sim_handle, team1color="blue", team2color="red", direction1color="white", direction2color="black", ball_color="yellow", pitch_color="g", lines_color="w", goal_color="grey", draw_numbers=True, axes=None, fontdict=None):

It plots players and ball inside the pitch in the position and direction
in which they currently are in the simulation, using matplotlib.
To use it inside Jupyter-like notebooks it is necessary to use one of the following "magic function"\:
```python
%matplotlib inline # for single image
%matplotlib notebook # for interactive updating (not existing on Google Colab)
```

Parameters (only the first one is mandatory)\:

sim_handle\: it should contain an integer that is the handle of the simulation

team1color\: matplotlib compatible color for first team

team2color\: matplotlib compatible color for second team

direction1color\: matplotlib compatible color used for the first team to draw
                    a line inside the player that indicates player's direction

direction2color\: matplotlib compatible color used for the second team to draw
                    a line inside the player that indicates player's direction

ball_color\: matplotlib compatible color for the ball

pitch_color\: matplotlib compatible color for the pitch

lines_color\: matplotlib compatible color for the lines

goal_color\: matplotlib compatible color for the goals

draw_numbers\: draw players' shirt numbers (0 is for goalkeeper)

fontdict\: a matplotlib compatible dictionary to override the default text
              properties. If fontdict is None, the defaults are determined
              by rcParams.

x_size_inches\: width of the figure in inches

y_size_inches\: height of the figure in inches

axes\: a matplotlib axes object where the pitch will be drawn. If omitted, a new
      figure with new axes will be created

figure\: a matplot compatible figure. This has to be provided only if the
         correspondent axes object has been provided and at the same time
         parameters x_size_inches and y_size_inches have been provided



Returns\: (fig, ax) a tuple containing the matplot figure if newly created
         (otherwise None) and the axes of the plot

After calling this function, to visualize the picture it is necessary to call robosoc2dplot.plot.show() . 

(It is not necessary only on Jupyter notebooks where previously "%matplotlib inline" or "%matplotlib notebook" has been called)
    
### show ()

It shows the plot previously generated with "draw()".

It is the same as calling matplotlib.pyplot.show() .

### play_whole_game_in_notebook (sim_handle, waiting_time=0.05, team1color, team2color, direction1color, direction2color, ball_color, pitch_color="g", lines_color="w", goal_color="grey", draw_numbers, fontdict,  x_size_inches, y_size_inches):
    
It plays and displays a whole game inside a Jupyter or other notebook cell.

This function is created on purpose for notebooks like Jupyter and its behaviour is undefined in other contexts (it will most probably hang the execution without visualizing any picture).

To have the interactive updating of the image, in the notebook it is necessary to write the "magic function"\:

```python
%matplotlib notebook
```

The magic function above is not working in some platform such as Google Colab, so you can't use robosoc2dplot.plot.play_whole_game_in_notebook() in it. Anyway in Google Colab you can use the similar function play_whole_game_in_notebook_inline()

Parameters (only the first one is mandatory)\:

sim_handle\: it should contain an integer that is the handle of the simulation
    
waiting_time\: a float expressing how much time (in seconds) to wait between a simulation step and the next

team1color\: matplotlib compatible color for first team

team2color\: matplotlib compatible color for second team

direction1color\: matplotlib compatible color used for the first team to draw
                    a line inside the player that indicates player's direction

direction2color\: matplotlib compatible color used for the second team to draw
                    a line inside the player that indicates player's direction

ball_color\: matplotlib compatible color for the ball

pitch_color\: matplotlib compatible color for the pitch

lines_color\: matplotlib compatible color for the lines

goal_color\: matplotlib compatible color for the goals

draw_numbers\: draw players' shirt numbers (0 is for goalkeeper)

fontdict\: a matplotlib compatible dictionary to override the default text
              properties. If fontdict is None, the defaults are determined
              by rcParams.

x_size_inches\: width of the figure in inches

y_size_inches\: height of the figure in inches

### play_steps_in_notebook(sim_handle,  waiting_time=0.01, team1color="blue", team2color="red", direction1color="white", direction2color="black",  ball_color="yellow", pitch_color="g", lines_color="w", goal_color="grey", draw_numbers=True, fontdict=None, x_size_inches=None, y_size_inches=None)

It advances and displays the simulation step by step inside a Jupyter or
other notebook cell

It is necessary to call the Jupyter magic function "%matplotlib notebook" before
calling this function otherwise it will not work.

Parameters (only the first one is mandatory)\:

sim_handle\: it should contain an integer that is the handle of the simulation

waiting_time\: a float expressing how much time (in seconds) to wait between a 
            simulation step and the next, when running the whole game

team1color\: matplotlib compatible color for first team

team2color\: matplotlib compatible color for second team

direction1color\: matplotlib compatible color used for the first team to draw
                    a line inside the player that indicates player's direction

direction2color\: matplotlib compatible color used for the second team to draw
                    a line inside the player that indicates player's direction

ball_color\: matplotlib compatible color for the ball

pitch_color\: matplotlib compatible color for the pitch

lines_color\: matplotlib compatible color for the lines

goal_color\: matplotlib compatible color for the goals

draw_numbers\: draw players' shirt numbers (0 is for goalkeeper)

fontdict\: a matplotlib compatible dictionary to override the default text
            properties. If fontdict is None, the defaults are determined
            by rcParams.

x_size_inches\: width of the figure in inches

y_size_inches\: height of the figure in inches

This function is created on purpose for notebooks like Jupyter and its
behaviour is undefined in other contexts (it will most probably not work
because it can't import IPython.display.clear_output).

To have the interactive updating of the image, in the notebook it is
necessary to previously call the magic function "%matplotlib notebook".
Some platforms such as Google Colab don't have the "%matplotlib notebook"
magic function, so unfortunately in those cases you can't use
robosoc2dplot.plot.play_steps_in_notebook() in it. Anyway in Google
Colab you can use the similar play_steps_in_notebook_inline()

### play_whole_game_in_notebook_inline (sim_handle, waiting_time=0.05, team1color, team2color, direction1color, direction2color, ball_color, pitch_color="g", lines_color="w", goal_color="grey", draw_numbers, fontdict,  x_size_inches, y_size_inches):
    
It plays and displays a whole game inside a Jupyter or other notebook cell.


This function is created on purpose for notebooks like Jupyter and its behaviour is undefined in other contexts (it will most probably hang the execution without visualizing any picture).

It is similar to play_whole_game_in_notebook() but it doesn't require "%matplotlib notebook", instead it requires\:

```python
%matplotlib inline
```

This function is working also on Google Colab.

Parameters (only the first one is mandatory)\:

sim_handle\: it should contain an integer that is the handle of the simulation
    
waiting_time\: a float expressing how much time (in seconds) to wait between a simulation step and the next

team1color\: matplotlib compatible color for first team

team2color\: matplotlib compatible color for second team

direction1color\: matplotlib compatible color used for the first team to draw
                    a line inside the player that indicates player's direction

direction2color\: matplotlib compatible color used for the second team to draw
                    a line inside the player that indicates player's direction

ball_color\: matplotlib compatible color for the ball

pitch_color\: matplotlib compatible color for the pitch

lines_color\: matplotlib compatible color for the lines

goal_color\: matplotlib compatible color for the goals

draw_numbers\: draw players' shirt numbers (0 is for goalkeeper)

fontdict\: a matplotlib compatible dictionary to override the default text
              properties. If fontdict is None, the defaults are determined
              by rcParams.

x_size_inches\: width of the figure in inches

y_size_inches\: height of the figure in inches

### play_steps_in_notebook_inline(sim_handle,  waiting_time=0.01, team1color="blue", team2color="red", direction1color="white", direction2color="black", ball_color="yellow", pitch_color="g", lines_color="w", goal_color="grey", draw_numbers=True, fontdict=None, x_size_inches=None, y_size_inches=None)
    
It advances and displays the simulation step by step inside a Jupyter or
other notebook cell.

It is necessary to call the Jupyter magic function "%matplotlib inline" before
calling this function otherwise it will not work.

Parameters (only the first one is mandatory)\:

sim_handle\: it should contain an integer that is the handle of the simulation

waiting_time\: a float expressing how much time (in seconds) to wait between a 
            simulation step and the next, when running the whole game

team1color\: matplotlib compatible color for first team

team2color\: matplotlib compatible color for second team

direction1color\: matplotlib compatible color used for the first team to draw
                    a line inside the player that indicates player's direction

direction2color\: matplotlib compatible color used for the second team to draw
                    a line inside the player that indicates player's direction

ball_color\: matplotlib compatible color for the ball

pitch_color\: matplotlib compatible color for the pitch

lines_color\: matplotlib compatible color for the lines

goal_color\: matplotlib compatible color for the goals

draw_numbers\: draw players' shirt numbers (0 is for goalkeeper)

fontdict\: a matplotlib compatible dictionary to override the default text
            properties. If fontdict is None, the defaults are determined
            by rcParams.

x_size_inches\: width of the figure in inches

y_size_inches\: height of the figure in inches

This function is created on purpose for notebooks like Jupyter and its
behaviour is undefined in other contexts (it will most probably not work
because it can't import IPython.display.clear_output).


### get_version()

It returns a string containing the version of the library.

It uses the format\: "major.minor.revision".

Example 1:
----------
```python
import robosoc2d
import robosoc2dplot.plot

sim_handle = robosoc2d.build_simpleplayer_simulator([], 4, [], 4)
robosoc2d.simulator_step_if_playing(sim_handle)
fig, ax = robosoc2dplot.plot.draw(sim_handle)
robosoc2dplot.plot.show()
```
Example 2, same as Example 1 but on Jupyter notebook:
-----------------------------------------------------
```python
%matplotlib inline
import robosoc2d
import robosoc2dplot.plot

sim_handle = robosoc2d.build_simpleplayer_simulator([], 4, [], 4)
robosoc2d.simulator_step_if_playing(sim_handle)
fig, ax = robosoc2dplot.plot.draw(sim_handle)
# robosoc2dplot.plot.show() # this line is commented out because it is not necessary for single-images on Jupyter-like notebooks
```

Example 3, on Jupyter notebook using "%matplotlib notebook" (not supported on Colab):
-------------------------------------------------------------------------------------
```python
# copy the line below twice if you previously used "%matplotlib inline": for some reason it will often not work otherwise !
%matplotlib notebook 
import robosoc2d
import robosoc2dplot.plot as r2plot

sim_handle = robosoc2d.build_simpleplayer_simulator([], 4, [], 4)
r2plot.play_whole_game_in_notebook(sim_handle, 0.05)
```
Example 4, on Jupyter notebook using "%matplotlib notebook" (not supported on Colab):
-------------------------------------------------------------------------------------
this is equivalent to Example 3, it shows how to run a complete game without
using "play_whole_game_in_notebook()", in case you need to process something
in the inner loop.

```python
%matplotlib notebook
import robosoc2d
import robosoc2dplot.plot as r2plot
import time

sim_handle = robosoc2d.build_simpleplayer_simulator([], 4, [], 4)
fig, ax = r2plot.draw(sim_handle)
while(robosoc2d.simulator_step_if_playing(sim_handle)):
    ax.clear()
    r2plot.draw(sim_handle, axes=ax)
    fig.canvas.draw()
    time.sleep(0.05)
```

Example 5, on Jupyter-like notebook, step by step update clicking a button, using "%matplotlib notebook"  (not supported on Colab):
--------------------------------------------------------------------------------------------------------------------------------------------
```python
%matplotlib notebook
import robosoc2d
import robosoc2dplot.plot as r2plot

sim_handle = robosoc2d.build_simpleplayer_simulator([], 4, [], 4)
r2plot.play_steps_in_notebook(sim_handle)
```

Example 6, on Jupyter notebook if there is not "%matplotlib notebook" (works on Colab):
---------------------------------------------------------------------------------------
```python
%matplotlib inline
import robosoc2d
import robosoc2dplot.plot as r2plot

sim_handle = robosoc2d.build_simpleplayer_simulator([], 4, [], 4)
r2plot.play_whole_game_in_notebook_inline(sim_handle, 0.05)
```
Example 7, on Jupyter notebook if there is not "%matplotlib notebook" (works on Colab):
---------------------------------------------------------------------------------------
this is equivalent to Example 6, it shows how to run a complete game without
using "play_whole_game_in_notebook_inline()", in case you need to process something
in the inner loop.
```python
%matplotlib inline
import robosoc2d
import robosoc2dplot.plot as r2plot
import time
from IPython.display import clear_output

sim_handle = robosoc2d.build_simpleplayer_simulator([], 4, [], 4)
while(robosoc2d.simulator_step_if_playing(sim_handle)):
    fig, ax = r2plot.draw(sim_handle)
    r2plot.show()
    time.sleep(0.05)
    clear_output(wait=True)
```

Example 8, on Jupyter-like notebook, step by step update clicking a button if there is not "%matplotlib notebook" (works on Colab):
-----------------------------------------------------------------------------------------------------------------------------------------
```python
%matplotlib inline
import robosoc2d
import robosoc2dplot.plot as r2plot

sim_handle = robosoc2d.build_simpleplayer_simulator([], 4, [], 4)
r2plot.play_steps_in_notebook_inline(sim_handle)
```