# (c) 2021 Ruggero Rossi
############################################################################
# robosoc2d states log player
############################################################################

import os
import math 
import tkinter as tk
from tkinter import font as tkfont
from tkinter import filedialog
import threading
import queue

import r2files

r2states= ['INACTIVE', 'READY', 'KICKOFF1', 'KICKOFF2', 'PLAY', 'STOPPED', 'GOALKICK1UP', 'GOALKICK1DOWN', 'GOALKICK2UP', 'GOALKICK2DOWN',
    'CORNER1UP', 'CORNER1DOWN', 'CORNER2UP', 'CORNER2DOWN', 'THROWIN1', 'THROWIN2', 'PAUSED', 'HALFTIME', 'GOAL1', 'GOAL2', 'ENDED']

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
    def __init__(self, start_tick, exit_event, data_queue, game, waiting_time):
        self.start_tick = start_tick
        self.exit_event = exit_event
        self.data_queue = data_queue
        self.game = game
        self.waiting_time = waiting_time
        threading.Thread.__init__(self)
        self.daemon = True  #this to set thread die when spawner ends

    def run(self):
        tick = self.start_tick
        while (not self.exit_event.wait(timeout=self.waiting_time) ) and (tick < len(self.game['ticks']) ):
            self.data_queue.put( tick )
            tick +=1


