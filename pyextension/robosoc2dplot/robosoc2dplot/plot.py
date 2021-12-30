# (c) 2021 Ruggero Rossi
#############################################################################
# robosoc2d Matplotlib draw
#############################################################################
# to install (be sure to have matplotlib and robosoc2d installed previously):
#  python3 setup.py install
#
# to uninstall:
#  pip uninstall robosoc2dplot
#
#############################################################################
# on Jupyther and notebooks:
# if you want to just display a single picture, at the beginning of the notebook write:
#   %matplotlib inline
#
# if you want to display the game during each steps, at the beginning of the notebook write:
#   %matplotlib notebook
"""
a library to draw a robosoc2d simulation using matplotlib
"""

import math
import robosoc2d
import matplotlib.pyplot as plt
import time

_PLOTVERSION="1.0.0"
def get_version():
    """
    It returns a string containing the version of the library.
    It uses the format: "major.minor.revision".
    """
    return _PLOTVERSION

def draw(sim_handle, team1color="blue", team2color="red",
         direction1color="white", direction2color="black",
         ball_color="yellow",
         pitch_color="g", lines_color="w", goal_color="grey",
         draw_numbers=True, fontdict=None,
         x_size_inches=None, y_size_inches=None, axes=None, figure=None):
    """
    It plots players and ball inside the pitch in the position and direction
    in which they currently are in the simulation, using matplotlib. 
    
    Parameters (only the first one is mandatory):

    sim_handle: it should contain an integer that is the handle of the simulation
    
    team1color: matplotlib compatible color for first team
    
    team2color: matplotlib compatible color for second team
    
    direction1color: matplotlib compatible color used for the first team to draw
                     a line inside the player that indicates player's direction
    
    direction2color: matplotlib compatible color used for the second team to draw
                     a line inside the player that indicates player's direction
    
    ball_color: matplotlib compatible color for the ball

    pitch_color: matplotlib compatible color for the pitch
    
    lines_color: matplotlib compatible color for the lines
    
    goal_color: matplotlib compatible color for the goals

    draw_numbers: draw players' shirt numbers (0 is for goalkeeper)

    fontdict: a matplotlib compatible dictionary to override the default text
              properties. If fontdict is None, the defaults are determined
              by rcParams.

    x_size_inches: width of the figure in inches

    y_size_inches: height of the figure in inches

    axes: a matplotlib axes object where the pitch will be drawn. If omitted,
          a new figure with new axes will be created

    figure: a matplot compatible figure. This has to be provided only if the
            correspondent axes object has been provided and at the same time
            parameters x_size_inches and y_size_inches have been provided


    Returns: (fig, ax) a tuple containing the matplot figure if newly created
             (otherwise None) and the axes of the plot

    After calling this function, to visualize the picture it is necessary to call
    robosoc2dplot.plot.show() .
    (It is not necessary only on Jupyter notebooks where previously
    "%matplotlib inline" or "%matplotlib notebook" has been called)
    """
    sim_state = robosoc2d.simulator_get_game_state(sim_handle)
    env = sim_state[0]
    pitch = sim_state[1]
    settings = sim_state[2]
    teams = [sim_state[3], sim_state[4]]
    state_string = robosoc2d.simulator_get_state_string(sim_handle)
    ax=axes

    colors=[team1color, team2color]
    directionlines_colors=[direction1color, direction2color]

    fig = figure
    if ax is None:
        fig, ax = plt.subplots()
    plt.xlim([pitch.border_left, pitch.border_right])
    plt.ylim([pitch.border_down, pitch.border_up])

    top_goal=pitch.y_goal1 + settings.pole_radius
    bottom_goal=pitch.y_goal2 - settings.pole_radius

    ax.add_patch(plt.Rectangle( (pitch.border_left, pitch.border_down), pitch.border_right-pitch.border_left, pitch.border_up-pitch.border_down, color=pitch_color, fill=True) ) # pitch
    ax.add_patch(plt.Rectangle( (pitch.x_goal2, bottom_goal), pitch.x2-pitch.x_goal2, top_goal-bottom_goal,  color=goal_color, fill=True) ) # left goal
    ax.add_patch(plt.Rectangle( (pitch.x_goal2, bottom_goal), pitch.x2-pitch.x_goal2, top_goal-bottom_goal, color=lines_color, fill=False) ) # left goal
    ax.add_patch(plt.Rectangle( (pitch.x1, bottom_goal), pitch.x_goal1-pitch.x1, top_goal-bottom_goal, color=goal_color, fill=True) ) # right goal
    ax.add_patch(plt.Rectangle( (pitch.x1, bottom_goal), pitch.x_goal1-pitch.x1, top_goal-bottom_goal, color=lines_color, fill=False) ) # right goal
    ax.add_patch(plt.Rectangle( (pitch.x2, pitch.y2), settings.pitch_length, settings.pitch_width, color=lines_color, fill=False) ) # external lines
    ax.add_patch(plt.Circle( (0.0, 0.0), settings.center_radius, color=lines_color, fill=False) ) #midfield  circle
    ax.add_patch(plt.Rectangle( (pitch.x2, pitch.area_dy), pitch.area_lx-pitch.x2, pitch.area_uy-pitch.area_dy, color=lines_color, fill=False) ) # penalty area left
    ax.add_patch(plt.Rectangle( (pitch.area_rx, pitch.area_dy), pitch.x1-pitch.area_rx, pitch.area_uy-pitch.area_dy, color=lines_color, fill=False) ) # penalty area right
    ax.add_patch(plt.Rectangle( (0.0,  pitch.y2), 0.0, settings.pitch_width, color=lines_color) )   # midfield line

    for t in range(2):
        for i in range(len(teams[t])):
            player=teams[t][i]
            ax.add_patch(plt.Circle( (player.x,player.y), settings.player_radius, color=colors[t], fill=True) ) 
            x=[player.x, player.x+settings.player_radius*math.cos(player.direction)]
            y=[player.y, player.y+settings.player_radius*math.sin(player.direction)]
            ax.plot(x, y, color=directionlines_colors[t])
            if draw_numbers:
                plt.text(player.x+settings.player_radius, player.y+settings.player_radius, str(i), fontdict)

    ax.add_patch(plt.Circle( (env.ball_x, env.ball_y), settings.ball_radius, color=ball_color, fill=True) ) 
    plt.xlabel(state_string)
    
    if (x_size_inches is not None) and (y_size_inches is not None) and (fig is not None):
        fig.set_size_inches(x_size_inches, y_size_inches)

    return  (fig, ax)


