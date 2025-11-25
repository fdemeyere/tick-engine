#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <queue>
#include <condition_variable>

struct Order {
    char type;
    int qty;
};

struct OrderQueue {
    std::queue<Order> queue;
    std::mutex mutex;
    std::condition_variable cv;
};

struct TradingState {
    std::mutex mutex;
    double last_price;
    double bal;
    int pos;

    TradingState(const double last_price, const double bal, const int pos) : last_price(last_price), bal(bal), pos(pos) {}
};

bool parseOrder(const std::string& cmd, char& c, int& qty) {
    const std::string err_msg = "Bad request";
    std::stringstream ss(cmd);
    std::string type, str_qty;

    std::getline(ss, type, ' ');
    std::getline(ss, str_qty);

    if (type.size() != 1) {
        std::cout << err_msg;
        return false;
    }

    c = type[0];
    if (c != 'b' && c != 's') {
        std::cout << err_msg;
        return false;
    }

    qty = std::stoi(str_qty);
    return true;
}

void placeOrders(OrderQueue& orders) {
    std::string cmd;
    char c;
    int qty;
    while (std::getline(std::cin, cmd)) {
        if (!parseOrder(cmd, c, qty)) {
            continue;
        }
        Order ord = {c, qty};
        {
            std::lock_guard<std::mutex> lock(orders.mutex);
            orders.queue.push(ord);
        }
        orders.cv.notify_one();
    }
}

void executeOrders(OrderQueue& orders, TradingState& trading_state) {
    while (true) {
        Order ord;
        {
            std::unique_lock<std::mutex> lock(orders.mutex);
            orders.cv.wait(lock, [&orders] { return !orders.queue.empty(); });

            ord = orders.queue.front();
            orders.queue.pop();
        }

        const char type = ord.type;
        const int qty = ord.qty;

        std::lock_guard<std::mutex> lock(trading_state.mutex);
        if ( type == 'b' && trading_state.bal >= trading_state.last_price * qty) {
            trading_state.bal -= trading_state.last_price * qty;
            trading_state.pos += qty;
        } else if (type == 's' && trading_state.pos >= qty) {
            trading_state.bal += trading_state.last_price * qty;
            trading_state.pos -= qty;
        } else {
            std::cout << "Not enough cash or shares\n";
        }

        std::cout << "bal: " << trading_state.bal << "\n" << "pos: " << trading_state.pos << "\n";
    }
}

int main() {
    TradingState trading_state(-1, 1000, 0);

    OrderQueue orders;

    std::string last_price;
    bool active_trading = false;

    std::thread consumer_thread;
    std::thread producer_thread;

    std::string line;
    // std::ifstream read_file("../HistoricalData_1763459211275.csv"); // Apple
    std::ifstream read_file("../HistoricalData_1763676611258.csv"); // Amazon
    std::ofstream write_file("../feed.txt");

    getline(read_file, line);

    int tick = 0;

    while (getline(read_file, line)) {
        std::stringstream ss(line);
        std::string date;

        char comma = ',';

        std::getline(ss, date, comma);
        std::getline(ss, last_price, comma);

        {
            std::lock_guard<std::mutex> lock(trading_state.mutex);
            trading_state.last_price = std::stod( last_price.substr(1));
        }

        if (!active_trading) {
            consumer_thread = std::thread(executeOrders, std::ref(orders), std::ref(trading_state));
            producer_thread = std::thread(placeOrders, std::ref(orders));
            active_trading = true;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        std::lock_guard<std::mutex> lock(trading_state.mutex);
        write_file << tick << comma << trading_state.last_price << comma << trading_state.pos << comma << trading_state.bal << "\n" << std::flush;
        tick++;
    }
    write_file.close();
    read_file.close();

    producer_thread.join();
    consumer_thread.join();

    return 0;
}