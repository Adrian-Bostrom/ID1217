#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <iostream> 
#include <ctime>

#define MAX_TIME 5000 //max time to sleep between trips in ms
#define BRIDGE_MAX_TIME 2000 //max time to sleep between trips in ms
time_t timestamp;
struct tm datetime = *localtime(&timestamp);

class BridgeMonitor {
public:
    BridgeMonitor() : north_dribing(false), cars_on_le_bridge(0) {}

    void enqueueCar(int id, const std::string& direction) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (direction == "North") {
            north_queue.push(id);
            cv_north.notify_all();
        } else {
            south_queue.push(id);
            cv_south.notify_all();
        }
        time(&timestamp);
        datetime = *localtime(&timestamp);
        std::cout << datetime.tm_hour << ":" << datetime.tm_min << ":" << datetime.tm_sec << " Car " << id << " got in queue to go " << direction << std::endl;
    }

    void crossBridge(int id, const std::string& direction) {
        std::unique_lock<std::mutex> lock(mutex_);
        if(direction == "North"){
            cv_north.wait(lock, [this, direction, id] {
                return (north_dribing && direction == "North" && !north_queue.empty() && north_queue.front() == id);                         
            });
        }
        else{   
            cv_south.wait(lock, [this, direction, id] {
                return (!north_dribing && direction == "South" && !south_queue.empty() && south_queue.front() == id);            
            });
        }

        if (direction == "North") {
            north_queue.pop();
        } else {
            south_queue.pop();
        }
        time(&timestamp);
        std::cout << datetime.tm_hour << ":" << datetime.tm_min << ":" << datetime.tm_sec << " Car " << id << " is crossing the bridge in direction " << direction << std::endl;
        cars_on_le_bridge++;
        lock.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(rand() % BRIDGE_MAX_TIME)); // Simulate time to cross the bridge

        lock.lock();
        cars_on_le_bridge--;
        time(&timestamp);
        std::cout << datetime.tm_hour << ":" << datetime.tm_min << ":" << datetime.tm_sec << " Car " << id << " has crossed the bridge in direction " << direction << std::endl;

        
        if (north_dribing){
            if (north_queue.empty() && cars_on_le_bridge == 0){
                //std::this_thread::sleep_for(std::chrono::seconds(1));
                north_dribing=false;
                cv_south.notify_all();
            }
            cv_north.notify_all();
        }
        else if(south_queue.empty() && cars_on_le_bridge == 0){
            //std::this_thread::sleep_for(std::chrono::seconds(1));
            north_dribing=true;
            cv_north.notify_all();
        }
        else cv_south.notify_all();

        }
    

    private:
        std::mutex mutex_;
        std::condition_variable cv_north, cv_south;
        bool north_dribing;
        int cars_on_le_bridge;
        std::queue<int> north_queue;
        std::queue<int> south_queue;
    };

void car(BridgeMonitor& monitor, int id, std::string direction, int trips) {
 
    for(int i = 0; i < trips; i++){
        monitor.enqueueCar(id, direction);
        monitor.crossBridge(id, direction);
        if(direction == "North"){
            direction = std::string("South");
        } else{
            direction = std::string("North");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(rand() % MAX_TIME));
    }
}

int main() {
    int northcars, southcars, trips;
    
    std::cout << "Type a number of trips: ";
    std::cin >> trips;
    std::cout << "Type a number of initial northbound cars: ";
    std::cin >> northcars;
    std::cout << "Type a number of initial southbound cars: ";
    std::cin >> southcars;
    BridgeMonitor monitor;
    std::vector<std::thread> cars;
    for (int i = 0; i < northcars; ++i) {
        cars.emplace_back(car, std::ref(monitor), i, "North", trips);
    }
    for (int i = northcars; i < southcars + northcars; ++i) {
        cars.emplace_back(car, std::ref(monitor), i, "South", trips);
    }

    for (auto& car_thread : cars) {
        car_thread.join();
    }

    return 0;
}