#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <queue>

struct Order {
    char type;
    int qty;
};

struct OrderQueue {
    std::queue<Order> queue;
    std::mutex mutex;
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
        std::lock_guard<std::mutex> lock(orders.mutex);
        orders.queue.push(ord);
    }
}

void executeOrders(OrderQueue& orders, double& balance, int& pos, const double last_price) {
    while (true) {
        Order ord;
        {
            std::lock_guard<std::mutex> lock(orders.mutex);
            if (orders.queue.empty()) {
                break;
            }
            ord = orders.queue.front();
            orders.queue.pop();
        }

        const char type = ord.type;
        const int qty = ord.qty;

        if ( type == 'b' && balance >= last_price * qty) {
            balance -= last_price * qty;
            pos += qty;
        } else if (type == 's' && pos >= qty) {
            balance += last_price * qty;
            pos -= qty;
        } else {
            std::cout << "Not enough cash or shares\n";
        }

        std::cout << "bal: " << balance << "\n" << "pos: " << pos << "\n";
    }
}

int main() {
    double bal = 1000;
    int pos = 0;

    OrderQueue orders;

    std::string last_price;
    bool active_trading = false;

    std::thread trading_thread;

    std::string line;
    std::ifstream read_file("../HistoricalData_1763459211275.csv");
    std::ofstream write_file("../feed.txt");

    getline(read_file, line);

    int tick = 0;

    while (getline(read_file, line)) {
        std::stringstream ss(line);
        std::string date;

        char comma = ',';

        std::getline(ss, date, comma);
        std::getline(ss, last_price, comma);

        last_price = last_price.substr(1);

        if (!active_trading) {
            trading_thread = std::thread(placeOrders, std::ref(orders));
            active_trading = true;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        executeOrders(orders, bal, pos, std::stod(last_price));
        write_file << tick << comma << last_price << comma << pos << comma << bal << "\n" << std::flush;
        tick++;
    }
    write_file.close();
    read_file.close();

    trading_thread.join();

    return 0;
}