from matplotlib.animation import FuncAnimation
import matplotlib.pyplot as plt
import numpy as np
import time


prices_file = open("feed.txt")

prices = []

for line in prices_file:
    prices.append(float(line.split(",")[1]))

fig = plt.figure()

axis = plt.axes(xlim=(0, len(prices)-1),
                ylim=(0, 500))
line, = axis.plot([], [], lw=2)


def init():
    line.set_data([], [])
    return line,


def animate(i):
    x = np.linspace(0, i, i+1)
    y = prices[0:i+1]
    line.set_data(x, y)

    curr_slice = prices[:i+1]
    min_so_far = min(curr_slice)
    max_so_far = max(curr_slice)

    axis.set_xlim(0, i)
    axis.set_ylim(min_so_far*0.8, max_so_far*1.2)

    return line,


anim = FuncAnimation(fig, animate, init_func = init,
                     frames=len(prices), interval=20, blit=False)
plt.show()





