#include <System/Math/Math.h>

int Sign(int value)
{
	if (value > 0) { return 1; }
	if (value < 0) { return -1; }
	return 0;
}

float Sign(float value)
{
	if (value > 0.0f) { return 1.0f; }
	if (value < 0.0f) { return -1.0f; }
	return 0.0f;
}

int Clamp(int value, int min, int max)
{
	if (value <= min) { return min; }
	if (max <= value) { return max; }
	return value;
}

float Clamp(float value, float min, float max)
{
	if (value <= min) { return min; }
	if (max <= value) { return max; }
	return value;
}
