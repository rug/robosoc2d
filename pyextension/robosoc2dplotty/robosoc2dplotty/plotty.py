# (c) 2021 Ruggero Rossi
#############################################################################
# robosoc2d plotly draw
#############################################################################
# to install (be sure to have plotly and robosoc2d installed previously):
#  python3 setup.py install
#
# to uninstall:
#  pip uninstall robosoc2dplotty
#
#############################################################################
# works with jupyter notebook 6.0.1
# does NOT work with jupyter notebook 6.2.0
"""
a library to draw a robosoc2d simulation using plotty
"""

import math
import robosoc2d
import plotly.graph_objects as go
import time
import copy

_PLOTTYVERSION="1.0.0"
def get_version():
    """
    It returns a string containing the version of the library.
    It uses the format: "major.minor.revision".
    """
    return _PLOTTYVERSION

players_shapes_start = 8 # but actually it's calculated during each draw()

def draw(sim_handle, team1color="blue", team2color="red",
         border1color = "black", border2color = "black",
         direction1color="white", direction2color="black",
         ball_color="yellow", 
         pitch_color="green", lines_color="white", goal_color="grey",
         draw_numbers=True, width=800, height=600):
    """
    It plots players and ball inside the pitch in the position and direction
    in which they currently are in the simulation, using plotly. 
    
    Parameters (only the first one is mandatory):

    sim_handle: it should contain an integer that is the handle of the simulation
    
    team1color: plotly compatible color for first team
    
    team2color: plotly compatible color for second team

    border1color: plotly compatible color for first team border color

    border2color: plotly compatible color for second team border color
    
    direction1color: plotly compatible color used for the first team to draw
                    a line inside the player that indicates player's direction
    
    direction2color: plotly compatible color used for the second team to draw
                    a line inside the player that indicates player's direction
    
    ball_color: plotly compatible color for the ball

    pitch_color: plotly compatible color for the pitch
    
    lines_color: plotly compatible color for the lines
    
    goal_color: plotly compatible color for the goals

    draw_numbers: draw players' shirt numbers (0 is for goalkeeper)

    width: picture width, in pixels

    height: picture height, in pixels

    Returns: a plotly FigureWidget object

    After calling this function, if you are just visualizing one picture it is
    necessary to call robosoc2dplotty.plotty.show(figure) to make it appear.
    If instead you are visualizing the game in a dynamic way in a Jupyter-like
    notebook, showing new updates inside the same picture, you don't have to
    call robosoc2dplotty.plotty.show() but the Jupyter internal function
    display(figure) . In this case, to subsequently update the picture after
    one or more simulation steps, call robosoc2dplotty.plotty.update()
    
    """
    global players_shapes_start
    sim_state = robosoc2d.simulator_get_game_state(sim_handle)
    env = sim_state[0]
    pitch = sim_state[1]
    settings = sim_state[2]
    teams = [sim_state[3], sim_state[4]]
    state_string = robosoc2d.simulator_get_state_string(sim_handle)

    fig = go.FigureWidget()
    #fig = go.Figure()  #with this you wouldn't have ipywidgets interactions
    fig.update_layout(width=width, height=height)

    colors=[team1color, team2color]
    border_colors=[border1color, border2color]
    directionlines_colors=[direction1color, direction2color]

    fig.update_xaxes(range=[pitch.border_left, pitch.border_right], zeroline=False)
    fig.update_yaxes(range=[pitch.border_down, pitch.border_up])

    top_goal=pitch.y_goal1 + settings.pole_radius
    bottom_goal=pitch.y_goal2 - settings.pole_radius

    _players_shapes_start = 0

    # whole green field
    fig.add_shape(type="rect", xref="x", yref="y", fillcolor=pitch_color,
                    x0=pitch.border_left, y0=pitch.border_down,
                    x1=pitch.border_right, y1=pitch.border_up)

    _players_shapes_start +=1

    # left goal 
    fig.add_shape(type="rect", xref="x", yref="y", fillcolor=goal_color,
                    x0=pitch.x_goal2, y0=bottom_goal,
                    x1=pitch.x2, y1=top_goal,
                    line_color=lines_color)
    
    _players_shapes_start +=1

    # right goal
    fig.add_shape(type="rect", xref="x", yref="y", fillcolor=goal_color,
                    x0=pitch.x1, y0=bottom_goal,
                    x1=pitch.x_goal1, y1=top_goal,
                    line_color=lines_color)

    _players_shapes_start +=1
    
    # external lines
    fig.add_shape(type="rect", xref="x", yref="y",
                    x0=pitch.x2, y0=pitch.y2,
                    x1=pitch.x1, y1=pitch.y1,
                    line_color=lines_color)

    _players_shapes_start +=1
    
    # center circle
    fig.add_shape(type="circle", xref="x", yref="y", 
                    x0=-settings.center_radius, y0=-settings.center_radius,
                    x1=+settings.center_radius, y1=+settings.center_radius,
                    line_color=lines_color)

    _players_shapes_start +=1
    
    # left area
    fig.add_shape(type="rect", xref="x", yref="y",
                    x0=pitch.x2, y0=pitch.area_dy,
                    x1=pitch.area_lx, y1=pitch.area_uy,
                    line_color=lines_color)

    _players_shapes_start +=1

    # right area
    fig.add_shape(type="rect", xref="x", yref="y",
                    x0=pitch.area_rx, y0=pitch.area_dy,
                    x1=pitch.x1, y1=pitch.area_uy,
                    line_color=lines_color)

    _players_shapes_start +=1
    
    # midfield line
    fig.add_shape(type="line", xref="x", yref="y",
                            x0=0.0, y0=pitch.y2,
                            x1=0.0, y1=pitch.y1,
                            line_color=lines_color)
    _players_shapes_start +=1
    
    for t in range(2):
        for i in range(len(teams[t])):
            player=teams[t][i]
            
            fig.add_shape(type="circle", xref="x", yref="y", fillcolor=colors[t],
                            x0=player.x -settings.player_radius, y0=player.y -settings.player_radius,
                            x1=player.x +settings.player_radius, y1=player.y +settings.player_radius,
                            line_color=border_colors[t])
            
            fig.add_shape(type="line", xref="x", yref="y", fillcolor=colors[t],
                            x0=player.x, y0=player.y,
                            x1=player.x+settings.player_radius*math.cos(player.direction), y1=player.y+settings.player_radius*math.sin(player.direction),
                            line_color=directionlines_colors[t], line_width=2)

            fig.add_annotation(dict(font=dict(color='black',size=15),
                                        x=player.x+settings.player_radius,
                                        y=player.y+settings.player_radius,
                                        showarrow=False,
                                        text=str(i),
                                        textangle=0,
                                        xanchor='left',
                                        yanchor='bottom',
                                        xref="x",
                                        yref="y",
                                        visible=draw_numbers))

    fig.add_shape(type="circle", xref="x", yref="y", fillcolor=ball_color,
                            x0=env.ball_x -settings.ball_radius, y0=env.ball_y -settings.ball_radius,
                            x1=env.ball_x +settings.ball_radius, y1=env.ball_y +settings.ball_radius,
                            line_color="black")

    fig.update_layout(title={"text":state_string})

    players_shapes_start=_players_shapes_start

    return  fig

