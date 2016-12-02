#include <iostream>
#include <stack>
#include <vector>
#include <utility>
#include <map>
#include <mutex>
#include <thread>

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

public:
    static ThreadManager& getInstance()
    {
        static ThreadManager instance;

        return instance;
    }

    ThreadManager(ThreadManager const&) = delete;
    ThreadManager& operator=(const ThreadManager&) = delete;

private:
    ThreadManager() 
    {
        unsigned int threadsCount = std::thread::hardware_concurrency();
        if( threadsCount <= 1 )
            threadsCount = ThreadManager::cThreadsCount;

        for(int i = 0; i < threadsCount; ++i)
        { // Threads initialization
            std::thread && thread = std::thread([]() -> void
            {  // Thread life cicle
                
            });// Thread life cicle
            
            thread.detach();
            std::thread::id threadId = thread.get_id();
            TThreadData && threadData = std::make_pair(std::move(thread), TPrimes());
            mThreadsPool.emplace(threadId, std::move(threadData));
        } // Threads initialization
    };
};

int main()
{
    ThreadManager::getInstance();

    return 0;
}