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

import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

class Foo {

    Lock lock; 
    Condition cv;
    int num = 0;
    
    public Foo() {
        lock = new ReentrantLock();
        cv = lock.newCondition();
    }

    public void first(Runnable printFirst) throws InterruptedException 
    {
        lock.lock();
      
        try {
            while (num != 0)
            cv.await();
          
            // printFirst.run() outputs "first". Do not change or remove this line.
            printFirst.run();
            num = 1;
            cv.signalAll();
        } finally {
            lock.unlock();
        }
    }

    public void second(Runnable printSecond) throws InterruptedException {
        lock.lock();
        
        try
        {
           while(num != 1)
              cv.await();

          // printSecond.run() outputs "second". Do not change or remove this line.
          printSecond.run();

          num = 2;
          cv.signalAll();
        } finally {
          lock.unlock();
        }
    }

    public void third(Runnable printThird) throws InterruptedException {
        lock.lock();
        
      try {
          while (num != 2)
              cv.await();

          // printThird.run() outputs "third". Do not change or remove this line.
          printThird.run();
          num = 3;
          num = num %3;
          cv.signalAll();
      } finally {
        lock.unlock();
      }
    }
}

//Approach 2 of 4: using Volatile

class Foo {

    volatile int num = 0;
    
    public Foo() {
        
    }

    public void first(Runnable printFirst) throws InterruptedException 
    {
         // printFirst.run() outputs "first". Do not change or remove this line.
        printFirst.run();
        num = 1;  
    }

    public void second(Runnable printSecond) throws InterruptedException 
    {
        while(num < 1){
            Thread.yield();
        }
        
        // printSecond.run() outputs "second". Do not change or remove this line.
        printSecond.run();
        num = 2;    
    }

    public void third(Runnable printThird) throws InterruptedException 
    {    
        while (num < 2){
            Thread.yield();
        }
        
        // printThird.run() outputs "third". Do not change or remove this line.
        printThird.run();
    }
}

//Approach 3 0f 4: Using `semaphone`
class Foo {
    
    Semaphore second;
    Semaphore third;
    
    public Foo()
    {
        second = new Semaphore(0);
        third = new Semaphore(0);
    }
    
     public void first(Runnable printFirst) throws InterruptedException
     {
         printFirst.run();
         second.release();
     }
    
     public void second(Runnable printSecond) throws InterruptedException
     {
        second.acquire();
        printSecond.run();
        third.release();
     }
    
     public void third(Runnable printThird) throws InterruptedException 
     {
        third.acquire();
        printThird.run();
     }
}

//Approach 4 0f 4: Using `atomic`
class Foo {
    
    private AtomicInteger fence;
    
    public Foo()
    {
        fence = new AtomicInteger(0);
    }
    
     public void first(Runnable printFirst) throws InterruptedException
     {
         printFirst.run();
         fence.incrementAndGet();
     }
    
     public void second(Runnable printSecond) throws InterruptedException
     {
        while (fence.get() != 1) {

        }
         
        printSecond.run();
        fence.incrementAndGet();
     }
    
     public void third(Runnable printThird) throws InterruptedException 
     {
        while (fence.get() != 2) {

        }
        
         printThird.run();
        fence.incrementAndGet();
     }
}
