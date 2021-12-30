robosoc2dplotty
===============
It's a package to plot robosoc2d simulations with plotly.

It contains only one module: robosoc2dplot.plotty .

IMPORTANT !
It works with Jupyter Notebook 6.0.1
For some reason it does NOT work rightfully with Jupyter Notebook 6.2.0

INSTALLATION
------------

open the terminal or shell and enter:

python3 setup.py install

instead, to uninstall it:

pip uninstall robosoc2dplotty

robosoc2dplot.plotty
--------------------
It's the only module of the package.
It contains 5 functions \:

draw () to actually draw the simulation state

update () to update the picture of the simulation state with current state

show ()  to show up the figure previously generate with draw()

get_version() to get the version of the library

play_whole_game_in_notebook () to play a whole game in a Jupyter notebook

play_steps_in_notebook () to advance and display the simulation step by step inside a Jupyter notebook

### draw(sim_handle, team1color="blue", team2color="red", border1color = "black", border2color = "black", direction1color="white", direction2color="black", ball_color="yellow", pitch_color="green", lines_color="white", goal_color="grey", draw_numbers=True, width=800, height=600)
    
It plots players and ball inside the pitch in the position and direction in which they currently are in the simulation, using plotly. 

Parameters (only the first one is mandatory)\:

sim_handle\: it should contain an integer that is the handle of the simulation

team1color\: plotly compatible color for first team

team2color\: plotly compatible color for second team

border1color\: plotly compatible color for first team border color

border2color\: plotly compatible color for second team border color

direction1color\: plotly compatible color used for the first team to draw
                a line inside the player that indicates player's direction

direction2color\: plotly compatible color used for the second team to draw
                a line inside the player that indicates player's direction

ball_color\: plotly compatible color for the ball

draw_numbers\: draw players' shirt numbers (0 is for goalkeeper)

width\: picture width, in pixels

height\: picture height, in pixels

Returns\: a plotly FigureWidget object

After calling this function, if you are just visualizing one picture it is necessary to call robosoc2dplotty.plotty.show(figure) to make it appear.
If instead you are visualizing the game in a dynamic way in a Jupyter-like notebook, showing new updates inside the same picture, you don't have to
call robosoc2dplotty.plotty.show() but the Jupyter internal function display(figure) . In this case, to subsequently update the picture after
one or more simulation steps, call robosoc2dplotty.plotty.update()

### update(sim_handle, figure, draw_numbers=True, players_shapes_start_offset=None, annotations_start_offset=0)

It updates players and ball with current position and direction,
using plotly. 

Parameters (only the first two are mandatory)\n:

sim_handle\: it should contain an integer that is the handle of the simulation

figure\: a plotly FigureWidget of the pitch that has been previously
        created with robosoc2dplotty.plotty.draw()

draw_numbers\: draw players' shirt numbers (0 is for goalkeeper)

players_shapes_start_offset\: if you modified the draw() function, 
                                or if you used a different function to
                                draw the pitch, this should indicate the index of
                                the first shape of the first player

annotations_start_offset\: if you modified the draw() function, 
                                or if you used a different function to
                                draw the pitch, this should indicate the index of
                                the first annotation of the first player to be
                                used as shirt number


### show(figure)

It shows the figure plot previously generated with robosoc2dplotty.plotty.draw(), that is the only parameter.

It has to be used only to produce a single static figure. If instead you want to visualize the game in a dynamic way in a Jupyter-like
notebook, showing new updates inside the same picture, you don't have to call robosoc2dplotty.plotty.show() but the Jupyter internal function
display(figure) . In this case, to subsequently update the picture after one or more simulation steps, call robosoc2dplotty.plotty.update()


### get_version()

It returns a string containing the version of the library.
It uses the format: "major.minor.revision".


### play_whole_game_in_notebook(sim_handle, waiting_time=0.01, team1color="blue", team2color="red", border1color = "black", border2color = "black",    direction1color="white", direction2color="black", ball_color="yellow", pitch_color="green", lines_color="white", goal_color="grey", draw_numbers=True, width=800, height=600)

