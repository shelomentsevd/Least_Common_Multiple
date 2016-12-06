#include <iostream>
#include <stack>
#include <vector>
#include <utility>
#include <map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>

#include "eratosthenes_sieve.hpp"

#define MAXIMUM 10000

// Singleton pattern
class ThreadManager 
{
    // Typedefs
    typedef std::map<unsigned, unsigned> TPrimes;
    typedef std::pair<std::thread, TPrimes> TThreadData;

    // Consts
    const unsigned int cThreadsCount = 4;

    // Members
    std::map<std::thread::id, TThreadData> mThreadsPool;
    std::vector<unsigned int> mPrimes;
    // Number stack and his mutex
    std::stack<unsigned int> mNumbersStack;
    std::mutex mNumbersStackMutex;
    // Condition variable
    std::condition_variable mStackEmpty;
public:
    static ThreadManager& getInstance()
    {
        static ThreadManager instance;

        return instance;
    }

    void initilize()
    {
        unsigned int threadsCount = std::thread::hardware_concurrency();
        if( threadsCount <= 1 )
            threadsCount = cThreadsCount;

        for(int i = 0; i < threadsCount; ++i)
        { // Threads initialization
            std::thread && thread = std::thread([&]() -> void
            {  // Thread life cicle
                // Wait until we got data inside stack
                std::unique_lock<std::mutex> mutexLock(mNumbersStackMutex);
                mStackEmpty.wait(mutexLock, [&]
                {  // Wait until stack empty
                    return !mNumbersStack.empty();
                });// Wait until stack empty

                unsigned int number = mNumbersStack.top();
                
                if( number )
                { // Remove from stack
                    mNumbersStack.pop();
                }
                else
                { // If zero - terminate thread
                    // TODO: terminate thread   
                }

                // We got number, unlock and notify other threads
                mutexLock.unlock();
                mStackEmpty.notify_all();
                // TODO: Calculations here
            });// Thread life cicle
            
            std::thread::id threadId = thread.get_id();
            TThreadData && threadData = std::make_pair(std::move(thread), TPrimes());
            mThreadsPool.emplace(threadId, std::move(threadData));
        } // Threads initialization
    }

    void add(unsigned int number)
    {
        std::unique_lock<std::mutex> mutexLock(mNumbersStackMutex);
        mNumbersStack.push(number);
        mutexLock.unlock();
        mStackEmpty.notify_all();
    }

    ThreadManager(ThreadManager const&) = delete;
    ThreadManager& operator=(const ThreadManager&) = delete;
    ~ThreadManager()
    {
        for (auto && threadIt: mThreadsPool) 
        {
            TThreadData & threadData = threadIt.second;
            std::thread & thread = threadData.first;
            if( thread.joinable() )
                thread.join();
        }
    }
private:
    ThreadManager() 
    {
        mPrimes = eratosthenes_sieve::primes(MAXIMUM);
    }
};

int main()
{
    ThreadManager::getInstance().initilize();

    unsigned int number = 0;
    
    do 
    { // Add numbers until user don't put zero
        std::cin >> number;
        
        if( number > MAXIMUM )
            continue;

        ThreadManager::getInstance().add(number);
    } while ( number );
    // TODO: Get data from threads, show result, free memory

    return 0;
}