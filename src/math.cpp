#include "math.hpp"

#include <cmath>
#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace mathutils {

// ── Helpers ────────────────────────────────────────────────────────────────

static void require_nonempty(const std::vector<double>& v)
{
	if (v.empty())
		throw std::invalid_argument("mathutils: vector must not be empty");
}

// ── Basic arithmetic ────────────────────────────────────────────────────────

double add(double a, double b)
{
	return a + b;
}

double subtract(double a, double b)
{
	return a - b;
}

double multiply(double a, double b)
{
	return a * b;
}

double divide(double a, double b)
{
	if (b == 0.0)
		throw std::domain_error("mathutils::divide: division by zero");
	return a / b;
}

// ── Powers and roots ────────────────────────────────────────────────────────

double pow(double base, double exponent)
{
	return std::pow(base, exponent);
}

double sqrt(double x)
{
	if (x < 0.0)
		throw std::domain_error("mathutils::sqrt: negative argument");
	return std::sqrt(x);
}

double cbrt(double x)
{
	return std::cbrt(x);
}

// ── Trigonometry ────────────────────────────────────────────────────────────

double sin(double x)  { return std::sin(x); }
double cos(double x)  { return std::cos(x); }
double tan(double x)  { return std::tan(x); }

double asin(double x)
{
	if (x < -1.0 || x > 1.0)
		throw std::domain_error("mathutils::asin: argument out of [-1, 1]");
	return std::asin(x);
}

double acos(double x)
{
	if (x < -1.0 || x > 1.0)
		throw std::domain_error("mathutils::acos: argument out of [-1, 1]");
	return std::acos(x);
}

double atan(double x)         { return std::atan(x); }
double atan2(double y, double x) { return std::atan2(y, x); }

// ── Logarithms / exponential ────────────────────────────────────────────────

double log(double x)
{
	if (x <= 0.0)
		throw std::domain_error("mathutils::log: argument must be > 0");
	return std::log(x);
}

double log2(double x)
{
	if (x <= 0.0)
		throw std::domain_error("mathutils::log2: argument must be > 0");
	return std::log2(x);
}

double log10(double x)
{
	if (x <= 0.0)
		throw std::domain_error("mathutils::log10: argument must be > 0");
	return std::log10(x);
}

double exp(double x) { return std::exp(x); }

// ── Aggregates ──────────────────────────────────────────────────────────────

double min(const std::vector<double>& v)
{
	require_nonempty(v);
	return *std::min_element(v.begin(), v.end());
}

double max(const std::vector<double>& v)
{
	require_nonempty(v);
	return *std::max_element(v.begin(), v.end());
}

double sum(const std::vector<double>& v)
{
	require_nonempty(v);
	return std::accumulate(v.begin(), v.end(), 0.0);
}

double mean(const std::vector<double>& v)
{
	require_nonempty(v);
	return sum(v) / static_cast<double>(v.size());
}

double median(std::vector<double> v)
{
	require_nonempty(v);
	std::size_t n = v.size();
	std::sort(v.begin(), v.end());
	if (n % 2 == 1)
		return v[n / 2];
	return (v[n / 2 - 1] + v[n / 2]) / 2.0;
}

double stddev(const std::vector<double>& v)
{
	require_nonempty(v);
	double m = mean(v);
	double variance = 0.0;
	for (double x : v) {
		double diff = x - m;
		variance += diff * diff;
	}
	// Population standard deviation.
	return std::sqrt(variance / static_cast<double>(v.size()));
}

// ── Rounding ────────────────────────────────────────────────────────────────

double floor(double x) { return std::floor(x); }
double ceil(double x)  { return std::ceil(x); }
double round(double x) { return std::round(x); }
double trunc(double x) { return std::trunc(x); }

} // namespace mathutils
