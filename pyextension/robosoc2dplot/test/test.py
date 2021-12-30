import robosoc2d
import robosoc2dplot.plot as r2plot

sim_handle = robosoc2d.build_simpleplayer_simulator([], 4, [], 4)
robosoc2d.simulator_step_if_playing(sim_handle)
fig, ax = r2plot.draw(sim_handle)
r2plot.show() # same as plt.show() but without the need to "import matplotlib.pyplot as plt"