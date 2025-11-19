from matplotlib.animation import FuncAnimation
import matplotlib.pyplot as plt
import numpy as np

import time
import threading

feed = open("feed.txt")
# fig = plt.figure()

# axes = plt.axes()
# line, = axes.plot([], [], lw=1)

prices = []
prices_lock = threading.Lock()


def init(line):
    line.set_data([], [])
    return line,


def animate(i, line, axis):
    with prices_lock:
        if not prices:
            return line,
        x = np.arange(len(prices))
        y = np.array(prices)
        line.set_data(x, y)

    min_so_far = min(prices)
    max_so_far = max(prices)

    axis.set_xlim(0, len(y) - 1)
    axis.set_ylim(min_so_far*0.8, max_so_far*1.2)

    return line,


def reader_thread_func():
    while True:
        row = feed.readline()
        if not row:
            time.sleep(0.5)
            continue

        parts = row.split(",")
        price = float(parts[1])

        with prices_lock:
            prices.append(price)


def main():
    t = threading.Thread(target=reader_thread_func, daemon=True)
    t.start()

    fig, axis = plt.subplots()
    line, = axis.plot([], [], lw=1)

    anim = FuncAnimation(fig, animate, init_func=lambda: init(line),
                         fargs=(line, axis), interval=800, blit=True)

    plt.show()
    feed.close()


if __name__ == "__main__":
    main()
