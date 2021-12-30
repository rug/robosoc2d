# (c) 2021 Ruggero Rossi
############################################################################
# robosoc2d GUI
############################################################################
# to install (be sure to have robosoc2d installed previously and to have a 
# p_ython version with Tkinter built-in):
#  p_ython3 setup.p_y install
#
# to uninstall:
#  pip uninstall robosoc2dgui
"""
a GUI library to visualize robosoc2d simulations inside windows
"""

import sys
try:
    import tkinter
except ImportError:
    sys.exit("Tkinter not found. Please install Tkinter to use this package, or use a p_ython version with Tkinter built in.")

import math 
import tkinter as tk
from tkinter import font as tkfont
import threading
import queue
import robosoc2d

_GUIVERSION="1.0.0"
def get_version():
    """
    It returns a string containing the version of the library.
    It uses the format: "major.minor.revision".
    """
    return _GUIVERSION

# a subclass of Canvas for dealing with resizing of windows
#from https://stackoverflow.com/questions/22835289/how-to-get-tkinter-canvas-to-dynamically-resize-to-window-width
class _ResizingCanvas(tk.Canvas):
    def __init__(self, gui_player, parent, **kwargs):
        self.gui = gui_player
        tk.Canvas.__init__(self,parent,**kwargs)
        self.bind("<Configure>", self._on_resize)
        self.height = self.gui.original_pitch_canvas_height
        self.width = self.gui.original_pitch_canvas_width

    def _on_resize(self, event):
        self.height = event.height
        self.width = event.width
        # resize the canvas 
        self.config(width=self.width, height=self.height)
        self.gui.pitch_canvas_height=self.height
        self.gui.pitch_canvas_width=self.width
        self.gui._calc_ratios()
        self.gui._resize_pitch()
        self.gui._redraw_players()

#used to run a complete game in a thread, while main program is busy in the Tkinter GUI loop
class _PlayThread(threading.Thread):
    def __init__(self, exit_event, data_queue, sim_handle, routine, waiting_time):
        self.exit_event = exit_event
        self.data_queue = data_queue
        self.sim_handle = sim_handle
        self.routine = routine
        self.waiting_time = waiting_time
        threading.Thread.__init__(self)
        self.daemon = True  #this to set thread die when spawner ends

    def run(self):
        if self.routine is not None:
            self.routine(self.exit_event, self.data_queue, self.sim_handle, self.waiting_time)
        else:
            not_ended = True
            while (not self.exit_event.wait(timeout=self.waiting_time) ) and not_ended:
                #play game here
                not_ended= robosoc2d.simulator_step_if_playing(self.sim_handle)
                self.data_queue.put( (robosoc2d.simulator_get_game_state(self.sim_handle),robosoc2d.simulator_get_state_string(self.sim_handle)) )

