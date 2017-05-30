#ifndef VECTOR_H
#define VECTOR_H

#include <iostream>
#include <vector>
#include <initializer_list>

#include <stdlib.h>
#include <ctime>
#include <cfloat>
#include <cmath>

double dRand(double dMin, double dMax);
int iRand(int iMin, int iMax);

class Vector
{
    public:
        Vector(unsigned _size = 0);
        Vector(unsigned _size, unsigned _value);
        Vector(std::initializer_list<double> l);

        ~Vector();

        /// METHODS
            void append(const Vector& _b);
            void disp() const;

            unsigned size() const;
            double length() const;
            double length2() const;

            Vector getUnit() const;
            void normalize();
            void scale();

            void randomize(double _min, double _max);

            double& operator[](unsigned _index);
            Vector& operator+=(Vector a);
            Vector& operator*=(const double& a);


        /// Attributes
            std::vector<double> data;

};

bool operator==(const Vector& a, const Vector& b);

Vector operator+(const Vector& a, const Vector& b);
Vector operator-(const Vector& a, const Vector& b);

Vector operator*(const double& a, const Vector& b);

void append(std::vector<Vector>& a, const std::vector<Vector>& b);

#endif // VECTOR_H
