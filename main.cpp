#include <iostream>
#include <stack>
#include <vector>
#include <utility>
#include <map>
#include <cmath>
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
    // Helpers
    void merge(const TPrimes & from, TPrimes & to)
    {
        for( auto it = from.begin(); it != from.end(); ++it )
        {
            unsigned itPrime = it->first;
            unsigned itPower = it->second;

            auto toPrimeIt = to.find(itPrime);
            if( toPrimeIt != to.end() )
            {
                unsigned toPrimeItPower = toPrimeIt->second;
                if( toPrimeItPower < itPower )
                    to[itPrime] = itPower;
            }
            else
                to[itPrime] = itPower;
        }
    }

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
                bool numberIsNotZero = true;
                std::thread::id threadId = std::this_thread::get_id();
                TThreadData & threadData = mThreadsPool[threadId];
                TPrimes & threadPrimes = threadData.second;

                do
                {
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
                        numberIsNotZero = false;
                        break;
                    }

                    // We got number, unlock and notify other threads
                    mutexLock.unlock();
                    mStackEmpty.notify_all();

                    // Do number factorization
                    TPrimes numberPrimes;
                    while( number != 1 )
                    {

                        bool isMultiplier = false;
                        for( auto it = mPrimes.begin(); 
                             !isMultiplier && (*it) <= number && it != mPrimes.end(); 
                             ++it  )
                        { // Number factorization
                            isMultiplier = (number % (*it)) == 0;

                            if( isMultiplier )
                            {
                                auto primeIt = numberPrimes.find(*it);
                                if( primeIt != numberPrimes.end() )
                                    numberPrimes[*it] += 1;
                                else
                                    numberPrimes[*it] = 1;

                                number = number / (*it);
                            }
                        } // Number factorization
                    }

                    // Merge numberPrimes into threadPrimes
                    merge(numberPrimes, threadPrimes);
                } while(numberIsNotZero);
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

    // Waits until all threads finish work and writes their data to result using merge
    void stop(TPrimes & result)
    {
        for (auto && threadIt: mThreadsPool) 
        {
            TThreadData & threadData = threadIt.second;
            std::thread & thread = threadData.first;
            if( thread.joinable() )
            {
                thread.join();
                // Merge data to result
                TPrimes & threadPrimes = threadData.second;
                merge(threadPrimes, result);
            }
        }
    }

    ThreadManager(ThreadManager const&) = delete;
    ThreadManager& operator=(const ThreadManager&) = delete;
    ~ThreadManager() {}
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

    std::map<unsigned, unsigned> result;
    ThreadManager::getInstance().stop(result);

    unsigned int lcm = 1;
    for( auto && it: result )
    {
        std::cout << it.first << " : " << it.second << std::endl;
        lcm = lcm * pow(it.first, it.second);
    }
    std::cout << "Least common multiplier is " << lcm << std::endl;

    return 0;
}