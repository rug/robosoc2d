import robosoc2d
import robosoc2dgui.gui as r2gui

def my_routine(exit_event, data_queue, sim_handle, waiting_time):
    not_ended = True
    while (not exit_event.wait(timeout=waiting_time) ) and not_ended:
        #play game here
        not_ended= robosoc2d.simulator_step_if_playing(sim_handle)
        data_queue.put( (robosoc2d.simulator_get_game_state(sim_handle),robosoc2d.simulator_get_state_string(sim_handle)+" !!!") )

class SillyPlayer:
    def step(self, env, pitch, settings, team1, team2):  
        action=(robosoc2d.ACTION_DASH, 0.0, 2.6, 0.0)
        #action=(robosoc2d.ACTION_NOOP, 0.0, 0.0, 0.0)
        return action

if __name__ == "__main__":
    sim_handle = robosoc2d.build_simpleplayer_simulator([], 4, [], 4)
    r2gui.play_whole_game(sim_handle)

    sim_handle = robosoc2d.build_simpleplayer_simulator([], 4, [], 4)
    r2gui.play_routine(sim_handle, my_routine, "Second game, faster!", waiting_time=0.05, width=950, height=600,
            team1color="brown", team2color="yellow", border1color="white", border2color="red", 
            direction1color="green", direction2color="purple", ball_color="red",
            pitch_color="blue", lines_color="green", goal_color="violet")

    my_team1=[SillyPlayer(), SillyPlayer()]
    my_team2=[SillyPlayer()]
    sim_handle = robosoc2d.build_simpleplayer_simulator(my_team1, 3, my_team2, 3, "The Snakes", "Pacific United", 42)
    r2gui.play_whole_game(sim_handle)
