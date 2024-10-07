/**
 * @file worker_pool.hpp
 * @author Albert Kharisov <albkharisov@gmail.com>
 * @version 1.0
 *
 * @brief Worker pool for parallel tasks processing.
 * TODO: refactor with templates for more versatility.
 */

#pragma once

#include <condition_variable>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <optional>

/**
 * @brief Debug class for thread-safe cout printing.
 */
class SafeCout {
    /** mutex guard for cout */
    static inline std::mutex m;

public:
    /** A constructor. Using RAII for management. */
    SafeCout()  { SafeCout::m.lock(); }
    /** A destructor. Using RAII for management. */
    ~SafeCout() { SafeCout::m.unlock(); }

    /** An operator<<. Overloads common way of printing in C++. */
    template <typename T>
    SafeCout& operator<<(const T& t) {
        std::cout << t;
        return *this;
    }

    /** An operator<<(for manip). An overloading for std::endl and others. */
    SafeCout& operator<<(std::ostream& (*manip)(std::ostream&)) {
        std::cout << manip;
        return *this;
    }
};

class WorkerPool;

/**
 * Class for processing jobs.
 * This class manages internal thread's lifetime.
 * It only should be manages from WorkerPool class.
 */
class Worker {
    friend WorkerPool;
    /** Link to the owner of worker */
    WorkerPool& owner;
    /** Flag to terminate the worker */
    std::atomic<bool> stop_flag{false};
    /** Thread in which worker processes jobs */
    std::thread thread;

    /**
     * Function worker processes jobs.
     *
     * It waits for the jobs to appear in worker pool,
     * processes them and puts the result back in
     * worker pool's result queue.
     */
    void operator()();

    /** Forbid this. Can't create Worker outside the pool */
    Worker(const Worker&) = delete;
    /** Forbid this. Can't create Worker outside the pool */
    Worker& operator=(const Worker&) = delete;
    /** Forbid this. Can't create Worker outside the pool */
    Worker(Worker&&) = delete;
    /** Forbid this. Can't create Worker outside the pool */
    Worker& operator=(Worker&&) = delete;
public:
    /** Signal to terminate the worker */
    void stop(void) { stop_flag.store(true); }

    /**
     * A constructor.
     * Immediately starts jobs processing.
     */
    Worker (WorkerPool& pool)
            : owner(pool),
              thread(&Worker::operator(), this) {}

    /**
     * A destructor.
     * Waits for the thread to stop.
     */
    ~Worker() {
        if (thread.joinable()) {
            thread.join();
        }
    }
};

/**
 * Handy class for retreiving job func/arg/promise from tuple.
 */
struct Job {
    /** Job result type */
    using JobResult = std::string;
    /** Job arguments tuple type */
    using JobArgs = std::tuple<std::string, std::string, std::string>;
    /** Job function type */
    using JobFunc = std::function<JobResult(std::string, std::string, std::string)>;
    /** Unit of storage all necessery parameters to process job */
    using JobRequest = std::tuple<JobFunc, JobArgs, std::promise<JobResult>>;

    /** Get a job function from tuple */
    static auto& func(JobRequest& request) { return std::get<0>(request); }
    /** Get job arguments from tuple */
    static auto& args(JobRequest& request) { return std::get<1>(request); }
    /** Get a promise from tuple */
    static auto& promise(JobRequest& request) { return std::get<2>(request); }
};

/**
 * @brief A class for handy task parallelizing.
 *
 * Starts N threads and distribute incoming tasks.
 * Tasks are set with `set_job()`. Results of the operations
 * are acquired via `get_answer()`. Results are
 * guaranteed to be in the same order as jobs came.
 *
 * Usage example:
 *
 *      WorkerPool worker_pool();
 *      worker_pool.set_job(my_job, {"s1", "s2", "s3"});
 *      auto result = worker_pool.get_answer();
 *      worker_pool.stop();
 */
class WorkerPool {
    friend Worker;
    /** Type of jobs to be performed */
    using Jobs = std::queue<Job::JobRequest>;

    /** Queue for a results. External access via `get_answer()` */
    std::queue<std::future<Job::JobResult>> results;
    /** Container of tasks to perform */
    Jobs jobs;
    /** Condvar for jobs */
    std::condition_variable cv_jobs;
    /** Mutex for jobs */
    std::mutex m_jobs;
    /** Condvar for results */
    std::condition_variable cv_results;
    /** Mutex for results */
    std::mutex m_results;

    // has to be initialized last
    /** Workers to process incoming tasks */
    std::vector<std::unique_ptr<Worker>> workers;

public:
    /** Hardware supported concurrency */
    static const unsigned int nthreads;

    /**
     * A constructor.
     *
     * Starts workers.
     *
     * @param num_threads number of workers to start for \
     *  parallel processing
     */
    WorkerPool(unsigned num_threads = nthreads) {
        SafeCout() << "WorkerPool start with " << num_threads << " threads" << std::endl;

        workers.reserve(num_threads);
        for (unsigned i = 0; i < num_threads; ++i) {
            workers.emplace_back(std::make_unique<Worker>(*this));
        }
    }

    /**
     * A destructor.
     *
     * Notifies all workers to stop waiting for next tasks.
     * Doesn't wait for a workers threads to join because
     * it's done in workers destructors.
     */
    ~WorkerPool() {
        for (auto& w : workers) { w->stop(); }
        cv_jobs.notify_all();
    }

    /**
     * Sets job to process.
     *
     * @param job job to be performed
     * @param args arguments to pass to the job
     */
    void set_job(Job::JobFunc job, Job::JobArgs args);

    /**
     * Acquiring the result of the job. Blocking.
     * If no job set waits till it's set.
     * The only way to return from the blocking is either
     * get the job done or stop worker pool.
     *
     * @return result of the operation or an empty
     *  optional if worker pool is stopped.
     */
    std::optional<Job::JobResult> get_answer();

    /**
     * Stop worker pool and release all waiters from blocking.
     */
    void stop() {
        stop_flag.store(true);
        cv_results.notify_all();
    }

private:
    /** Flag for stopping the waiters */
    std::atomic<bool> stop_flag{false};
    /** Get jobs mutex  */
    std::mutex& get_mutex() { return m_jobs; }
    /** Get jobs condvar  */
    std::condition_variable& get_condvar() { return cv_jobs; }
    /** Get jobs  */
    Jobs& get_jobs() { return jobs; }
};