class GuiPlayer():
    """creates a Tkinter gui window to show a robosoc2d simulation in real time.
    """
    def __init__(self, sim_handle,
                 title="Very Simplified 2D Robotic Soccer Simulator", width=840,
                 height=540, team1color="blue", team2color="red", 
                 border1color = "black", border2color = "black",
                 direction1color="white", direction2color="black",
                 ball_color="yellow", pitch_color="green", lines_color="white",
                 goal_color="gray"):
        """        
        sim_handle: this is the only mandatory parameter and it should contain
                    an integer that is the handle of the simulation.
        
        title: is a string containing the tile of the window

        width: the width of the window
        
        height: the height of the window
        
        team1color: tkinter compatible color for team1
        
        team2color: tkinter compatible color for team2

        border1color: tkinter compatible color for the outline of team1
    
        border2color: tkinter compatible color for the outline of team2
        
        direction1color: tkinter compatible color used for the first team to draw
                         a line inside the player that indicates player's direction
        
        direction2color: tkinter compatible color used for the second team to draw
                         a line inside the player that indicates player's direction
        
        ball_color: tkinter compatible color for the ball
        """
        self.team1color = team1color
        self.team2color = team2color
        self.border1color = border1color
        self.border2color = border2color
        self.direction1color = direction1color
        self.direction2color = direction2color
        self.original_ball_color = ball_color
        self.original_pitch_color = pitch_color
        self.original_lines_color = lines_color
        self.original_goal_color = goal_color

        self.data_queue = queue.Queue()
        self.sim_handle = sim_handle
        self._update_data()
        self.players_radius = self.settings.player_radius
        self.ball_radius = self.settings.ball_radius
        #self.pitch_width=self.settings.pitch_length #unused
        #self.pitch_height=self.settings.pitch_width
        self.original_pitch_canvas_width = width
        self.original_pitch_canvas_height = height
        self.pitch_canvas_width = self.original_pitch_canvas_width
        self.pitch_canvas_height = self.original_pitch_canvas_height
        self._calc_ratios()
        self.distance_numbers = 1.6 # multiplier that multiplies the players projected radius to use as distance of shirt numbers from players

        self._set_colors_colored()
        self.window = tk.Tk()
        self.window.title(title)

        self.top_frame = tk.Frame(self.window)
        self.top_frame.pack(side = "top")
        self.middle_frame1 = tk.Frame(self.window)
        self.middle_frame1.pack(side = "top")
        self.middle_frame2 = tk.Frame(self.window)
        self.middle_frame2.pack(side = "top")
        self.bottom_frame = tk.Frame(self.window)
        self.bottom_frame.pack(side = "bottom", fill="both", expand=True)

        self.bool_black_andwhite =  tk.BooleanVar() 
        self.chk_blackandwhite= tk.Checkbutton(self.middle_frame1, text="greyscale", variable=self.bool_black_andwhite, command=self._switch_colors)
        self.chk_blackandwhite.pack(side = "left")
        self.bool_show_numbers =  tk.BooleanVar()
        self.bool_show_numbers.set(True)
        self.chk_show_numbers= tk.Checkbutton(self.middle_frame1, text="shirt numbers", variable=self.bool_show_numbers, command=self._view_shirts)
        self.chk_show_numbers.pack(side = "left")

        self.label_info = tk.Label(self.middle_frame2, text = "...")
        self.label_info.pack(side = "left")
        
        self.canvas = _ResizingCanvas(self, self.bottom_frame, width = self.pitch_canvas_width, height = self.pitch_canvas_height, bg="lightgrey", highlightthickness=0)
        self.canvas.pack(side = "top", fill=tk.BOTH, expand=tk.YES)

        # creating a root menu to insert all the sub menus
        self.root_menu = tk.Menu(self.window)
        self.window.config(menu = self.root_menu)
        # creating sub menus in the root menu
        self.file_menu = tk.Menu(self.root_menu) # it intializes a new sub menu in the root menu
        self.root_menu.add_cascade(label = "File", menu = self.file_menu) # it creates the name of the sub menu
        self.file_menu.add_separator() # it adds a line
        self.file_menu.add_command(label = "Exit", command = self.window.destroy)

        #let's go
        self._paint_pitch()
        self._create_players()

    def play_routine(self, routine, waiting_time=0.09):
        """
        It calls a user-defined function or callable as soon as the gui window is
        created. It may be useful to create custom inner loops for the simulation,
        controlling what to do and how to use data between a simulation step and the
        other. If you don't have this necessity, you may want to use the
        play_whole_game() method instead.
        
        routine: it should contain a function or a callable that will be called in a
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
        
        waiting_time: is a float that will be passed to your callable routine (the second
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
        """
        self.exit_event=threading.Event()
        self.play_thread = _PlayThread(self.exit_event, self.data_queue, self.sim_handle, routine, waiting_time)
        self.play_thread.start()
        self.last_after=0
        self._consumer()
        self.window.mainloop()  #blocking until you close the window
        if self.last_after!=0:
            self.window.after_cancel(self.last_after)
        if self.play_thread is not None: #be sure to close a thread that's playing the game
            self.exit_event.set()
            self.play_thread.join()

    def play_whole_game(self, waiting_time=0.09):
        """
        Plays a whole game and shows it in realtime inside a Tkinter window. 
        
        Parameter:
        waiting_time: is a float that will be passed to your callable routine (the second
                 parameter) as a suggestion about how much time (in seconds) to wait
                 between simulation steps (a good value is the default 0.09). Your
                 routine may ignore this value.
        """
        self.play_routine(None, waiting_time)

    def _calc_ratios(self):
        self.center_width =self.pitch_canvas_width/2
        self.center_height =self.pitch_canvas_height/2
        self.x_ratio = self.pitch_canvas_width/(self.pitch.border_right-self.pitch.border_left)
        self.y_ratio = self.pitch_canvas_height/(self.pitch.border_up-self.pitch.border_down)

        #external field
        self.border_up = self._y2ci(self.pitch.border_up)
        self.border_down = self._y2ci(self.pitch.border_down)
        self.border_left = self._x2c(self.pitch.border_left)
        self.border_right = self._x2c(self.pitch.border_right)

        #pitch limits
        self.left_x = self._x2c(self.pitch.x2)
        self.top_y = self._y2ci(self.pitch.y1)
        self.right_x = self._x2c(self.pitch.x1)
        self.bottom_y = self._y2ci(self.pitch.y2)
        
        #central limits
        self.left_center_circle = self._x2c(-self.settings.center_radius)
        self.right_center_circle = self._x2c(self.settings.center_radius)
        self.up_center_circle = self._y2ci(self.settings.center_radius)
        self.down_center_circle = self._y2ci(-self.settings.center_radius)

        #areas
        self.are_left=self._x2c(self.pitch.area_lx)
        self.area_right=self._x2c(self.pitch.area_rx)
        self.area_top=self._y2ci(self.pitch.area_uy)
        self.area_bottom=self._y2ci(self.pitch.area_dy)

        #midfield
        self.midfield_x=self._x2c(0.0)

        #goals
        self.left_goal=self._x2c(self.pitch.x_goal2)
        self.right_goal=self._x2c(self.pitch.x_goal1)
        self.top_goal=self._y2ci(self.pitch.y_goal1 + self.settings.pole_radius)
        self.bottom_goal=self._y2ci(self.pitch.y_goal2 - self.settings.pole_radius)

    #converts the coords of the game pitch to canvas
    def _x2c(self, x):
        x2=x*self.x_ratio+self.center_width
        return x2

    def _y2c(self, y):
        y2=-y*self.y_ratio+self.center_height
        return y2

    #doesn't invert sign, useful for simmetric stuff and tkinter calls that wants fro top-left to bottom-right
    def _y2ci(self, y):
        y2=y*self.y_ratio+self.center_height
        return y2

    def _update_data(self):
        sim_state = robosoc2d.simulator_get_game_state(self.sim_handle)
        self.env = sim_state[0]
        self.pitch = sim_state[1]
        self.settings = sim_state[2]
        self.teams = [sim_state[3], sim_state[4]]
        self.state_string = robosoc2d.simulator_get_state_string(self.sim_handle)
    
    def _update_game_data(self):
        sim_state = robosoc2d.simulator_get_game_state(self.sim_handle)
        try:
            sim_state = self.data_queue.get(block=False)
        except queue.Empty:
            pass
        else:
            self.env = sim_state[0][0]
            self.teams = [sim_state[0][3], sim_state[0][4]]
            self.state_string = sim_state[1]

    def _update_gui(self):
        self._update_game_data()
        self._redraw_players()
        self._update_info()

    def _consumer(self):
        self._update_gui()
        self.last_after = self.window.after(1, lambda: self._consumer())
        #self.window.after(1, self._consumer )
        
    def _set_colors_colored(self):
        self.ball_color = self.original_ball_color
        self.colors = [self.team1color, self.team2color]
        self.border_colors = [self.border1color, self.border2color]
        self.directionlines_colors=[self.direction1color, self.direction2color]
        self.pitch_color = self.original_pitch_color
        self.lines_color = self.original_lines_color
        self.goal_color = self.original_goal_color

    def _set_colors_blackandwhite(self):
        self.ball_color="white"
        self.colors=["black", "white"]
        self.border_colors = ["black", "black"]
        self.directionlines_colors=["white", "black"]
        self.pitch_color="lightgrey"
        self.lines_color="black"
        self.goal_color="gray"

    def _paint_pitch(self):
        self.canvas_pitch = []
        self.canvas_pitch.append(self.canvas.create_rectangle(self.border_left, self.border_up, self.border_right+1, self.border_down+1, outline=self.pitch_color, fill=self.pitch_color) ) # pitch
        self.canvas_pitch.append(self.canvas.create_rectangle(self.left_goal, self.top_goal, self.left_x+1, self.bottom_goal+1, outline=self.lines_color, fill=self.goal_color) ) # left goal
        self.canvas_pitch.append(self.canvas.create_rectangle(self.right_x, self.top_goal, self.right_goal+1, self.bottom_goal+1, outline=self.lines_color, fill=self.goal_color) ) # right goal
        self.canvas_pitch.append(self.canvas.create_rectangle(self.left_x, self.top_y, self.right_x+1, self.bottom_y+1, outline=self.lines_color, fill="") ) # external lines
        self.canvas_pitch.append(self.canvas.create_oval(self.left_center_circle, self.up_center_circle, self.right_center_circle+1, self.down_center_circle+1, outline=self.lines_color, fill="") ) #midfield  circle
        self.canvas_pitch.append(self.canvas.create_rectangle(self.left_x, self.area_top, self.are_left+1, self.area_bottom+1, outline=self.lines_color, fill="") ) # penalty area left
        self.canvas_pitch.append(self.canvas.create_rectangle(self.area_right, self.area_top, self.right_x+1, self.area_bottom+1, outline=self.lines_color, fill="") ) # penalty area right
        self.canvas_pitch.append(self.canvas.create_line(self.midfield_x,  self.top_y, self.midfield_x, self.bottom_y+1, fill=self.lines_color) )   # midfield line
        
    def _resize_pitch(self):
        self.canvas.coords(self.canvas_pitch[0], self.border_left, self.border_up, self.border_right+1, self.border_down+1)  # pitch
        self.canvas.coords(self.canvas_pitch[1], self.left_goal, self.top_goal, self.left_x+1, self.bottom_goal+1)  # left goal
        self.canvas.coords(self.canvas_pitch[2], self.right_x, self.top_goal, self.right_goal+1, self.bottom_goal+1)  # right goal
        self.canvas.coords(self.canvas_pitch[3], self.left_x, self.top_y, self.right_x+1, self.bottom_y+1)  # external lines
        self.canvas.coords(self.canvas_pitch[4], self.left_center_circle, self.up_center_circle, self.right_center_circle+1, self.down_center_circle+1)  #midfield  circle
        self.canvas.coords(self.canvas_pitch[5], self.left_x, self.area_top, self.are_left+1, self.area_bottom+1)  # left area
        self.canvas.coords(self.canvas_pitch[6], self.area_right, self.area_top, self.right_x+1, self.area_bottom+1)  # right area
        self.canvas.coords(self.canvas_pitch[7], self.midfield_x,  self.top_y, self.midfield_x, self.bottom_y+1)  # midfield line
        
    def _create_players(self):
        font1 = tkfont.Font(size=int(self.players_radius*self.x_ratio))

        shownumbers='hidden'
        if self.bool_show_numbers.get():
            shownumbers='normal'
        
        self.circles=[[],[]]
        self.lines=[[],[]]
        self.numbers=[[],[]]
        for t in range(2):
            for i in range(len(self.teams[t])):
                lx=self._x2c(-self.players_radius)
                dy=self._y2c(-self.players_radius)
                rx=self._x2c(self.players_radius)
                uy=self._y2c(self.players_radius)
                self.circles[t].append(self.canvas.create_oval(lx, uy, rx+1, dy+1, outline=self.border_colors[t], fill=self.colors[t]) )
                p_x=self._x2c(self.teams[t][i].x)
                p_y=self._y2c(self.teams[t][i].y)
                face_x=self._x2c(self.teams[t][i].x+ math.cos(self.teams[t][i].direction)*self.players_radius )
                face_y=self._y2c(self.teams[t][i].y+ math.sin(self.teams[t][i].direction)*self.players_radius )
                self.lines[t].append(self.canvas.create_line(p_x, p_y, face_x+1, face_y+1, fill=self.directionlines_colors[t]) )
                self.numbers[t].append(self.canvas.create_text(self._x2c(0.0), self._y2c(0.0), text=str(i), anchor=tk.CENTER, state=shownumbers, font=font1) )
                
        blx=self._x2c(-self.ball_radius)
        bdy=self._y2c(-self.ball_radius)
        brx=self._x2c(self.ball_radius)
        buy=self._y2c(self.ball_radius)
        self.ball_circle= self.canvas.create_oval(blx, buy, brx+1, bdy+1, outline="black", fill=self.ball_color)

    def _redraw_players(self):
        font1 = tkfont.Font(size=int(self.players_radius*self.x_ratio))        
        for t in range(2):
            for i in range(len(self.teams[t])):
                lx=self._x2c(self.teams[t][i].x-self.players_radius)
                dy=self._y2c(self.teams[t][i].y-self.players_radius)
                rx=self._x2c(self.teams[t][i].x+self.players_radius)
                uy=self._y2c(self.teams[t][i].y+self.players_radius)
                self.canvas.coords(self.circles[t][i], lx, uy, rx+1, dy+1) 
                p_x=self._x2c(self.teams[t][i].x)
                p_y=self._y2c(self.teams[t][i].y)
                face_x=self._x2c(self.teams[t][i].x+ math.cos(self.teams[t][i].direction)*self.players_radius )
                face_y=self._y2c(self.teams[t][i].y+ math.sin(self.teams[t][i].direction)*self.players_radius )
                self.canvas.coords(self.lines[t][i], p_x, p_y, face_x+1, face_y+1)
                self.canvas.coords(self.numbers[t][i], self._x2c(self.teams[t][i].x-self.players_radius*self.distance_numbers), self._y2c(self.teams[t][i].y-self.players_radius*self.distance_numbers))
                self.canvas.itemconfig(self.numbers[t][i], font=font1) 

        blx=self._x2c(self.env.ball_x-self.ball_radius)
        bdy=self._y2c(self.env.ball_y-self.ball_radius)
        brx=self._x2c(self.env.ball_x+self.ball_radius)
        buy=self._y2c(self.env.ball_y+self.ball_radius)
        self.canvas.coords(self.ball_circle, blx, buy, brx+1, bdy+1)

    def _switch_colors(self):
        if self.bool_black_andwhite.get():
            self._set_colors_blackandwhite()
        else:
            self._set_colors_colored()

        for t in range(2):
            for i in range(len(self.teams[t])):
                self.canvas.itemconfig(self.circles[t][i], outline=self.border_colors[t], fill=self.colors[t])
                self.canvas.itemconfig(self.lines[t][i], fill=self.directionlines_colors[t])
        self.canvas.itemconfig(self.ball_circle, fill=self.ball_color)

        self.canvas.itemconfig(self.canvas_pitch[0], outline=self.pitch_color, fill=self.pitch_color)  # pitch
        self.canvas.itemconfig(self.canvas_pitch[1], outline=self.lines_color, fill=self.goal_color)  # left goal
        self.canvas.itemconfig(self.canvas_pitch[2], outline=self.lines_color, fill=self.goal_color)  # right goal
        self.canvas.itemconfig(self.canvas_pitch[3], outline=self.lines_color, fill="")  # external lines
        self.canvas.itemconfig(self.canvas_pitch[4], outline=self.lines_color, fill="")  #midfield  circle
        self.canvas.itemconfig(self.canvas_pitch[5], outline=self.lines_color, fill="")  # left area
        self.canvas.itemconfig(self.canvas_pitch[6], outline=self.lines_color, fill="")  # right area
        self.canvas.itemconfig(self.canvas_pitch[7], fill=self.lines_color)    # midfield line

    def _update_info(self):
        self.label_info['text']=self.state_string

    def _view_shirts(self):
        for t in range(2):
            for i in range(len(self.teams[t])):
                if self.bool_show_numbers.get():
                    self.canvas.itemconfigure(self.numbers[t][i], state='normal')
                else:
                    self.canvas.itemconfigure(self.numbers[t][i], state='hidden')


