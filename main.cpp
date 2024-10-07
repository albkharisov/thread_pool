/**
 * @file main.cpp
 * @author Albert Kharisov <albkharisov@gmail.com>
 * @version 1.0
 *
 * @brief Main entry point of the program
 *
 * Start reading standard input by 3 and send it to the worker pool
 * to process solving in parallel. Run standalone printer thread for
 * collecting and printing solutions.
 */

#include <thread>
#include <array>
#include <string>
#include "worker_pool.hpp"
#include "square_solver.hpp"

/**
 * @brief Entry point function
 *
 * Start reading standard input by 3 and send it to the worker pool
 * to process solving in parallel. Run standalone printer thread for
 * collecting and printing result of equation.
 *
 * @param argc Number of arguments
 * @param argv Arguments passed to the program
 * @return Result of program operation
 */
int main(int argc, char* argv[]) {
    // main thread reads cin, printer thread writes to cout,
    // that's why worker pool is nthread-2.
    WorkerPool worker_pool(WorkerPool::nthreads - 2);

    auto printer = std::thread([&worker_pool]{
        for (;;) {
            auto result = worker_pool.get_answer();
            if (!result) { break; }
            // only 1 thread prints, so no need to guard cout
            std::cout << *result << std::endl;
        }
    });

    std::array<std::string, 3> input;
    for (int cnt = 0; std::cin >> input[cnt++]; ) {
        if (cnt == 3) {
            cnt = 0;
            worker_pool.set_job(calculate_square_roots,
                                {std::move(input[0]), std::move(input[1]), std::move(input[2])});
        }
    }

    // stop workers to release printer thread
    worker_pool.stop();
    printer.join();

    return 0;
}

