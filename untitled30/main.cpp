#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <mutex>

std::mutex mtx;

class Station {
    int currentTrain = 0;

public:
    std::mutex stationMutex;

    int getCurrentTrain() {
        return currentTrain;
    }

    void setCurrentTrain(int state) {
        currentTrain = state;
    }
};

class Train {
    std::string name;
    double time;
    Station* station;
    bool checkPoint = false;
    std::thread trainThread;
public:
    Train(const std::string& inName, double inTime, Station* inStation) : name(inName), time(inTime), station(inStation) {}

    void startMotion() {
        trainThread = std::thread(&Train::move, this);
    }

    void joinTrainThread() {
        if (trainThread.joinable()) {
            trainThread.join();
        }
    }

    void move() {
        int count = 0;
        while (true) {
            mtx.lock();
            std::cout << name << " travel time - " << ++count << " sec" << std::endl;
            mtx.unlock();
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (time == count && !checkPoint) {
                station->stationMutex.lock();
                while (station->getCurrentTrain() == 1) {
                    std::this_thread::sleep_for(std::chrono::seconds(3));
                }
                station->stationMutex.unlock();
                station->setCurrentTrain(1);
                mtx.lock();
                std::cout << name << " on station!" << std::endl;
                mtx.unlock();
                waitForDepart();
                break;
            }
        }
    }

    void waitForDepart() {
        while (true) {
            if (station->getCurrentTrain() == 1) {
                std::string user_command;
                mtx.lock();
                std::cout << "depart? (train " << name << ") "<<std::endl;
                mtx.unlock();
                std::cin >> user_command;

                if (user_command == "depart") {
                    station->setCurrentTrain(0);
                    mtx.lock();
                    std::cout << name << " departed!" << std::endl;
                    mtx.unlock();
                    break;
                }
            }
        }
    }
};

int main() {
    Station station;
    std::vector<Train*> trains;
    std::vector<std::string> names = { "A", "B", "C" };
    double user_time;

    for (int i = 0; i < names.size(); ++i) {
        std::cout << "Enter travel time for " << names[i] << " train (in seconds): ";
        std::cin >> user_time;
        Train* train = new Train(names[i], user_time, &station);
        trains.push_back(train);
    }

    for (int i = 0; i < trains.size(); ++i) {
        mtx.lock();
        trains[i]->startMotion();
        mtx.unlock();
    }

    for (int i = 0; i < trains.size(); ++i) {
        trains[i]->joinTrainThread();
    }

    for (int i = 0; i < trains.size(); ++i) {
        delete trains[i];
    }
}