def show():
    """
    shows the plot previously generated with "draw()"
    """
    plt.show()


def play_whole_game_in_notebook(sim_handle, waiting_time=0.01,
         team1color="blue", team2color="red",
         direction1color="white", direction2color="black",
         ball_color="yellow",
         pitch_color="g", lines_color="w", goal_color="grey",
         draw_numbers=True, fontdict=None,
         x_size_inches=None, y_size_inches=None):
    """
    It plays and displays a whole game inside a Jupyter or other notebook cell

    It is necessary to call the Jupyter magic function "%matplotlib notebook" before
    calling this function otherwise it will not work.

    Parameters (only the first one is mandatory):

    sim_handle: it should contain an integer that is the handle of the simulation
    
    waiting_time: a float expressing how much time (in seconds) to wait between a 
              simulation step and the next

    team1color: matplotlib compatible color for first team
    
    team2color: matplotlib compatible color for second team
    
    direction1color: matplotlib compatible color used for the first team to draw
                     a line inside the player that indicates player's direction
    
    direction2color: matplotlib compatible color used for the second team to draw
                     a line inside the player that indicates player's direction
    
    ball_color: matplotlib compatible color for the ball

    pitch_color: matplotlib compatible color for the pitch
    
    lines_color: matplotlib compatible color for the lines
    
    goal_color: matplotlib compatible color for the goals

    draw_numbers: draw players' shirt numbers (0 is for goalkeeper)

    fontdict: a matplotlib compatible dictionary to override the default text
              properties. If fontdict is None, the defaults are determined
              by rcParams.

    x_size_inches: width of the figure in inches

    y_size_inches: height of the figure in inches
    
    This function is created on purpose for notebooks like Jupyter and its
    behaviour is undefined in other contexts (it will most probably hang the
    execution without visualizing any picture).

    To have the interactive updating of the image, in the notebook it is
    necessary to previously call the magic function "%matplotlib notebook".
    Some platforms such as Google Colab don't have the "%matplotlib notebook"
    magic function, so unfortunately in those cases you can't use
    robosoc2dplot.plot.play_whole_game_in_notebook() in it. Anyway in Google
    Colab you can use the similar function play_whole_game_in_notebook_inline()

    """
    #plt.ion()
    fig, ax = draw(sim_handle, team1color, team2color, direction1color, direction2color,
         ball_color, pitch_color, lines_color, goal_color,
         draw_numbers, fontdict, x_size_inches, y_size_inches)
    while(robosoc2d.simulator_step_if_playing(sim_handle)):
        ax.clear()
        draw(sim_handle, team1color, team2color, direction1color, direction2color,
             ball_color, pitch_color, lines_color, goal_color,
             draw_numbers, fontdict, axes=ax, figure=fig)
        fig.canvas.draw()
        time.sleep(waiting_time)
    return  (fig, ax)