def update(sim_handle, figure, draw_numbers=True, players_shapes_start_offset=None, annotations_start_offset=0):
    """
    It updates players and ball with current position and direction,
    using plotly. 
    
    Parameters (only the first two are mandatory):
    
    sim_handle: it should contain an integer that is the handle of the simulation

    figure: a plotly FigureWidget of the pitch that has been previously
            created with robosoc2dplotty.plotty.draw()
    
    draw_numbers: draw players' shirt numbers (0 is for goalkeeper)

    players_shapes_start_offset: if you modified the draw() function, 
                                 or if you used a different function to
                                 draw the pitch, this should indicate the index of
                                 the first shape of the first player
    
    annotations_start_offset: if you modified the draw() function, 
                                 or if you used a different function to
                                 draw the pitch, this should indicate the index of
                                 the first annotation of the first player to be
                                 used as shirt number
    
    """
    sim_state = robosoc2d.simulator_get_game_state(sim_handle)
    env = sim_state[0]
    pitch = sim_state[1]
    settings = sim_state[2]
    teams = [sim_state[3], sim_state[4]]
    state_string = robosoc2d.simulator_get_state_string(sim_handle)
    fig= figure
    if fig is None:
        return

    shapes_start = players_shapes_start
    if(players_shapes_start_offset is not None):
        shapes_start = players_shapes_start_offset

    s = shapes_start
    a = annotations_start_offset
    layout=copy.deepcopy(fig.layout)
    
    for t in range(2):
        for i in range(len(teams[t])):
            player=teams[t][i]
            
            layout.shapes[s]['x0']=player.x -settings.player_radius
            layout.shapes[s]['y0']=player.y -settings.player_radius
            layout.shapes[s]['x1']=player.x +settings.player_radius
            layout.shapes[s]['y1']=player.y +settings.player_radius
            s+=1
            
            layout.shapes[s]['x0']=player.x
            layout.shapes[s]['y0']=player.y
            layout.shapes[s]['x1']=player.x+settings.player_radius*math.cos(player.direction)
            layout.shapes[s]['y1']=player.y+settings.player_radius*math.sin(player.direction)
            s+=1
            
            layout.annotations[a]['x']=player.x+settings.player_radius
            layout.annotations[a]['y']=player.y+settings.player_radius
            layout.annotations[a]['visible']=draw_numbers
            a+=1
            
    layout.shapes[s]['x0']=env.ball_x -settings.ball_radius
    layout.shapes[s]['y0']=env.ball_y -settings.ball_radius
    layout.shapes[s]['x1']=env.ball_x +settings.ball_radius
    layout.shapes[s]['y1']=env.ball_y +settings.ball_radius

    layout['title']['text']=state_string

    fig.layout = layout

    return


