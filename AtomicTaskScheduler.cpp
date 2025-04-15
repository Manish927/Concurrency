/*
An atomic task scheduler ensures that tasks are executed at specific times with thread safety and no overlapping execution. Below is an implementation of an atomic task scheduler in C++ using C++14, leveraging std::thread, std::mutex, and std::condition_variable for concurrency and synchronization

Explanation
1.	Task Structure:
•	Each task has a function (func) to execute and a time point (executeAt) indicating when it should run.
•	A priority_queue is used to manage tasks, with the earliest task at the top.
2.	Scheduler Thread:
•	Continuously checks the task queue for tasks ready to execute.
•	Waits until the next task's execution time or until a new task is added.
3.	Thread Safety:
•	A mutex ensures thread-safe access to the task queue.
•	A condition_variable is used to notify the scheduler thread when new tasks are added.
4.	Task Scheduling:
•	schedule: Schedules a task to run at a specific time.
•	scheduleAfter: Schedules a task to run after a delay (in milliseconds).
5.	Main Function:
•	Demonstrates scheduling three tasks with different delays.
•	The main thread sleeps to allow the scheduler to execute tasks.
---
Output
For the above code, the output will look something like this (timestamps will vary):
Task 3 executed at 1234567890123
Task 1 executed at 1234567891123
Task 2 executed at 1234567892123

Key Features
1.	Atomic Execution: Tasks are executed one at a time in the order of their scheduled time.
2.	Concurrency: The scheduler runs in a separate thread.
3.	Thread Safety: Uses mutex and condition_variable for safe access to shared resources.
4.	Flexible Scheduling: Supports scheduling tasks at specific times or after delays.
---
Complexity
•	Task Scheduling: O(log n) for adding tasks to the priority queue.
•	Task Execution: O(1) for executing the top task.
•	Space Complexity: O(n), where n is the number of tasks in the queue.
This implementation is efficient and ensures atomic execution of tasks.

*/

#include <iostream>
#include <queue>
#include <functional>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
using namespace std;

class AtomicTaskScheduler {
private:
    struct Task {
        function<void()> func; // The task to execute
        chrono::time_point<chrono::steady_clock> executeAt; // Execution time

        // Comparator for priority queue (earliest task first)
        bool operator>(const Task& other) const {
            return executeAt > other.executeAt;
        }
    };

    priority_queue<Task, vector<Task>, greater<Task>> taskQueue; // Min-heap for tasks
    mutex mtx; // Mutex for thread safety
    condition_variable cv; // Condition variable for task scheduling
    bool stopScheduler = false; // Flag to stop the scheduler
    thread schedulerThread; // Scheduler thread

    // Scheduler thread function
    void run() {
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
    AtomicTaskScheduler() {
        // Start the scheduler thread
        schedulerThread = thread([this]() { run(); });
    }

    ~AtomicTaskScheduler() {
        {
            lock_guard<mutex> lock(mtx);
            stopScheduler = true;
        }
        cv.notify_all(); // Notify all threads to stop
        if (schedulerThread.joinable()) {
            schedulerThread.join(); // Wait for the scheduler thread to finish
        }
    }

    // Schedule a task to run at a specific time
    void schedule(function<void()> func, chrono::time_point<chrono::steady_clock> time) {
        {
            lock_guard<mutex> lock(mtx);
            taskQueue.push({func, time});
        }
        cv.notify_all(); // Notify the scheduler thread
    }

    // Schedule a task to run after a delay (in milliseconds)
    void scheduleAfter(function<void()> func, int delayMs) {
        auto executeAt = chrono::steady_clock::now() + chrono::milliseconds(delayMs);
        schedule(func, executeAt);
    }
};

int main() {
    AtomicTaskScheduler scheduler;

    // Schedule tasks
    scheduler.scheduleAfter([]() { cout << "Task 1 executed at " << chrono::steady_clock::now().time_since_epoch().count() << endl; }, 1000); // 1 second delay
    scheduler.scheduleAfter([]() { cout << "Task 2 executed at " << chrono::steady_clock::now().time_since_epoch().count() << endl; }, 2000); // 2 seconds delay
    scheduler.scheduleAfter([]() { cout << "Task 3 executed at " << chrono::steady_clock::now().time_since_epoch().count() << endl; }, 500);  // 0.5 second delay

    // Keep the main thread alive for a while to let tasks execute
    this_thread::sleep_for(chrono::seconds(3));

    return 0;
}