It plays a whole game in a Jupyter-like notebook

This function works only inside a Jupyter-like notebook (otherwise it will continue creating new figures or have other unpredictable behaviours). As of 02 March 2021 it doesn't seem to run correctly on Google Colab (in Google Colab you can use instead the similar function
robosoc2dplot.plot.play_whole_game_in_notebook_inline() )

Parameters (only the first is mandatory)\:

sim_handle\: it should contain an integer that is the handle of the simulation

waiting_time\: a float expressing how much time (in seconds) to wait between a 
            simulation step and the next

team1color\: plotly compatible color for first team

team2color\: plotly compatible color for second team

border1color\: plotly compatible color for first team border color

border2color\: plotly compatible color for second team border color

direction1color\: plotly compatible color used for the first team to draw
                a line inside the player that indicates player's direction

direction2color\: plotly compatible color used for the second team to draw
                a line inside the player that indicates player's direction

ball_color\: plotly compatible color for the ball

draw_numbers\: draw players' shirt numbers (0 is for goalkeeper)

width\: picture width, in pixels

height\: picture height, in pixels


Returns\: a plotly FigureWidget object

### play_steps_in_notebook (sim_handle,  waiting_time=0.01, team1color="blue", team2color="red", border1color = "black", border2color = "black",  direction1color="white", direction2color="black", ball_color="yellow", pitch_color="green", lines_color="white", goal_color="grey", draw_numbers=True, width=800, height=600)
    
It advances and displays the simulation step by step inside a Jupyter or
other notebook cell.

Parameters (only the first one is mandatory)\:

sim_handle\: it should contain an integer that is the handle of the simulation

waiting_time\: a float expressing how much time (in seconds) to wait between a 
            simulation step and the next, when running the whole game

team1color\: plotly compatible color for first team

team2color\: plotly compatible color for second team

border1color\: plotly compatible color for first team border color

border2color\: plotly compatible color for second team border color

direction1color\: plotly compatible color used for the first team to draw
                a line inside the player that indicates player's direction

direction2color\: plotly compatible color used for the second team to draw
                a line inside the player that indicates player's direction

ball_color\: plotly compatible color for the ball

draw_numbers\: draw players' shirt numbers (0 is for goalkeeper)

width\: picture width, in pixels

height\: picture height, in pixels

This function is created on purpose for notebooks like Jupyter and its
behaviour is undefined in other contexts.


Example 1:
----------
```python
# single picture
import robosoc2d
import robosoc2dplotty.plotty as r2plotty
import time

sim_handle = robosoc2d.build_simpleplayer_simulator([], 4, [], 4)
robosoc2d.simulator_step_if_playing(sim_handle)
fig = r2plotty.draw(sim_handle, draw_numbers=True)
r2plotty.show(fig)
```

Example 2, on Jupyter-like notebook, whole game (CAREFUL, THIS MAY HANG YOUR NOTEBOOK !):
-----------------------------------------------------------------------------------------
```python
import robosoc2d
import robosoc2dplotty.plotty as r2plotty

sim_handle = robosoc2d.build_simpleplayer_simulator([], 4, [], 4)
r2plotty.play_whole_game_in_notebook(sim_handle)
```

Example 3 (equivalent to the previous cell but without using "play_whole_game_in_notebook()", in case you need to process something in the inner loop):
----------------------------------------------------------------------------------------------------------------------------------------------------------
```python
import robosoc2d
import robosoc2dplotty.plotty as r2plotty
import time

sim_handle = robosoc2d.build_simpleplayer_simulator([], 4, [], 4)
fig = r2plotty.draw(sim_handle)
display(fig)
while(robosoc2d.simulator_step_if_playing(sim_handle)):
    r2plotty.update(sim_handle, fig)
    time.sleep(0.01)
```

Example 4, on Jupyter-like notebook, step by step update clicking a button:
---------------------------------------------------------------------------
```python
import robosoc2d
import robosoc2dplotty.plotty as r2plotty

sim_handle = robosoc2d.build_simpleplayer_simulator([], 4, [], 4)
r2plotty.play_steps_in_notebook(sim_handle)
```