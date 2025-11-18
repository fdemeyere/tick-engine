from matplotlib.animation import FuncAnimation
import matplotlib.pyplot as plt
import numpy as np
import time
import sys


feed = open("feed.txt")
fig = plt.figure()

axis = plt.axes(xlim=(0, 1),
                ylim=(0, 1))
line, = axis.plot([], [], lw=1)

prices = []


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


def frame_generator():
    i = 0
    while True:
        token = feed.readline()
        if not token:
            time.sleep(0.5)
            continue
        price = float(token)
        prices.append(price)
        yield i
        i += 1



anim = FuncAnimation(fig, animate, init_func = init,
                     frames=frame_generator(), interval=0, blit=True)
plt.show()

feed.close()