class LogPlayer():
    def __init__(self, 
                 title="Robosoc2d log player", width=840,
                 height=540, team1color="blue", team2color="red", 
                 border1color = "black", border2color = "black",
                 direction1color="white", direction2color="black",
                 ball_color="yellow", pitch_color="green", lines_color="white",
                 goal_color="gray"):

        self.play_thread = None
        self.exit_event= None
        self.last_after= 0
        self.circles=[[],[]]
        self.lines=[[],[]]
        self.numbers=[[],[]]
        self.pitch = []

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
        self.game = None
        self._set_initial_objects()
        self._calc_pitch()

        self.original_pitch_canvas_width = width
        self.original_pitch_canvas_height = height
        self.pitch_canvas_width = self.original_pitch_canvas_width
        self.pitch_canvas_height = self.original_pitch_canvas_height
        self._calc_ratios()
        self.distance_numbers = 1.6 # multiplier that multiplies the players projected radius to use as distance of shirt numbers from players

        self._set_colors_colored()
        self.window = tk.Tk()
        self.window.title(title)

        self.top_title_frame = tk.Frame(self.window)
        self.top_title_frame.pack(side = "top")
        self.top_frame = tk.Frame(self.window)
        self.top_frame.pack(side = "top")
        self.middle_frame1 = tk.Frame(self.window)
        self.middle_frame1.pack(side = "top")
        self.middle_frame2 = tk.Frame(self.window)
        self.middle_frame2.pack(side = "top")
        self.bottom_frame = tk.Frame(self.window)
        self.bottom_frame.pack(side = "bottom", fill="both", expand=True)
        self.label_filename = tk.Label(self.top_title_frame, text = "no game loaded")
        self.label_filename.pack(side = "top")
        self.btn_play = tk.Button(self.top_frame, text = "|>", fg = "green", command = self._play)
        self.btn_play.pack(side = "left")
        self.btn_stop = tk.Button(self.top_frame, text = "||", fg = "black", command = self._stop)
        self.btn_stop.pack(side = "left")
        self.btn_back = tk.Button(self.top_frame, text = "<<", fg = "black", command = self._back)
        self.btn_back.pack(side = "left")
        self.btn_fwd = tk.Button(self.top_frame, text = ">>", fg = "black", command = self._fwd)
        self.btn_fwd.pack(side = "left")
        self.tick_var = tk.IntVar()
        self.tick_var.set(0)
        self.tickslide= tk.Scale(self.top_frame, variable=self.tick_var , width = 8, length=400 , from_ = 0, to = 6000, orient=tk.HORIZONTAL, command = self._update_tick_slide)
        self.tickslide.pack(side = "left")
        self.tickbox = tk.Spinbox(self.top_frame, textvariable = self.tick_var ,  width = 6 , from_ = 0, to = 6000 , increment = 1, command = self._update_tick_spinbox)
        self.tickbox.pack(side = "left")
        self.bool_black_andwhite =  tk.BooleanVar() 
        self.chk_blackandwhite= tk.Checkbutton(self.middle_frame1, text="greyscale", variable=self.bool_black_andwhite, command=self._switch_colors)
        self.chk_blackandwhite.pack(side = "left")
        self.bool_show_numbers =  tk.BooleanVar() 
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
        self.file_menu.add_command(label = "Open game log", command = self._load_game)
        self.file_menu.add_separator() # it adds a line
        self.file_menu.add_command(label = "Exit", command = self.window.destroy)
        self.image_menu = tk.Menu(self.root_menu) # it intializes a new sub menu in the root menu
        self.root_menu.add_cascade(label = "Image", menu = self.image_menu) # it creates the name of the sub menu
        self.image_menu.add_command(label = "Save pitch as PostScript image", command = self._save_postscript)

        #let's go
        self._paint_pitch()
        self._create_players()

        self.window.mainloop()  #blocking until you close the window
        if self.last_after!=0:
            self.window.after_cancel(self.last_after)
        if self.play_thread is not None: #be sure to close a thread that's playing the game
            self.exit_event.set()
            self.play_thread.join()
    
    def _update_static_pitch(self):
        if self.play_thread is not None:
            return
        if self.game is None:
            return
        
        t = self.tick_var.get()
        self._update_current_tick(t)
        self._update_status_string(t)
        self._update_gui()

    def _update_current_tick(self, tick):
        tick_state=self.game['ticks'][tick]
        self.teams = tick_state['teams']
        self.ball_x  = tick_state['ball_x']
        self.ball_y  = tick_state['ball_y']

    def _update_status_string(self, tick):
        tick_state=self.game['ticks'][tick]
        team1_pos= " (left) " if tick< self.game['sett']['ticks_per_time'] else " (right) "
        team2_pos= " (right) " if tick< self.game['sett']['ticks_per_time'] else " (left) "
        score="]   "+self.game['team1_name']+team1_pos+": "+str(tick_state['score1'])+"    "+ self.game['team2_name']+team2_pos+": "+str(tick_state['score2'])
        self.state_string = "[t: "+str(tick)+score+"   "+r2states[tick_state['state']]

    def _update_tick_slide(self, value):
        self._update_static_pitch()

    def _update_tick_spinbox(self):
        self._update_static_pitch()

    def _play(self, waiting_time=0.0998):
        if self.game is None:
            return
        if self.play_thread is not None:
            return
        self.start_tick= self.tick_var.get()
        self.exit_event=threading.Event()
        self.data_queue = queue.Queue()
        self.play_thread = _PlayThread(self.start_tick, self.exit_event, self.data_queue, self.game, waiting_time)
        self.play_thread.start()
        self.last_after=0
        self._consumer()

    def _stop(self):
        if self.exit_event is not None:
            self.exit_event.set()
        if self.last_after!=0:
            self.window.after_cancel(self.last_after)
        self.play_thread = None

    def _back(self):
        self._stop()
        self.start_tick= self.tick_var.get() -1
        if self.start_tick <0:
            self.start_tick=0
        self.tick_var.set(self.start_tick)
        self._update_static_pitch()

    def _fwd(self):
        self._stop()
        if self.game is None:
            self.start_tick= self.tick_var.get() +1
            if self.start_tick > self.tickbox['to'] :
                self.start_tick= self.tickbox['to'] 
        else:
            self.start_tick= self.tick_var.get() +1
            if self.start_tick >= len(self.game['ticks']) :
                self.start_tick= len(self.game['ticks'])-1
        self.tick_var.set(self.start_tick)
        self._update_static_pitch()
    
    def _load_game(self):
        filename =  filedialog.askopenfilename(initialdir = "./",title = "Select game",filetypes = (("Robosoc2d states log","*.states.txt"),("all files","*.*")))
        if (filename is not None) and (len(filename) > 0):
            self._stop()
            self.game  = r2files.load_state_log (filename)
            self.label_filename.config(text = os.path.basename(filename))
            self._set_initial_objects()
            self._calc_pitch()
            self._calc_ratios()
            self._paint_pitch()
            self._create_players()
            self.tick_var.set(0)
            self.tickslide['to']=len(self.game['ticks'])-1
            self.tickbox['to']=len(self.game['ticks'])-1
        
    def _set_initial_objects(self):
        self.ball_x = 0.0
        self.ball_y = 0.0
        self.state_string = "ready"
        if self.game is None:
            return
        self.teams =[ [{'x':0, 'y':0, 'direction':0} for n in range(self.game['n_players'][i])] for i in range(2)]
        self.players_radius = self.game['sett']['player_radius']
        self.ball_radius = self.game['sett']['ball_radius']

    def _calc_pitch(self):
        if self.game is None:
            return
        #calc pitch depending on settings
        self.pitch_x1 = self.game['sett']['pitch_length']/2.0
        self.pitch_y1 = self.game['sett']['pitch_width']/2.0
        self.pitch_x2 = -self.pitch_x1
        self.pitch_y2 = -self.pitch_y1
        self.pitch_x_goal1 = self.pitch_x1 + self.game['sett']['net_length']
        self.pitch_x_goal2 = -self.pitch_x_goal1
        self.pitch_y_goal1 = self.game['sett']['goal_width']/2.0
        self.pitch_y_goal2 = -self.pitch_y_goal1
        self.pitch_border_right = self.pitch_x1 + self.game['sett']['out_pitch_limit']
        self.pitch_border_left = self.pitch_x2 - self.game['sett']['out_pitch_limit']
        self.pitch_border_up= self.pitch_y1 + self.game['sett']['out_pitch_limit']
        self.pitch_border_down = self.pitch_y2 - self.game['sett']['out_pitch_limit']
        area_length = (self.game['sett']['pitch_length']/105.0)*16.5
        area_width = (self.game['sett']['pitch_width']/68.0)*40.32
        self.pitch_area_lx = self.pitch_x2 + area_length
        self.pitch_area_rx = self.pitch_x1 - area_length
        self.pitch_area_uy = area_width/2.0
        self.pitch_area_dy = -area_width/2.0

    def _calc_ratios(self):
        if self.game is None:
            return
        self.center_width =self.pitch_canvas_width/2
        self.center_height =self.pitch_canvas_height/2
        self.x_ratio = self.pitch_canvas_width/(self.pitch_border_right-self.pitch_border_left)
        self.y_ratio = self.pitch_canvas_height/(self.pitch_border_up-self.pitch_border_down)

        #external field
        self.border_up = self._y2ci(self.pitch_border_up)
        self.border_down = self._y2ci(self.pitch_border_down)
        self.border_left = self._x2c(self.pitch_border_left)
        self.border_right = self._x2c(self.pitch_border_right)

        #pitch limits
        self.left_x = self._x2c(self.pitch_x2)
        self.top_y = self._y2ci(self.pitch_y1)
        self.right_x = self._x2c(self.pitch_x1)
        self.bottom_y = self._y2ci(self.pitch_y2)
        
        #central limits
        self.left_center_circle = self._x2c(-self.game['sett']['center_radius'])
        self.right_center_circle = self._x2c(self.game['sett']['center_radius'])
        self.up_center_circle = self._y2ci(self.game['sett']['center_radius'])
        self.down_center_circle = self._y2ci(-self.game['sett']['center_radius'])

        #areas
        self.area_left=self._x2c(self.pitch_area_lx)
        self.area_right=self._x2c(self.pitch_area_rx)
        self.area_top=self._y2ci(self.pitch_area_uy)
        self.area_bottom=self._y2ci(self.pitch_area_dy)

        #midfield
        self.midfield_x=self._x2c(0.0)

        #goals
        self.left_goal=self._x2c(self.pitch_x_goal2)
        self.right_goal=self._x2c(self.pitch_x_goal1)
        self.top_goal=self._y2ci(self.pitch_y_goal1 + self.game['sett']['pole_radius'])
        self.bottom_goal=self._y2ci(self.pitch_y_goal2 - self.game['sett']['pole_radius'])

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
    
    def _update_game_data(self):
        ended = False
        try:
            t = self.data_queue.get(block=False)
        except queue.Empty:
            pass
        else:
            self._update_current_tick(t)
            self.tick_var.set(t)
            if t== len(self.game['ticks'])-1 :
                ended = True
            self._update_status_string(t)
        return ended

    def _update_gui(self):
        self._redraw_players()
        self._update_info()

    def _consumer(self):
        ended = self._update_game_data()
        self._update_gui()
        if not ended:
            self.last_after = self.window.after(1, lambda: self._consumer())
            #self.window.after(1, self._consumer )
        else:
            self.play_thread = None
        
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
        if self.game is None:
            return
        self._delete_pitch()
        self.pitch = []
        self.pitch.append(self.canvas.create_rectangle(self.border_left, self.border_up, self.border_right+1, self.border_down+1, outline=self.pitch_color, fill=self.pitch_color) ) # pitch
        self.pitch.append(self.canvas.create_rectangle(self.left_goal, self.top_goal, self.left_x+1, self.bottom_goal+1, outline=self.lines_color, fill=self.goal_color) ) # left goal
        self.pitch.append(self.canvas.create_rectangle(self.right_x, self.top_goal, self.right_goal+1, self.bottom_goal+1, outline=self.lines_color, fill=self.goal_color) ) # right goal
        self.pitch.append(self.canvas.create_rectangle(self.left_x, self.top_y, self.right_x+1, self.bottom_y+1, outline=self.lines_color, fill="") ) # external lines
        self.pitch.append(self.canvas.create_oval(self.left_center_circle, self.up_center_circle, self.right_center_circle+1, self.down_center_circle+1, outline=self.lines_color, fill="") ) #midfield  circle
        self.pitch.append(self.canvas.create_rectangle(self.left_x, self.area_top, self.area_left+1, self.area_bottom+1, outline=self.lines_color, fill="") ) # penalty area left
        self.pitch.append(self.canvas.create_rectangle(self.area_right, self.area_top, self.right_x+1, self.area_bottom+1, outline=self.lines_color, fill="") ) # penalty area right
        self.pitch.append(self.canvas.create_line(self.midfield_x,  self.top_y, self.midfield_x, self.bottom_y+1, fill=self.lines_color) )   # midfield line
        
    def _resize_pitch(self):
        if self.game is None:
            return
        self.canvas.coords(self.pitch[0], self.border_left, self.border_up, self.border_right+1, self.border_down+1)  # pitch
        self.canvas.coords(self.pitch[1], self.left_goal, self.top_goal, self.left_x+1, self.bottom_goal+1)  # left goal
        self.canvas.coords(self.pitch[2], self.right_x, self.top_goal, self.right_goal+1, self.bottom_goal+1)  # right goal
        self.canvas.coords(self.pitch[3], self.left_x, self.top_y, self.right_x+1, self.bottom_y+1)  # external lines
        self.canvas.coords(self.pitch[4], self.left_center_circle, self.up_center_circle, self.right_center_circle+1, self.down_center_circle+1)  #midfield  circle
        self.canvas.coords(self.pitch[5], self.left_x, self.area_top, self.area_left+1, self.area_bottom+1)  # left area
        self.canvas.coords(self.pitch[6], self.area_right, self.area_top, self.right_x+1, self.area_bottom+1)  # right area
        self.canvas.coords(self.pitch[7], self.midfield_x,  self.top_y, self.midfield_x, self.bottom_y+1)  # midfield line
    
    def _delete_pitch(self):
        for o in self.pitch:
            self.canvas.delete(o)

    def _create_players(self):
        if self.game is None:
            return
        font1 = tkfont.Font(size=int(self.players_radius*self.x_ratio))

        shownumbers='hidden'
        if self.bool_show_numbers.get():
            shownumbers='normal'
        
        self._delete_players()
        self.circles=[[],[]]
        self.lines=[[],[]]
        self.numbers=[[],[]]
        for t in range(2):
            for i in range(self.game['n_players'][t]):
                lx=self._x2c(-self.players_radius)
                dy=self._y2c(-self.players_radius)
                rx=self._x2c(self.players_radius)
                uy=self._y2c(self.players_radius)
                self.circles[t].append(self.canvas.create_oval(lx, uy, rx+1, dy+1, outline=self.border_colors[t], fill=self.colors[t]) )
                p_x=self._x2c(0.0) #tick['teams'][t][i].x
                p_y=self._y2c(0.0)
                face_x=self._x2c(self.teams[t][i]['x']+ math.cos(self.teams[t][i]['direction'])*self.players_radius )
                face_y=self._y2c(self.teams[t][i]['y']+ math.sin(self.teams[t][i]['direction'])*self.players_radius )
                self.lines[t].append(self.canvas.create_line(p_x, p_y, face_x+1, face_y+1, fill=self.directionlines_colors[t]) )
                self.numbers[t].append(self.canvas.create_text(self._x2c(0.0), self._y2c(0.0), text=str(i), anchor=tk.CENTER, state=shownumbers, font=font1) )
                
        blx=self._x2c(-self.ball_radius)
        bdy=self._y2c(-self.ball_radius)
        brx=self._x2c(self.ball_radius)
        buy=self._y2c(self.ball_radius)
        self.ball_circle= self.canvas.create_oval(blx, buy, brx+1, bdy+1, outline="black", fill=self.ball_color)

    def _delete_players(self):
        for t in self.circles:
            for c in t:
                self.canvas.delete(c) 

        for t in self.lines:
            for l in t:
                self.canvas.delete(l)
        
        for t in self.numbers:
            for n in t:
                self.canvas.delete(n) 

    def _redraw_players(self):
        if self.game is None:
            return
        font1 = tkfont.Font(size=int(self.players_radius*self.x_ratio))    
        for t in range(2):
            for i in range(self.game['n_players'][t]):
                lx=self._x2c(self.teams[t][i]['x']-self.players_radius)
                dy=self._y2c(self.teams[t][i]['y']-self.players_radius)
                rx=self._x2c(self.teams[t][i]['x']+self.players_radius)
                uy=self._y2c(self.teams[t][i]['y']+self.players_radius)
                self.canvas.coords(self.circles[t][i], lx, uy, rx+1, dy+1) 
                p_x=self._x2c(self.teams[t][i]['x'])
                p_y=self._y2c(self.teams[t][i]['y'])
                face_x=self._x2c(self.teams[t][i]['x']+ math.cos(self.teams[t][i]['direction'])*self.players_radius )
                face_y=self._y2c(self.teams[t][i]['y']+ math.sin(self.teams[t][i]['direction'])*self.players_radius )
                self.canvas.coords(self.lines[t][i], p_x, p_y, face_x+1, face_y+1)
                self.canvas.coords(self.numbers[t][i], self._x2c(self.teams[t][i]['x']-self.players_radius*self.distance_numbers), self._y2c(self.teams[t][i]['y']-self.players_radius*self.distance_numbers))
                self.canvas.itemconfig(self.numbers[t][i], font=font1) 

        blx=self._x2c(self.ball_x-self.ball_radius)
        bdy=self._y2c(self.ball_y-self.ball_radius)
        brx=self._x2c(self.ball_x+self.ball_radius)
        buy=self._y2c(self.ball_y+self.ball_radius)
        self.canvas.coords(self.ball_circle, blx, buy, brx+1, bdy+1)

    def _switch_colors(self):
        if self.bool_black_andwhite.get():
            self._set_colors_blackandwhite()
        else:
            self._set_colors_colored()

        if self.game is None:
            return
        for t in range(2):
            for i in range(self.game['n_players'][t]):
                self.canvas.itemconfig(self.circles[t][i], outline=self.border_colors[t], fill=self.colors[t])
                self.canvas.itemconfig(self.lines[t][i], fill=self.directionlines_colors[t])
        self.canvas.itemconfig(self.ball_circle, fill=self.ball_color)

        self.canvas.itemconfig(self.pitch[0], outline=self.pitch_color, fill=self.pitch_color)  # pitch
        self.canvas.itemconfig(self.pitch[1], outline=self.lines_color, fill=self.goal_color)  # left goal
        self.canvas.itemconfig(self.pitch[2], outline=self.lines_color, fill=self.goal_color)  # right goal
        self.canvas.itemconfig(self.pitch[3], outline=self.lines_color, fill="")  # external lines
        self.canvas.itemconfig(self.pitch[4], outline=self.lines_color, fill="")  #midfield  circle
        self.canvas.itemconfig(self.pitch[5], outline=self.lines_color, fill="")  # left area
        self.canvas.itemconfig(self.pitch[6], outline=self.lines_color, fill="")  # right area
        self.canvas.itemconfig(self.pitch[7], fill=self.lines_color)    # midfield line

    def _update_info(self):
        self.label_info['text']=self.state_string

    def _view_shirts(self):
        if self.game is None:
            return
        for t in range(2):
            for i in range(self.game['n_players'][t]):
                if self.bool_show_numbers.get():
                    self.canvas.itemconfigure(self.numbers[t][i], state='normal')
                else:
                    self.canvas.itemconfigure(self.numbers[t][i], state='hidden')
    
    def _save_postscript(self):
        self._stop()
        filename =  filedialog.asksaveasfilename(initialdir = "./",title = "Enter image name",filetypes = (("PostScript","*.ps"),("all files","*.*")))
        if filename is not None:
            self.canvas.postscript(file=filename)

LogPlayer()
