#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>


int main() {
    std::string line;
    std::ifstream read_file("../HistoricalData_1763459211275.csv");
    std::ofstream write_file("../feed.txt");

    getline(read_file, line);

    while (getline(read_file, line)) {
        std::stringstream ss(line);
        std::string date, last_price;

        char comma = ',';

        std::getline(ss, date, comma);
        std::getline(ss, last_price, comma);

        last_price = last_price.substr(1);

        write_file << last_price << "\n" << std::flush;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    write_file.close();
    read_file.close();

    return 0;
}

