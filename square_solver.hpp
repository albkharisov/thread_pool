/**
 * @file square_solver.hpp
 * @author Albert Kharisov <albkharisov@gmail.com>
 * @version 1.0
 *
 * @brief Square equations solver function
 */

#pragma once

#include <string>

/**
 * @brief Calculate square roots and extremum (if so)
 * and returns string with an answer.
 *
 * Calculate square roots from 3 strings.
 * Equation looks like:
 *  a*x^2 + b*x + c = 0
 *
 * If a == 0, solve as a linear equation (no extremum provided).
 *
 * Example of result string format:
 * (1 2 0) => (0 -2) Xmin=2
 *
 * @param a_str parameter a in quadratic equation
 * @param b_str parameter b in quadratic equation
 * @param c_str parameter c in quadratic equation
 *
 * @return string with result or an error description
 */
std::string calculate_square_roots(std::string a_str, std::string b_str, std::string c_str);

