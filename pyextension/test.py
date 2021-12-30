import robosoc2d

sim_handle = robosoc2d.build_simpleplayer_simulator([], 4, [], 4)
while robosoc2d.simulator_step_if_playing(sim_handle):
    print(robosoc2d.simulator_get_state_string(sim_handle))
print(robosoc2d.simulator_get_state_string(sim_handle))
robosoc2d.simulator_delete_all()
class MyPlayer:
    def __init__(self):
        self.c=0
    def step(self, env, pitch, settings, team1, team2):
        print("player step says that's tick per time= "+str(settings.ticks_per_time)+" , internal variable c="+str(self.c))
        self.c+=1
        action=(robosoc2d.ACTION_DASH, 1.5, 0.06, 0.0)
        return action


my_team=[MyPlayer() for n in range(4) ]
sim_handle = robosoc2d.build_simpleplayer_simulator(my_team, 0, [], 4) #, "my team", "simple players team", robosoc2d.get_seed_by_current_time(),sett)
robosoc2d.simulator_step_if_playing(handle=sim_handle)
robosoc2d.simulator_step_if_playing(handle=sim_handle)
print(robosoc2d.simulator_get_state_string(sim_handle))
robosoc2d.simulator_delete(sim_handle)

sett=robosoc2d.get_default_settings()
sett.ticks_per_time=421
sim_handle = robosoc2d.build_simpleplayer_simulator([], 4, [], 4, "The snakes",  "Pacific United", robosoc2d.get_seed_by_current_time(), sett)
#sim_handle = robosoc2d.build_simpleplayer_simulator([], 4, [], 4, "The snakes",  "Pacific United", game_settings=sett, random_seed=robosoc2d.get_seed_by_current_time())

robosoc2d.simulator_play_game(sim_handle)
simState = robosoc2d.simulator_get_game_state(sim_handle)
print(simState[0])
mydict=eval(str(simState[0]))
print(str(mydict["n_players1"]))
print(simState[1])
mydict=eval(str(simState[1]))
print(str(mydict["goal_kick_rx"]))
print(simState[2])
mydict=eval(str(simState[2]))
print(str(mydict["ticks_per_time"]))
aplayerinfo=simState[3][0]
print(aplayerinfo)
mydict=eval(str(aplayerinfo))
print(str(mydict["direction"]))
print("random seed: "+str(robosoc2d.simulator_get_random_seed(sim_handle)))
print(robosoc2d.simulator_get_team_names(sim_handle))

simState = robosoc2d.simulator_get_game_state(sim_handle)
copiedEnv =simState[0].copy()
copiedEnv.tick=100
myState = robosoc2d.environment()
print(simState[0])
print(copiedEnv)
print(myState)

print(robosoc2d.simulator_is_valid(sim_handle))
print(robosoc2d.simulator_is_valid(4000))



