#include <iostream>
#include <queue>
#include <functional>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
using namespace std;

class APIScheduler {
private:
    struct Task {
        function<void()> func; // The API call or task to execute
        chrono::time_point<chrono::steady_clock> executeAt; // When to execute the task

        // Comparator for priority queue (earliest task first)
        bool operator>(const Task& other) const {
            return executeAt > other.executeAt;
        }
    };

    priority_queue<Task, vector<Task>, greater<Task>> taskQueue; // Min-heap for tasks
    mutex mtx; // Mutex for thread safety
    condition_variable cv; // Condition variable for task scheduling
    bool stopScheduler = false; // Flag to stop the scheduler

    // Scheduler thread function
    void schedulerThread() {
        while (true) {
            unique_lock<mutex> lock(mtx);

            // Wait until there is a task or the scheduler is stopped
            cv.wait(lock, [this]() { return !taskQueue.empty() || stopScheduler; });

            if (stopScheduler && taskQueue.empty()) {
                break; // Exit the thread if the scheduler is stopped
            }

            auto now = chrono::steady_clock::now();
            auto nextTask = taskQueue.top();

            if (now >= nextTask.executeAt) {
                // Execute the task
                taskQueue.pop();
                lock.unlock(); // Unlock before executing the task
                nextTask.func();
            } else {
                // Wait until the next task's execution time
                cv.wait_until(lock, nextTask.executeAt);
            }
        }
    }

public:
    APIScheduler() {
        // Start the scheduler thread
        thread([this]() { schedulerThread(); }).detach();
    }

    ~APIScheduler() {
        // Stop the scheduler
        {
            lock_guard<mutex> lock(mtx);
            stopScheduler = true;
        }
        cv.notify_all();
    }

    // Schedule a task to run after a delay (in milliseconds)
    void schedule(function<void()> func, int delayMs) {
        auto executeAt = chrono::steady_clock::now() + chrono::milliseconds(delayMs);
        {
            lock_guard<mutex> lock(mtx);
            taskQueue.push({func, executeAt});
        }
        cv.notify_all();
    }
};

int main() {
    APIScheduler scheduler;

    // Schedule API calls
    scheduler.schedule([]() { cout << "API Call 1 executed!" << endl; }, 1000); // 1 second delay
    scheduler.schedule([]() { cout << "API Call 2 executed!" << endl; }, 2000); // 2 seconds delay
    scheduler.schedule([]() { cout << "API Call 3 executed!" << endl; }, 500);  // 0.5 second delay

    // Keep the main thread alive for a while to let tasks execute
    this_thread::sleep_for(chrono::seconds(3));

    return 0;
}
