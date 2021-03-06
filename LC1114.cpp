/*
1114. Print in Order

Suppose we have a class:

public class Foo {
  public void first() { print("first"); }
  public void second() { print("second"); }
  public void third() { print("third"); }
}
The same instance of Foo will be passed to three different threads. Thread A will call first(), 
thread B will call second(), and thread C will call third(). 
Design a mechanism and modify the program to ensure that second() is executed after first(), 
and third() is executed after second().

Example 1:

Input: [1,2,3]
Output: "firstsecondthird"
Explanation: There are three threads being fired asynchronously. 
The input [1,2,3] means thread A calls first(), thread B calls second(), and thread C calls third(). 
"firstsecondthird" is the correct output.
Example 2:

Input: [1,3,2]
Output: "firstsecondthird"
Explanation: The input [1,3,2] means thread A calls first(), thread B calls third(), and thread C calls second().
"firstsecondthird" is the correct output.
 
Note:

We do not know how the threads will be scheduled in the operating system, even though the numbers in the
input seems to imply the ordering. The input format you see is mainly to ensure our tests' comprehensiveness.
*/

//Approach 1 0f 4: Using Mutex and Condition Variables.
class Foo 
{
    std::condition_variable cv;
    int spWakeUp = 1;
    std::mutex mutex;
    
public:
    Foo() {
    }

    void first(function<void()> printFirst) {
        {
          std::unique_lock lock(mutex);
          cv.wait(lock, [&](){return spWakeUp == 1; });
          printFirst();
          spWakeUp = 2;
       }
       cv.notify_all();
   }
    void second(function<void()> printSecond) {
        {
           std::unique_lock lock(mutex);
           cv.wait(lock, [&](){return spWakeUp == 2; });
           printSecond();
           spWakeUp = 3;
        }
        cv.notify_all();
    }

    void third(function<void()> printThird) {
        {
            std::unique_lock lock(mutex);
            cv.wait(lock, [&](){return spWakeUp == 3; });
            printThird();
            spWakeUp = 1;
        }
        cv.notify_all();
    }
};

//Approach 2 of 4: using Volatile

class Foo {
    volatile int count;
    
public:
    Foo() : count(1) {
        
    }

    void first(function<void()> printFirst) {
        
        printFirst();
        count++;
    }

    void second(function<void()> printSecond) {
              
        while(count != 2)
        {
            std::this_thread::yield();
        }
        
        printSecond();
        count++;
    }

    void third(function<void()> printThird) {
        
        while(count != 3)
        {
            std::this_thread::yield();
        }
        
        printThird();
        
    }
};

//Approach 3 0f 4: Using `semaphone`
#include <semaphore.h>
class Foo {
    sem_t second;
    sem_t third;
    
public:
    Foo() : {
        sem_init(&second, 0, 0);
        sem_init(&third, 0, 0);
    }

    ~Foo()
    {
        sem_destroy(&second);
        sem_destroy(&third);
    }
    void first(function<void()> printFirst) {
        
        printFirst();
        sem_post(&second);
    }

    void second(function<void()> printSecond) {
        
        sem_wait(&second);
        printSecond();
        sem_post(&third);
    }

    void third(function<void()> printThird) {
        
        sem_wait(&third)
        printThird();
    }
};

//Approach 4 0f 4: Using `atomic`
class Foo {
    std::atomic<int> count;
    
public:
    Foo() : count(1) {
        
    }

    void first(function<void()> printFirst) {
        
        printFirst();
        count.store(2, std::memory_order_release);
    }

    void second(function<void()> printSecond) {
        
        while(count != 2)
        {     }
        
        printSecond();
        count.store(3, std::memory_order_release);
    }

    void third(function<void()> printThird) {
        
        while(count != 3)
        {     }
        
        printThird();
    }
};
