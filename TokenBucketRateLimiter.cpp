/*
Explanation
1.	Rate Limiter:
•	The RateLimiter class uses a token bucket algorithm.
•	Tokens are refilled at a constant rate (refillRate), and each API call consumes one token.
•	If no tokens are available, the API call is delayed until tokens are refilled.
2.	APIScheduler:
•	The scheduler integrates the RateLimiter to ensure that API calls respect the rate limit.
•	If the rate limit is exceeded, the scheduler waits briefly and retries.
3.	Main Function:
•	Demonstrates scheduling API calls with a rate limit of 2 requests per second.
•	Tasks are executed in order, respecting the rate limit.
---
Output
For the above code, the output will be:
API Call 1 executed!
API Call 2 executed!
API Call 3 executed!
API Call 4 executed!
*/



#include <iostream>
#include <queue>
#include <functional>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
using namespace std;

class RateLimiter {
private:
    int maxTokens; // Maximum number of tokens in the bucket
    int tokens;    // Current number of tokens
    int refillRate; // Tokens added per second
    chrono::time_point<chrono::steady_clock> lastRefillTime;
    mutex mtx;

    // Refill tokens based on elapsed time
    void refill() {
        auto now = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - lastRefillTime).count();
        int newTokens = (elapsed * refillRate) / 1000; // Calculate tokens to add
        if (newTokens > 0) {
            tokens = min(maxTokens, tokens + newTokens);
            lastRefillTime = now;
        }
    }

public:
    RateLimiter(int maxTokens, int refillRate)
        : maxTokens(maxTokens), tokens(maxTokens), refillRate(refillRate), lastRefillTime(chrono::steady_clock::now()) {}

    // Try to consume a token; return true if successful, false otherwise
    bool tryConsume() {
        lock_guard<mutex> lock(mtx);
        refill();
        if (tokens > 0) {
            tokens--;
            return true;
        }
        return false;
    }
};

class APIScheduler {
private:
    struct Task {
        function<void()> func;
        chrono::time_point<chrono::steady_clock> executeAt;

        bool operator>(const Task& other) const {
            return executeAt > other.executeAt;
        }
    };

    priority_queue<Task, vector<Task>, greater<Task>> taskQueue;
    mutex mtx;
    condition_variable cv;
    bool stopScheduler = false;
    RateLimiter rateLimiter;

    void schedulerThread() {
        while (true) {
            unique_lock<mutex> lock(mtx);

            cv.wait(lock, [this]() { return !taskQueue.empty() || stopScheduler; });

            if (stopScheduler && taskQueue.empty()) {
                break;
            }

            auto now = chrono::steady_clock::now();
            auto nextTask = taskQueue.top();

            if (now >= nextTask.executeAt) {
                if (rateLimiter.tryConsume()) {
                    taskQueue.pop();
                    lock.unlock();
                    nextTask.func();
                } else {
                    // If rate limit is exceeded, wait and retry
                    this_thread::sleep_for(chrono::milliseconds(100));
                }
            } else {
                cv.wait_until(lock, nextTask.executeAt);
            }
        }
    }

public:
    APIScheduler(int maxRequestsPerSecond)
        : rateLimiter(maxRequestsPerSecond, maxRequestsPerSecond) {
        thread([this]() { schedulerThread(); }).detach();
    }

    ~APIScheduler() {
        {
            lock_guard<mutex> lock(mtx);
            stopScheduler = true;
        }
        cv.notify_all();
    }

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
    APIScheduler scheduler(2); // Allow 2 API calls per second

    // Schedule API calls
    scheduler.schedule([]() { cout << "API Call 1 executed!" << endl; }, 0);
    scheduler.schedule([]() { cout << "API Call 2 executed!" << endl; }, 500);
    scheduler.schedule([]() { cout << "API Call 3 executed!" << endl; }, 1000);
    scheduler.schedule([]() { cout << "API Call 4 executed!" << endl; }, 1500);

    // Keep the main thread alive for a while to let tasks execute
    this_thread::sleep_for(chrono::seconds(5));

    return 0;
}