def play_whole_game(sim_handle, title="Very Simplified 2D Robotic Soccer Simulator", waiting_time=0.09, width=840, height=540,
                    team1color="blue", team2color="red", border1color = "black", border2color = "black",
                    direction1color="white", direction2color="black", ball_color="yellow",
                    pitch_color="green", lines_color="white", goal_color="gray"):
    """
    Plays a whole game and shows it in realtime inside a Tkinter window. 
    
    This function saves the user from creating a GuiPlayer object manually.
    Parameters:

    sim_handle: this is the only mandatory parameter and it should contain
                an integer that is the handle of the simulation.
    
    title: is a string containing the tile of the window
    
    waiting_time: is a float that will be passed to your callable routine (the second
             parameter) as a suggestion about how much time (in seconds) to wait
             between simulation steps (a good value is the default 0.09). Your
             routine may ignore this value.
    
    width: the width of the window
    
    height: the height of the window
    
    team1color: tkinter compatible color for team1
    
    team2color: tkinter compatible color for team2

    border1color: tkinter compatible color for the outline of team1
    
    border2color: tkinter compatible color for the outline of team2
    
    direction1color: tkinter compatible color used for the first team to draw
                     a line inside the player that indicates player's direction
    
    direction2color: tkinter compatible color used for the second team to draw
                     a line inside the player that indicates player's direction
    
    ball_color: tkinter compatible color for the ball
    """
    gui = GuiPlayer(sim_handle, title, width, height, team1color, team2color, border1color, border2color,
                    direction1color, direction2color, ball_color, pitch_color, lines_color, goal_color)
    gui.play_whole_game(waiting_time)


