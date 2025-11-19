#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <queue>

bool placeOrder(const std::string& cmd, double& balance, int& pos, const std::string& last_price) {
    std::string bad_type_msg = "Bad type. Must be 'b' or 's'";

    std::stringstream ss(cmd);
    std::string type, str_qty;

    std::getline(ss, type, ' ');
    std::getline(ss, str_qty);
    const int order_qty = std::stoi(str_qty);
    const int int_last_price = std::stoi(last_price);
    const int market_value = pos * int_last_price;

    if (type.size() != 1) {
        std::cout << bad_type_msg;
        return false;
    }

    char c = std::tolower(type[0]);

    if (c == 'b' && balance >= int_last_price * order_qty) {
        balance -= int_last_price * order_qty;
        pos += order_qty;
        return true;
    }
    if (c == 's' && pos >= order_qty) {
        balance += int_last_price * order_qty;
        pos -= order_qty;
        return true;
    }
    std::cout << bad_type_msg;
    return false;
}

void getStdin(double balance, int pos, std::string& last_price) {
    std::string cmd;
    while (std::getline(std::cin, cmd)) {
        placeOrder(cmd, balance, pos, last_price);
        std::cout << "bal: " << balance << "\n" << "pos: " << pos << "\n";
    }
}

int main() {
    double starting_balance = 1000;
    int starting_pos = 0;



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
            trading_thread = std::thread(getStdin, starting_balance, starting_pos, std::ref(last_price));
            active_trading = true;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    write_file.close();
    read_file.close();

    trading_thread.join();

    return 0;
}

