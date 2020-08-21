#pragma once

inline float isApproximatively(float a, float b, float approx)
{
	return (a + approx) > b && (a - approx) < b;
}