def show(figure):
    """
    It shows the figure plot previously generated with
    robosoc2dplotty.plotty.draw(), that is the only parameter.

    It has to be used only to produce a single static figure.
    If instead you want to visualize the game in a dynamic way in a Jupyter-like
    notebook, showing new updates inside the same picture, you don't have to
    call robosoc2dplotty.plotty.show() but the Jupyter internal function
    display(figure) . In this case, to subsequently update the picture after
    one or more simulation steps, call robosoc2dplotty.plotty.update()
    """
    figure.show()


def play_whole_game_in_notebook(sim_handle, waiting_time=0.01,
         team1color="blue", team2color="red",
         border1color = "black", border2color = "black",
         direction1color="white", direction2color="black",
         ball_color="yellow",
         pitch_color="green", lines_color="white", goal_color="grey",
         draw_numbers=True, width=800, height=600):
    """
    It plays a whole game in a Jupyter-like notebook

    This function works only inside a Jupyter-like notebook (otherwise it will
    continue creating new figures or have other unpredictable behaviours).
    As of 02 March 2021 it doesn't seem to run correctly on Google Colab
    (in Google Colab you can use instead the similar function
    robosoc2dplot.plot.play_whole_game_in_notebook_inline() )

    Parameters (only the first is mandatory):
    
    sim_handle: it should contain an integer that is the handle of the simulation
    
    waiting_time: a float expressing how much time (in seconds) to wait between a 
              simulation step and the next
    
    team1color: plotly compatible color for first team
    
    team2color: plotly compatible color for second team

    border1color: plotly compatible color for first team border color

    border2color: plotly compatible color for second team border color
    
    direction1color: plotly compatible color used for the first team to draw
                    a line inside the player that indicates player's direction
    
    direction2color: plotly compatible color used for the second team to draw
                    a line inside the player that indicates player's direction
    
    ball_color: plotly compatible color for the ball

    pitch_color: plotly compatible color for the pitch
    
    lines_color: plotly compatible color for the lines
    
    goal_color: plotly compatible color for the goals

    draw_numbers: draw players' shirt numbers (0 is for goalkeeper)

    width: picture width, in pixels

    height: picture height, in pixels


    Returns: a plotly FigureWidget object

    """
    from IPython.display import display

    sim_handle = robosoc2d.build_simpleplayer_simulator([], 4, [], 4)
    fig = draw(sim_handle, team1color, team2color, border1color, border2color,
               direction1color, direction2color, ball_color,
               pitch_color, lines_color, goal_color,
               draw_numbers, width, height)
    display(fig)
    while(robosoc2d.simulator_step_if_playing(sim_handle)):
        update(sim_handle, fig, draw_numbers)
        time.sleep(waiting_time)

    return  fig


def play_steps_in_notebook(sim_handle,  waiting_time=0.01, team1color="blue", team2color="red",
        border1color = "black", border2color = "black",
        direction1color="white", direction2color="black",
        ball_color="yellow",
        pitch_color="green", lines_color="white", goal_color="grey",
        draw_numbers=True, width=800, height=600):
    """
    It advances and displays the simulation step by step inside a Jupyter or
    other notebook cell.

    Parameters (only the first one is mandatory):

    sim_handle: it should contain an integer that is the handle of the simulation

    waiting_time: a float expressing how much time (in seconds) to wait between a 
              simulation step and the next, when running the whole game

    team1color: plotly compatible color for first team
    
    team2color: plotly compatible color for second team

    border1color: plotly compatible color for first team border color

    border2color: plotly compatible color for second team border color
    
    direction1color: plotly compatible color used for the first team to draw
                    a line inside the player that indicates player's direction
    
    direction2color: plotly compatible color used for the second team to draw
                    a line inside the player that indicates player's direction
    
    ball_color: plotly compatible color for the ball

    pitch_color: plotly compatible color for the pitch
    
    lines_color: plotly compatible color for the lines
    
    goal_color: plotly compatible color for the goals

    draw_numbers: draw players' shirt numbers (0 is for goalkeeper)

    width: picture width, in pixels

    height: picture height, in pixels
    
    This function is created on purpose for notebooks like Jupyter and its
    behaviour is undefined in other contexts.

    """
    from IPython.display import display
    import ipywidgets as widgets

    slider_steps = widgets.IntSlider(value=1, min=1, max=100, step=1, description='steps:')
    button_steps = widgets.Button(description='next steps')
    button_whole = widgets.Button(description='until the end')

    display(slider_steps)
    display(button_steps)    
    display(button_whole)  

    fig = draw(sim_handle, team1color, team2color, border1color, border2color,
               direction1color, direction2color, ball_color,
               pitch_color, lines_color, goal_color, draw_numbers, width, height)
    display(fig)

    def _show_steps():
        update(sim_handle, fig, draw_numbers)
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