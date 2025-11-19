#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <queue>

struct Order{
    char type;
    int qty;
};

bool parseOrder(const std::string& ord, char& c, int& qty) {
    const std::string err_msg = "Bad request";
    std::stringstream ss(ord);
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

void placeOrders(std::queue<Order>& orders) {
    std::string ord;
    char c;
    int qty;
    while (std::getline(std::cin, ord)) {
        if (!parseOrder(ord, c, qty)) {
            continue;
        }

        Order o = {c, qty};
        orders.push(o);
    }
}

void executeOrders(std::queue<Order>& orders, double& balance, int& pos, const double last_price) {
    while (!orders.empty()) {
        Order ord = orders.front();
        const char type = ord.type;
        const int qty = ord.qty;
        if ( type == 'b' && balance >= last_price * qty) {
            balance -= last_price * qty;
            pos += qty;
        } else if (type == 's' && pos >= qty) {
            balance += last_price * qty;
            pos -= qty;
        } else {
            std::cout << "Not enough cash or shares" << "\n";
        }
        orders.pop();
        std::cout << "bal: " << balance << "\n" << "pos: " << pos << "\n";
    }
}

int main() {
    double balance = 1000;
    int pos = 0;

    std::queue<Order> orders;

    std::string last_price;
    bool active_trading = false;

    std::thread trading_thread;

    std::string line;
    std::ifstream read_file("../HistoricalData_1763459211275.csv");
    std::ofstream write_file("../feed.txt");

    getline(read_file, line);

    while (getline(read_file, line)) {
        std::stringstream ss(line);
        std::string date;

        char comma = ',';

        std::getline(ss, date, comma);
        std::getline(ss, last_price, comma);

        last_price = last_price.substr(1);

        write_file << last_price << "\n" << std::flush;
        if (!active_trading) {
            trading_thread = std::thread(placeOrders, std::ref(orders));
            active_trading = true;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        executeOrders(orders, balance, pos, std::stod(last_price));

    }
    write_file.close();
    read_file.close();

    trading_thread.join();

    return 0;
}

