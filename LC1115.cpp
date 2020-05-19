/*
1115. Print FooBar Alternately
Suppose you are given the following code:

class FooBar {
  public void foo() {
    for (int i = 0; i < n; i++) {
      print("foo");
    }
  }

  public void bar() {
    for (int i = 0; i < n; i++) {
      print("bar");
    }
  }
}
The same instance of FooBar will be passed to two different threads. 
Thread A will call foo() while thread B will call bar(). Modify the given program to output "foobar" n times.

 

Example 1:

Input: n = 1
Output: "foobar"
Explanation: There are two threads being fired asynchronously. 
One of them calls foo(), while the other calls bar(). "foobar" is being output 1 time.
Example 2:

Input: n = 2
Output: "foobarfoobar"
Explanation: "foobar" is being output 2 times.
*/

//Approach 1: using 'semaphore'
#include <semaphore.h>

class FooBar 
{
private:
    int n;
    sem_t foo_sem;
    sem_t bar_sem;
public:
    FooBar(int n) 
    {
        this->n = n;
        sem_init(&foo_sem, 0, 1);
        sem_init(&bar_sem, 0, 0);
        ios_base::sync_with_stdio(false);
        cin.tie(NULL);
        cout.tie(NULL);
    }

    ~FooBar()
    {
        sem_destroy(&foo_sem);
        sem_destroy(&bar_sem);
    }
    void foo(function<void()> printFoo) 
    {    
        for (int i = 0; i < n; i++) 
        {    
            sem_wait(&foo_sem);
            // printFoo() outputs "foo". Do not change or remove this line.
        	  printFoo();
            sem_post(&bar_sem);
        }
    }

    void bar(function<void()> printBar) {
        
        for (int i = 0; i < n; i++) {
            sem_wait(&bar_sem);
        	  // printBar() outputs "bar". Do not change or remove this line.
        	  printBar();
            sem_post(&foo_sem);
        }
    }
};

//Approach 2: using 'mutex and condition variable'
class FooBar 
{
private:
    int n;
    std::condition_variable barrier;
    std::mutex mutex;
    bool flag = {true};
public:
    FooBar(int n) {
        this->n = n;
    }

    void foo(function<void()> printFoo) {
        
        for (int i = 0; i < n; i++) {
            
            std::unique_lock lock(mutex);
            barrier.wait(lock, [&](){return flag; });
        	  // printFoo() outputs "foo". Do not change or remove this line.
        	  printFoo();
            flag = false;
            barrier.notify_one();
        }
    }

    void bar(function<void()> printBar) {
        
        for (int i = 0; i < n; i++) {
            
            std::unique_lock lock(mutex);
            barrier.wait(lock, [&](){return !flag; });
        	  // printBar() outputs "bar". Do not change or remove this line.
        	  printBar();
            flag = true;
            barrier.notify_one();
        }
    }
};

//Approach 3: using 'volatile'

class FooBar 
{
private:
    int n;
    volatile int barrier = {0};
public:
    FooBar(int n) {
        this->n = n;
    }

    void foo(function<void()> printFoo) {
        
        for (int i = 0; i < n; i++) {
            
            while (barrier != 0)
            {
                std::this_thread::yield();
            }
        	// printFoo() outputs "foo". Do not change or remove this line.
        	printFoo();
            ++barrier;
        }
    }

    void bar(function<void()> printBar) {
        
        for (int i = 0; i < n; i++) {
            
            while(barrier != 1)
            {
                std::this_thread::yield();
            }
        	// printBar() outputs "bar". Do not change or remove this line.
        	printBar();
            --barrier;
        }
    }
};