def play_routine(sim_handle, routine,
                 title="Very Simplified 2D Robotic Soccer Simulator",
                 waiting_time=0.09, width=840, height=540,
                 team1color="blue", team2color="red",
                 border1color = "black", border2color = "black",
                 direction1color="white", direction2color="black",
                 ball_color="yellow", pitch_color="green",
                 lines_color="white", goal_color="gray"):
    """
    Creates a gui window with Tkinter inside which it is possible to show the game
    in realtime. You have to provide a function or callable (the second parameter
    "routine") that will provide for the inner loop.
    If you don't want to write your own function but you still want to show the
    game in a windows, you may call "play_whole_game()" instead of "play_routine()".
    Only the first two parameters are mandatory.

    Parameters:
    
    sim_handle: it should contain an integer that is the handle of the simulation. 
    
    routine: it should contain a function or a callable that will be called in a
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
    
    title: is a string containing the tile of the window
    
    waiting_time: is a float that will be passed to your callable routine (the second
             parameter) as a suggestion about how much time (in seconds) to wait
             between simulation steps (a good value is the default 0.09). Your
             routine may ignore this value.
    
    width: the width of the window
    
    height: the height of the window
    
    team1color: tkinter compatible color for team1
    
    team2color: tkinter compatible color for team2

    border1color: tkinter compatible color for the outline of team1
    
    border2color: tkinter compatible color for the outline of team2
    
    direction1color: tkinter compatible color used for the first team to draw
                     a line inside the player that indicates player's direction
    
    direction2color: tkinter compatible color used for the second team to draw
                     a line inside the player that indicates player's direction
    
    ball_color: tkinter compatible color for the ball

    example of a function to be used as second parameter "routine":

    def example_routine(exit_event, data_queue, sim_handle, waiting_time):
        not_ended = True
        while (not exit_event.wait(timeout=waiting_time) ) and not_ended:
            #play game here
            not_ended= robosoc2d.simulator_step_if_playing(sim_handle)
            data_queue.put( (robosoc2d.simulator_get_game_state(sim_handle),
                             robosoc2d.simulator_get_state_string(sim_handle)) )
    """
    gui = GuiPlayer(sim_handle, title, width, height, team1color, team2color, border1color, border2color, direction1color, direction2color,
                    ball_color, pitch_color, lines_color, goal_color)
    gui.play_routine(routine, waiting_time)


