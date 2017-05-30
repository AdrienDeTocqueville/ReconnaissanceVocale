#include "Vector.h"

double dRand(double dMin, double dMax)
{
    double d = (double)rand() / RAND_MAX;
    return dMin + d * (dMax - dMin);
}

int iRand(int iMin, int iMax)
{
    return iMin + (rand() % (int)(iMax - iMin + 1));
}

Vector::Vector(unsigned _size)
{
    data.resize(_size);
}

Vector::Vector(unsigned _size, unsigned _value)
{
    data.resize(_size, _value);
}

Vector::Vector(std::initializer_list<double> l):
    data(l)
{ }

Vector::~Vector()
{ }

void Vector::append(const Vector& _b)
{
    unsigned oldSize = data.size();
    data.resize(oldSize + _b.size());

    for (unsigned i(0) ; i < _b.size() ; i++)
        data[oldSize + i] = _b.data[i];
}

void Vector::disp() const
{
    std::cout << "(" << data[0];
    for (unsigned i(1) ; i < data.size() ; i++)
        std::cout << ", " << data[i];

    std::cout << ")" << std::endl;
}

unsigned Vector::size() const
{
    return data.size();
}

double Vector::length() const
{
    return sqrt(length2());
}

double Vector::length2() const
{
    double sum = 0.0f;

    for (unsigned i(0) ; i < data.size() ; i++)
        sum += data[i]*data[i];

    return sum;
}

Vector Vector::getUnit() const
{
    Vector c(data.size());

    double il = 1.0 / sqrt(length2());

    for (unsigned i(0) ; i < data.size() ; i++)
        c[i] = data[i] * il;

    return c;
}

void Vector::normalize()
{
    double il = 1.0 / sqrt(length2());

    for (unsigned i(0) ; i < data.size() ; i++)
        data[i] *= il;
}

void Vector::scale()
{
    double maxVal = DBL_MIN;

    for (unsigned i(0) ; i < data.size() ; i++)
    {
        if (data[i] > maxVal)
            maxVal = data[i];
    }

    for (unsigned i(0) ; i < data.size() ; i++)
        data[i] *= maxVal;
}

void Vector::randomize(double _min, double _max)
{
    for (unsigned i(0) ; i < data.size() ; i++)
        data[i] = dRand(_min, _max);
}

double& Vector::operator[](unsigned _index)
{
    return data[_index];
}

Vector& Vector::operator+=(Vector a)
{
    for (unsigned i(0) ; i < data.size() ; i++)
        data[i] += a.data[i];

    return *this;
}

Vector& Vector::operator*=(const double& a)
{
    for (unsigned i(0) ; i < data.size() ; i++)
        data[i] *= a;

    return *this;
}

bool operator==(const Vector& a, const Vector& b)
{
    if (a.size() != b.size())
        std::cout << "Vector::operator== -> les tailles ne correspondent pas" << std::endl;

    for (unsigned i(0) ; i < b.size() ; i++)
        if (a.data[i] != b.data[i])
            return false;

    return true;
}

Vector operator+(const Vector& a, const Vector& b)
{
    if (a.size() != b.size())
        std::cout << "Vector::operator+ -> les tailles ne correspondent pas" << std::endl;

    Vector result(b.size());

    for (unsigned i(0) ; i < b.size() ; i++)
        result[i] = a.data[i] + b.data[i];

    return result;
}

Vector operator-(const Vector& a, const Vector& b)
{
    if (a.size() != b.size())
        std::cout << "Vector::operator- -> les tailles ne correspondent pas" << std::endl;

    Vector result(b.size());

    for (unsigned i(0) ; i < b.size() ; i++)
        result[i] = a.data[i] - b.data[i];

    return result;
}

Vector operator*(const double& a, const Vector& b)
{
    Vector result(b.size());

    for (unsigned i(0) ; i < b.size() ; i++)
        result[i] = a * b.data[i];

    return result;
}

void append(std::vector<Vector>& a, const std::vector<Vector>& b)
{
    for (unsigned i(0) ; i < a.size() ; i++)
        a[i].append(b[i]);
}
