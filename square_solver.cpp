/**
 * @file square_solver.cpp
 * @author Albert Kharisov <albkharisov@gmail.com>
 * @version 1.0
 *
 * @brief Square equations solver functions
 */

#include <stdexcept>
#include <cmath>
#include "square_solver.hpp"


std::string calculate_square_roots(std::string a_str, std::string b_str, std::string c_str) {
    std::string answer;
    answer.resize(a_str.size() + b_str.size() + c_str.size() + 10);
    auto size = std::snprintf(answer.data(), answer.size(), "(%s %s %s) => ", a_str.c_str(), b_str.c_str(), c_str.c_str());
    answer.resize(size);

    char format_buf[30];
    int a, b, c;

    try {
        a = std::stoi(a_str);
        b = std::stoi(b_str);
        c = std::stoi(c_str);
    } catch (std::invalid_argument&) {
        return answer.append("invalid argument");
    } catch (std::out_of_range&) {
        return answer.append("out of range");
    }

    const double epsilon = 1e-7;
    auto decorate_float = [epsilon](double num) -> double {
        return (std::abs(num) < epsilon) ? 0 : num;
    };

    if (a == 0) {
        // linear equation
        if (b == 0) {
            // any X is a solution of equation
            answer.append("(x ∈ R)");
        } else if (c == 0) {
            answer.append("(0)");
        } else {
            auto x = static_cast<double>(b) / c;
            std::snprintf(format_buf, sizeof(format_buf), "(%.6g)", decorate_float(x));
            answer.append(format_buf);
        }
    } else {
        // square equation
        // epsilon is here to fix precision loss during floating point operations
        double d = static_cast<double>(b) * b - 4. * a * c;
        if (d < -epsilon) {
            answer.append("no roots");
        } else if (std::abs(d) < epsilon) {
            double x = static_cast<double>(-b) / (2. * a);
            std::snprintf(format_buf, sizeof(format_buf), "(%.6g)", decorate_float(x));
            answer.append(format_buf);
        } else {
            auto d_sqrt = std::sqrt(d);
            int b_sign = b < 0 ? -1 : 1;
            auto temp = -0.5 * (b + b_sign * d_sqrt);
            auto x1 = c / temp;
            auto x2 = temp / a;
            std::snprintf(format_buf, sizeof(format_buf), "(%.6g %.6g)", decorate_float(x1), decorate_float(x2));
            answer.append(format_buf);
        }

        double x_extremum = static_cast<double>(-b) / (2. * a);
        std::snprintf(format_buf, sizeof(format_buf), " X%s=%.6g", (a > 0) ? "min" : "max", decorate_float(x_extremum));
        answer.append(format_buf);
    }

    return answer;
}