def play_steps_in_notebook(sim_handle,  waiting_time=0.01, team1color="blue", team2color="red",
         direction1color="white", direction2color="black",
         ball_color="yellow",
         pitch_color="g", lines_color="w", goal_color="grey",
         draw_numbers=True, fontdict=None,
         x_size_inches=None, y_size_inches=None):
    """
    It advances and displays the simulation step by step inside a Jupyter or
    other notebook cell

    It is necessary to call the Jupyter magic function "%matplotlib notebook" before
    calling this function otherwise it will not work.

    Parameters (only the first one is mandatory):

    sim_handle: it should contain an integer that is the handle of the simulation

    waiting_time: a float expressing how much time (in seconds) to wait between a 
              simulation step and the next, when running the whole game

    team1color: matplotlib compatible color for first team
    
    team2color: matplotlib compatible color for second team
    
    direction1color: matplotlib compatible color used for the first team to draw
                     a line inside the player that indicates player's direction
    
    direction2color: matplotlib compatible color used for the second team to draw
                     a line inside the player that indicates player's direction
    
    ball_color: matplotlib compatible color for the ball

    pitch_color: matplotlib compatible color for the pitch
    
    lines_color: matplotlib compatible color for the lines
    
    goal_color: matplotlib compatible color for the goals

    draw_numbers: draw players' shirt numbers (0 is for goalkeeper)

    fontdict: a matplotlib compatible dictionary to override the default text
              properties. If fontdict is None, the defaults are determined
              by rcParams.

    x_size_inches: width of the figure in inches

    y_size_inches: height of the figure in inches
    
    This function is created on purpose for notebooks like Jupyter and its
    behaviour is undefined in other contexts (it will most probably not work
    because it can't import IPython.display.clear_output).

    To have the interactive updating of the image, in the notebook it is
    necessary to previously call the magic function "%matplotlib notebook".
    Some platforms such as Google Colab don't have the "%matplotlib notebook"
    magic function, so unfortunately in those cases you can't use
    robosoc2dplot.plot.play_steps_in_notebook() in it. Anyway in Google
    Colab you can use the similar play_steps_in_notebook_inline()

    """
    #plt.ion()
    from IPython.display import display
    import ipywidgets as widgets

    slider_steps = widgets.IntSlider(value=1, min=1, max=100, step=1, description='steps:')
    button_steps = widgets.Button(description='next steps')
    button_whole = widgets.Button(description='until the end')

    display(slider_steps)
    display(button_steps)    
    display(button_whole)  

    fig, ax = draw(sim_handle, team1color, team2color, direction1color, direction2color,
                    ball_color, pitch_color, lines_color, goal_color,
                    draw_numbers, fontdict, x_size_inches, y_size_inches)
    def _show_steps():  
        ax.clear()
        draw(sim_handle, team1color, team2color, direction1color, direction2color,
             ball_color, pitch_color, lines_color, goal_color,
             draw_numbers, fontdict, axes=ax, figure=fig)
        fig.canvas.draw() 
        return

    # the button callback functions
    def _on_button_steps_click(b):
        for i in range(slider_steps.value):
            if not robosoc2d.simulator_step_if_playing(sim_handle):
                break
        _show_steps()

    def _on_button_whole_click(b):
        finished= False
        while(not finished):
            for i in range(slider_steps.value):
                if not robosoc2d.simulator_step_if_playing(sim_handle):
                    finished = True
                    break
            _show_steps()
            time.sleep(waiting_time)

    button_steps.on_click(_on_button_steps_click)
    button_whole.on_click(_on_button_whole_click)
    _show_steps()

    return 

