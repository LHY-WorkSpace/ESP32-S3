#ifndef MATHFUN_H
#define MATHFUN_H
#include <math.h>

#define RADTODEG(x) ((x) * 57.295779513082320876798154814105f)
#define DEGTORAD(x) ((x) * 0.01745329251994329576923690768489f)

float FastSin(float x);
float FastCos(float x);

#endif

