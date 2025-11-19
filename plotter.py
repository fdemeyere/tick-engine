from matplotlib.animation import FuncAnimation
import matplotlib.pyplot as plt
import numpy as np

import time
import threading

feed = open("feed.txt")

prices = []
prices_lock = threading.Lock()

equities = []
equities_lock = threading.Lock()


def init(line):
    line.set_data([], [])
    return line,


def animate_price(i, line, axis):
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

    axis.set_xticks(np.arange(0, len(y), 10))

    return line,


def animate_equity(i, line, axis):
    with equities_lock:
        if not equities:
            return line,
        x = np.arange(len(equities))
        y = np.array(equities)
        line.set_data(x, y)

    min_so_far = min(equities)
    max_so_far = max(equities)

    axis.set_xlim(0, len(y)-1)
    axis.set_ylim(min_so_far*0.8, max_so_far*1.2)

    axis.set_xticks(np.arange(0, len(y), 10))

    return line,


def reader_thread_func():
    while True:
        row = feed.readline()
        if not row:
            time.sleep(0.5)
            continue

        parts = row.split(",")
        price = float(parts[1])
        equity = float(parts[1]) * float(parts[2]) + float(parts[3])

        with prices_lock:
            prices.append(price)

        with equities_lock:
            equities.append(equity)


def main():
    t = threading.Thread(target=reader_thread_func, daemon=True)
    t.start()

    # fig, axis = plt.subplots(1, 2)
    fig, (ax1, ax2) = plt.subplots(2, 1)
    ax1.set_title("Mid-price")
    ax1.set_xlabel("Day")
    ax1.set_ylabel("Dollars ($)")

    ax2.set_title("Equity")
    ax2.set_xlabel("Day")
    ax2.set_ylabel("Dollars ($)")

    line1, = ax1.plot([], [], lw=1)
    line2, = ax2.plot([], [], lw=1)

    anim1 = FuncAnimation(fig, animate_price, init_func=lambda: init(line1),
                         fargs=(line1, ax1), interval=800, blit=False)

    anim2 = FuncAnimation(fig, animate_equity, init_func=lambda: init(line2),
                         fargs=(line2, ax2), interval=800, blit=False)

    plt.show()
    feed.close()


if __name__ == "__main__":
    main()
