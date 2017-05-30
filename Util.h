#ifndef UTIL_H
#define UTIL_H

#include <functional>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>

using namespace std;


class Signal;

typedef function<double (double)> Activation;


string toString(int _number);
string toStringd(double _number);

string askFile(string _default, string _description = "Entrez un fichier");
string askFile(string _default, string _description, string _extension);

string getExtension(string file);
string removeExtension(string file);
string setExtension(string file, string _extension);

void removeNoiseParams(Signal& _audioSource);



double clamp(double a, double b, double c);
double random(int _min, int _max);

double heaviside(double _x);
double sigmoid(double _x);
double seuil(double _x);
double tanH(double _x);

double dSigmoid(double _x);
double dtanH(double _x);

class Matrix
{
    public:
        Matrix(unsigned _w = 1, unsigned _h = 1);

		Matrix getTranspose() const;
        Matrix apply(Activation f);

		float length() const;
        void disp() const;

        void setLine(std::vector<double> _values, unsigned _line);
        void setColumn(std::vector<double> _values, unsigned _column);
        void setDiagonal(std::vector<double> _values);

        std::vector<double> getLine(unsigned _line) const;
        std::vector<double>& getColumn(unsigned _column);

        Matrix& operator+=(Matrix a);
        Matrix& operator-=(Matrix a);

        std::vector<double>& operator[](unsigned _column);

        /// Atributes
            unsigned w, h;
            std::vector< std::vector<double> > values; // colonnes, lignes
};

Matrix operator*(const Matrix& a, const Matrix& b);
Matrix operator*(const double& a, const Matrix& b);
Matrix operator+(const Matrix& a, const Matrix& b);
Matrix operator-(const Matrix& a, const Matrix& b);

#endif // UTIL_H
