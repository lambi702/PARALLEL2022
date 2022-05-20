// https://stackoverflow.com/questions/15752659/thread-pooling-in-c11

#include <functional>
#include <condition_variable>

class ThreadPool {
public:
    void Start(uint32_t num_threads);
    void QueueJob(const std::function<void()>& job);
    void Stop();
    void busy();

    static void ThreadLoop();

    bool should_terminate = false;           // Tells threads to stop looking for jobs
    std::mutex queue_mutex;                  // Prevents data races to the job queue
    std::condition_variable mutex_condition; // Allows threads to wait on new jobs or termination 
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> jobs;
};

// For an efficient threadpool implementation, once threads are created according to num_threads, 
// it's better not to create new ones or destroy old ones (by joining). 
// There will be a performance penalty, and it might even make your application go slower than the serial version.
//  Thus, we keep a pool of threads that can be used at any time (if they aren't already running a job).
// Each thread should be running its own infinite loop, constantly waiting for new tasks to grab and run.

void ThreadPool::Start(uint32_t num_threads) {
    threads.resize(num_threads);
    for (uint32_t i = 0; i < num_threads; i++) {
        threads.at(i) = std::thread(ThreadLoop);
    }
}

// The infinite loop function. This is a while (true) loop waiting for the task queue to open up.

void ThreadPool::ThreadLoop() {
    while (true) {
        std::function<void()> job;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            mutex_condition.wait(lock, [this] {
                return !jobs.empty() || should_terminate;
            });
            if (should_terminate) {
                return;
            }
            job = jobs.front();
            jobs.pop();
        }
        job();
    }
}

// Add a new job to the pool; use a lock so that there isn't a data race.
// To use it:
// thread_pool->QueueJob([] { /* ... */ });

void ThreadPool::QueueJob(const std::function<void()>& job) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        jobs.push(job);
    }
    mutex_condition.notify_one();
}

// The busy() function can be used in a while loop, such that the main thread 
// can wait the threadpool to complete all the tasks before calling the threadpool destructor.

void ThreadPool::busy() {
    bool poolbusy;
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        poolbusy = jobs.empty();
    }
    return poolbusy;
}

// Stop the pool

void ThreadPool::Stop() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        should_terminate = true;
    }
    mutex_condition.notify_all();
    for (std::thread& active_thread : threads) {
        active_thread.join();
    }
    threads.clear();
}