def play_whole_game_in_notebook_inline(sim_handle, waiting_time=0.01, team1color="blue", team2color="red",
         direction1color="white", direction2color="black",
         ball_color="yellow",
         pitch_color="g", lines_color="w", goal_color="grey",
         draw_numbers=True, fontdict=None,
         x_size_inches=None, y_size_inches=None):
    """
    It plays and displays a whole game inside a Jupyter or other notebook cell

    It is necessary to call the Jupyter magic function "%matplotlib inline" before
    calling this function otherwise it will not work.

    Parameters (only the first one is mandatory):

    sim_handle: it should contain an integer that is the handle of the simulation
    
    waiting_time: a float expressing how much time (in seconds) to wait between a 
              simulation step and the next

    team1color: matplotlib compatible color for first team
    
    team2color: matplotlib compatible color for second team
    
    direction1color: matplotlib compatible color used for the first team to draw
                     a line inside the player that indicates player's direction
    
    direction2color: matplotlib compatible color used for the second team to draw
                     a line inside the player that indicates player's direction
    
    ball_color: matplotlib compatible color for the ball

    pitch_color: matplotlib compatible color for the pitch
    
    lines_color: matplotlib compatible color for the lines
    
    goal_color: matplotlib compatible color for the goals

    draw_numbers: draw players' shirt numbers (0 is for goalkeeper)

    fontdict: a matplotlib compatible dictionary to override the default text
              properties. If fontdict is None, the defaults are determined
              by rcParams.

    x_size_inches: width of the figure in inches

    y_size_inches: height of the figure in inches
    
    This function is created on purpose for notebooks like Jupyter and its
    behaviour is undefined in other contexts (it will most probably not work
    because it can't import IPython.display.clear_output).

    """
    #plt.ioff()
    from IPython.display import clear_output

    while(robosoc2d.simulator_step_if_playing(sim_handle)):
        fig, ax = draw(sim_handle, team1color, team2color, direction1color, direction2color,
                    ball_color, pitch_color, lines_color, goal_color,
                    draw_numbers, fontdict, x_size_inches, y_size_inches)
        show()
        time.sleep(waiting_time)
        clear_output(wait=True)
    return  (fig, ax)


def play_steps_in_notebook_inline(sim_handle,  waiting_time=0.01, team1color="blue", team2color="red",
         direction1color="white", direction2color="black", ball_color="yellow",
         pitch_color="g", lines_color="w", goal_color="grey",
         draw_numbers=True, fontdict=None,
         x_size_inches=None, y_size_inches=None):
    """
    It advances and displays the simulation step by step inside a Jupyter or
    other notebook cell.

    It is necessary to call the Jupyter magic function "%matplotlib inline" before
    calling this function otherwise it will not work.

    Parameters (only the first one is mandatory):

    sim_handle: it should contain an integer that is the handle of the simulation

    waiting_time: a float expressing how much time (in seconds) to wait between a 
              simulation step and the next, when running the whole game

    team1color: matplotlib compatible color for first team
    
    team2color: matplotlib compatible color for second team
    
    direction1color: matplotlib compatible color used for the first team to draw
                     a line inside the player that indicates player's direction
    
    direction2color: matplotlib compatible color used for the second team to draw
                     a line inside the player that indicates player's direction
    
    ball_color: matplotlib compatible color for the ball

    pitch_color: matplotlib compatible color for the pitch
    
    lines_color: matplotlib compatible color for the lines
    
    goal_color: matplotlib compatible color for the goals

    draw_numbers: draw players' shirt numbers (0 is for goalkeeper)

    fontdict: a matplotlib compatible dictionary to override the default text
              properties. If fontdict is None, the defaults are determined
              by rcParams.

    x_size_inches: width of the figure in inches

    y_size_inches: height of the figure in inches
    
    This function is created on purpose for notebooks like Jupyter and its
    behaviour is undefined in other contexts (it will most probably not work
    because it can't import IPython.display.clear_output).

    """
    #plt.ioff()
    from IPython.display import clear_output, display
    import ipywidgets as widgets

    slider_steps = widgets.IntSlider(value=1, min=1, max=100, step=1, description='steps:')
    button_steps = widgets.Button(description='next steps')
    button_whole = widgets.Button(description='until the end')

    def _show_steps():
        draw(sim_handle, team1color, team2color, direction1color, direction2color,
             ball_color, pitch_color, lines_color, goal_color,
             draw_numbers, fontdict, x_size_inches, y_size_inches)
        show()
        display(slider_steps)
        display(button_steps)    
        display(button_whole)    
        clear_output(wait=True)
        return

    # the button callback functions
    def _on_button_steps_click(b):
        for i in range(slider_steps.value):
            if not robosoc2d.simulator_step_if_playing(sim_handle):
                break
        _show_steps()

    def _on_button_whole_click(b):
        finished= False
        while(not finished):
            for i in range(slider_steps.value):
                if not robosoc2d.simulator_step_if_playing(sim_handle):
                    finished = True
                    break
            _show_steps()
            time.sleep(waiting_time)

    button_steps.on_click(_on_button_steps_click)
    button_whole.on_click(_on_button_whole_click)
    _show_steps()

    return 