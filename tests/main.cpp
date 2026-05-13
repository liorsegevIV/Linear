#include "math.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

// ── tiny helpers ────────────────────────────────────────────────────────────

static bool approx(double a, double b, double eps = 1e-9)
{
	return std::abs(a - b) < eps;
}

static int passed = 0, failed = 0;

#define CHECK(expr) \
	do { \
		if (expr) { \
			++passed; \
		} else { \
			++failed; \
			std::cerr << "FAIL: " #expr " (line " << __LINE__ << ")\n"; \
		} \
	} while (0)

#define CHECK_THROW(expr, exc) \
	do { \
		try { \
			(expr); \
			++failed; \
			std::cerr << "FAIL (no throw): " #expr " (line " << __LINE__ << ")\n"; \
		} catch (const exc&) { \
			++passed; \
		} \
	} while (0)

// ── tests ───────────────────────────────────────────────────────────────────

static void test_constants()
{
	CHECK(approx(mathutils::PI, 3.14159265358979323846));
	CHECK(approx(mathutils::E,  2.71828182845904523536));
}

static void test_arithmetic()
{
	CHECK(approx(mathutils::add(2.0, 3.0),      5.0));
	CHECK(approx(mathutils::subtract(5.0, 3.0), 2.0));
	CHECK(approx(mathutils::multiply(3.0, 4.0), 12.0));
	CHECK(approx(mathutils::divide(10.0, 4.0),  2.5));
	CHECK_THROW(mathutils::divide(1.0, 0.0), std::domain_error);
}

static void test_powers_roots()
{
	CHECK(approx(mathutils::pow(2.0, 10.0), 1024.0));
	CHECK(approx(mathutils::sqrt(9.0),  3.0));
	CHECK(approx(mathutils::cbrt(27.0), 3.0));
	CHECK_THROW(mathutils::sqrt(-1.0), std::domain_error);
}

static void test_trig()
{
	CHECK(approx(mathutils::sin(0.0), 0.0));
	CHECK(approx(mathutils::cos(0.0), 1.0));
	CHECK(approx(mathutils::tan(0.0), 0.0));
	CHECK(approx(mathutils::asin(0.0), 0.0));
	CHECK(approx(mathutils::acos(1.0), 0.0));
	CHECK(approx(mathutils::atan(0.0), 0.0));
	CHECK(approx(mathutils::atan2(1.0, 1.0), mathutils::PI / 4.0));
	CHECK_THROW(mathutils::asin(2.0),  std::domain_error);
	CHECK_THROW(mathutils::acos(-2.0), std::domain_error);
}

static void test_log_exp()
{
	CHECK(approx(mathutils::log(mathutils::E), 1.0));
	CHECK(approx(mathutils::log2(8.0),  3.0));
	CHECK(approx(mathutils::log10(100.0), 2.0));
	CHECK(approx(mathutils::exp(0.0),   1.0));
	CHECK_THROW(mathutils::log(0.0),   std::domain_error);
	CHECK_THROW(mathutils::log(-1.0),  std::domain_error);
	CHECK_THROW(mathutils::log2(0.0),  std::domain_error);
	CHECK_THROW(mathutils::log10(0.0), std::domain_error);
}

static void test_aggregates()
{
	std::vector<double> v = {3.0, 1.0, 4.0, 1.0, 5.0, 9.0, 2.0, 6.0};

	CHECK(approx(mathutils::min(v),    1.0));
	CHECK(approx(mathutils::max(v),    9.0));
	CHECK(approx(mathutils::sum(v),   31.0));
	CHECK(approx(mathutils::mean(v),   31.0 / 8.0));

	// Median of {1,1,2,3,4,5,6,9} → average of 3 and 4 = 3.5
	CHECK(approx(mathutils::median(v), 3.5));

	std::vector<double> odd = {1.0, 3.0, 2.0};
	// Sorted: {1,2,3}, median = 2
	CHECK(approx(mathutils::median(odd), 2.0));

	// stddev of {2, 4, 4, 4, 5, 5, 7, 9} = 2
	std::vector<double> s = {2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0};
	CHECK(approx(mathutils::stddev(s), 2.0));

	std::vector<double> empty;
	CHECK_THROW(mathutils::min(empty),    std::invalid_argument);
	CHECK_THROW(mathutils::max(empty),    std::invalid_argument);
	CHECK_THROW(mathutils::sum(empty),    std::invalid_argument);
	CHECK_THROW(mathutils::mean(empty),   std::invalid_argument);
	CHECK_THROW(mathutils::median(empty), std::invalid_argument);
	CHECK_THROW(mathutils::stddev(empty), std::invalid_argument);
}

static void test_rounding()
{
	CHECK(approx(mathutils::floor(2.9),  2.0));
	CHECK(approx(mathutils::floor(-2.1),-3.0));
	CHECK(approx(mathutils::ceil(2.1),   3.0));
	CHECK(approx(mathutils::ceil(-2.9), -2.0));
	CHECK(approx(mathutils::round(2.5),  3.0));
	CHECK(approx(mathutils::round(2.4),  2.0));
	CHECK(approx(mathutils::trunc(2.9),  2.0));
	CHECK(approx(mathutils::trunc(-2.9),-2.0));
}

// ── entry point ─────────────────────────────────────────────────────────────

int main()
{
	test_constants();
	test_arithmetic();
	test_powers_roots();
	test_trig();
	test_log_exp();
	test_aggregates();
	test_rounding();

	std::cout << passed << " checks passed";
	if (failed > 0)
		std::cout << ", " << failed << " FAILED";
	std::cout << "\n";
	return failed == 0 ? 0 : 1;
}
