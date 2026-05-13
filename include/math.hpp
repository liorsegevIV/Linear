#pragma once

#include <vector>
#include <stdexcept>

namespace mathutils {

	// ── Constants ──────────────────────────────────────────────────────────────
	constexpr double PI = 3.14159265358979323846;
	constexpr double E  = 2.71828182845904523536;

	// ── Basic arithmetic ───────────────────────────────────────────────────────
	double add(double a, double b);
	double subtract(double a, double b);
	double multiply(double a, double b);
	// Throws std::domain_error on division by zero.
	double divide(double a, double b);

	// ── Powers and roots ───────────────────────────────────────────────────────
	double pow(double base, double exponent);
	// Throws std::domain_error for negative x.
	double sqrt(double x);
	double cbrt(double x);

	// ── Trigonometry (radians) ─────────────────────────────────────────────────
	double sin(double x);
	double cos(double x);
	double tan(double x);
	// Throws std::domain_error when |x| > 1.
	double asin(double x);
	// Throws std::domain_error when |x| > 1.
	double acos(double x);
	double atan(double x);
	double atan2(double y, double x);

	// ── Logarithms / exponential ───────────────────────────────────────────────
	// Throws std::domain_error for x <= 0.
	double log(double x);
	double log2(double x);
	double log10(double x);
	double exp(double x);

	// ── Aggregate functions (non-empty vectors) ────────────────────────────────
	// All throw std::invalid_argument on an empty vector.
	double min(const std::vector<double>& v);
	double max(const std::vector<double>& v);
	double sum(const std::vector<double>& v);
	double mean(const std::vector<double>& v);
	// Takes v by value because the implementation sorts a copy.
	double median(std::vector<double> v);
	double stddev(const std::vector<double>& v);

	// ── Rounding ───────────────────────────────────────────────────────────────
	double floor(double x);
	double ceil(double x);
	double round(double x);
	double trunc(double x);

} // namespace mathutils
