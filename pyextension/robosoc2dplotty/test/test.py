import robosoc2d
import robosoc2dplotty.plotty as r2plotty

sim_handle = robosoc2d.build_simpleplayer_simulator([], 4, [], 4)
robosoc2d.simulator_step_if_playing(sim_handle)

fig = r2plotty.draw(sim_handle)
r2plotty.show(fig)


