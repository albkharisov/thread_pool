/**
 * @file worker_pool.hpp
 * @author Albert Kharisov <albkharisov@gmail.com>
 * @version 1.0
 *
 * @brief Worker pool for parallel tasks processing.
 * TODO: refactor with templates for more versatility.
 */

#include <condition_variable>
#include <mutex>
#include <thread>
#include <tuple>

#include "worker_pool.hpp"


const unsigned int WorkerPool::nthreads = std::thread::hardware_concurrency();


void WorkerPool::set_job(Job::JobFunc job, Job::JobArgs args) {
    auto promise = std::promise<Job::JobResult>();
    auto future = promise.get_future();

    {
        std::lock_guard<std::mutex> l(m_results);
        results.push(std::move(future));
    }
    cv_results.notify_one();

    {
        std::lock_guard<std::mutex> l(m_jobs);
        jobs.push({std::move(job), std::move(args), std::move(promise)});
    }
    cv_jobs.notify_one();
}

std::optional<Job::JobResult> WorkerPool::get_answer() {
    std::future<Job::JobResult> result;

    {
        std::unique_lock l(m_results);
        cv_results.wait(l, [this]{ return !results.empty() || stop_flag.load(); });
        if (stop_flag.load() && results.empty()) { return {}; }
        result = std::move(results.front());
        results.pop();
    }
    return {result.get()};
}

void Worker::operator() () {
    std::mutex& m = owner.get_mutex();
    std::condition_variable& cv = owner.get_condvar();
    auto& jobs = owner.get_jobs();

    for (;;) {
        std::unique_lock<std::mutex> l(m);

        cv.wait(l, [this, &jobs] { return stop_flag.load() || !jobs.empty(); });

        if (stop_flag.load() && jobs.empty()) {
            break;
        }

        auto& job = jobs.front();
        // Move instead of refs because we have to destruct the tuple from queue.
        // If do so after processing the job - we would have to lock mutex again.
        auto func = std::move(Job::func(job));
        auto args = std::move(Job::args(job));
        auto promise = std::move(Job::promise(job));
        jobs.pop();
        l.unlock();

        promise.set_value(std::apply(func, args));
    }
}

