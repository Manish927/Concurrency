//First program is more optimized

#include <queue>
#include <functional>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

class Scheduler {
public:
    Scheduler() : stopFlag_(false) {
        worker_ = std::thread([this] { this->run(); });
    }

    ~Scheduler() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopFlag_ = true;
            cv_.notify_all();
        }
        if (worker_.joinable()) {
            worker_.join();
        }
    }

    // Schedule a task to run after a delay
    void schedule(std::function<void()> task, std::chrono::milliseconds delay) {
        auto execTime = std::chrono::steady_clock::now() + delay;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            tasks_.emplace(execTime, std::move(task));
        }
        cv_.notify_all();
    }

private:
    using Task = std::pair<std::chrono::steady_clock::time_point, std::function<void()>>;
    struct Compare {
        bool operator()(const Task& a, const Task& b) {
            return a.first > b.first;
        }
    };

    std::priority_queue<Task, std::vector<Task>, Compare> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::thread worker_;
    std::atomic<bool> stopFlag_;

    void run() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                if (stopFlag_ && tasks_.empty()) break;
                if (tasks_.empty()) {
                    cv_.wait(lock, [this] { return stopFlag_ || !tasks_.empty(); });
                } else {
                    auto now = std::chrono::steady_clock::now();
                    auto nextTime = tasks_.top().first;
                    if (cv_.wait_until(lock, nextTime, [this, now] { return stopFlag_ || !tasks_.empty() && tasks_.top().first <= std::chrono::steady_clock::now(); })) {
                        if (stopFlag_ && tasks_.empty()) break;
                    }
                }
                if (!tasks_.empty() && tasks_.top().first <= std::chrono::steady_clock::now()) {
                    task = std::move(tasks_.top().second);
                    tasks_.pop();
                }
            }
            if (task) {
                task();
            }
        }
    }
};

int main() {
    Scheduler scheduler;
    scheduler.schedule([] { std::cout << "Task 1 after 1s\n"; }, std::chrono::seconds(1));
    scheduler.schedule([] { std::cout << "Task 2 after 2s\n"; }, std::chrono::seconds(2));
    std::this_thread::sleep_for(std::chrono::seconds(3));
    return 0;
}








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
