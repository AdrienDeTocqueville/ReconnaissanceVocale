#include "Util.h"
#include "Signal.h"
#include <cstdlib>

std::string toString(int _number)
{
    std::stringstream os;
    os << _number;

    return os.str();
}

std::string toStringd(double _number)
{
    std::stringstream os;
    os << _number;

    return os.str();
}

string askFile(string _default, string _description)
{
    cin.ignore();

    string input;

    cout << endl << "Fichier par default: " << _default << endl;
    cout << _description << ": ";   getline (cin, input);

    if (input.empty())  return _default;

    return input;
}

string askFile(string _default, string _description, string _extension)
{
    _default = removeExtension(_default) + _extension;
    return removeExtension(askFile(_default, _description)) + _extension;
}

string getExtension(string file)
{
    size_t pos = file.rfind(".");
    if (pos == std::string::npos)
        return "";

    return file.substr(pos, file.size()-1);
}

string removeExtension(string file)
{
    size_t pos = file.rfind(".");
    if (pos == std::string::npos)
        return file;

    return file.substr(0, pos);
}

string setExtension(string file, string _extension)
{
    return removeExtension(file) + _extension;
}

void removeNoiseParams(Signal& _audioSource)
{
    bool nouveauProfil;
    cout << "Creer un nouveau profil du bruit ? "; cin >> nouveauProfil;

    if (nouveauProfil)
    {
        double s, l;
        cout << "Debut du bruit de reference (en secondes): "; cin >> s;
        cout << "Duree du bruit de reference (en secondes): "; cin >> l;

        _audioSource.initDenoising(s, l);
    }

    _audioSource.denoise();
}



double clamp(double a, double b, double c)
{
    return std::max(std::min(a, c), b);
}

double random(int _min, int _max)
{
    return _min + static_cast <double> (rand()) /( static_cast <double> (RAND_MAX/(_max-_min)));
}

double heaviside(double _x)
{
    if (_x > 0) return 1.0;
    else return 0.0;
}

double sigmoid(double _x)
{
    return 1.0 / ( 1.0 + exp(-_x) );
}

double seuil(double _x)
{
    if (_x > 0.0) return 1.0;
    if (_x == 0.0) return 0.0;

    return -1.0;
}

double tanH(double _x)
{
    return tanh(_x);
}

double dSigmoid(double _x)
{
    double s = sigmoid(_x);
    return s*(1.0 - s);
}

double dtanH(double _x)
{
    float t = tanh(_x);
    return 1.0 - t*t;
}

Matrix::Matrix(unsigned _w, unsigned _h):
    w(_w), h(_h)
{
    values.resize(w, std::vector<double>(h, 0.0));
}

Matrix Matrix::getTranspose() const
{
	Matrix transpose(h, w);

    for (unsigned i(0) ; i < w ; i++)
        for (unsigned j(0) ; j < h ; j++)
			transpose.values[j][i] = values[i][j];

    return transpose;
}

Matrix Matrix::apply(Activation f)
{
    Matrix result(w, h);

    for (unsigned i(0) ; i < w ; i++)
        for (unsigned j(0) ; j < h ; j++)
            result.values[i][j] = f(values[i][j]);

    return result;
}

float Matrix::length() const
{
	float sum = 0;

    for (unsigned i(0) ; i < w ; i++)
        for (unsigned j(0) ; j < h ; j++)
			sum += values[i][j]*values[i][j];

    return sqrt(sum);
}

void Matrix::disp() const
{
    for (unsigned j(0) ; j < h ; j++)
    {
        for (unsigned i(0) ; i < w ; i++)
            std::cout << values[i][j] << "  ";

        std::cout << std::endl;
    }
}

void Matrix::setLine(std::vector<double> _values, unsigned _line)
{
    for (unsigned l(0) ; l < w ; l++)
        values[l][_line] = _values[l];
}

void Matrix::setColumn(std::vector<double> _values, unsigned _column)
{
    values[_column] = _values;
}

void Matrix::setDiagonal(std::vector<double> _values)
{
    for (unsigned i(0) ; i < w ; i++)
        values[i][i] = _values[i];
}

std::vector<double> Matrix::getLine(unsigned _line) const
{
    std::vector<double> line(w);

    for (unsigned l(0) ; l < w ; l++)
        line[l] = values[l][_line];

    return line;
}

std::vector<double>& Matrix::getColumn(unsigned _column)
{
    return values[_column];
}

Matrix& Matrix::operator+=(Matrix a)
{
    for (unsigned i(0) ; i < a.w ; i++)
        for (unsigned j(0) ; j < a.h ; j++)
            values[i][j] += a.values[i][j];

    return *this;
}

Matrix& Matrix::operator-=(Matrix a)
{
    for (unsigned i(0) ; i < a.w ; i++)
        for (unsigned j(0) ; j < a.h ; j++)
            values[i][j] -= a.values[i][j];

    return *this;
}

Matrix operator*(const Matrix& a, const Matrix& b)
{
    Matrix result(b.w, a.h);

    for (unsigned i(0) ; i < b.w ; i++)
        for (unsigned j(0) ; j < a.h ; j++)
            for (unsigned k(0) ; k < a.w ; k++)
                result.values[i][j] += a.values[k][j] * b.values[i][k];

    return result;
}

Matrix operator*(const double& a, const Matrix& m)
{
    Matrix result(m.w, m.h);

    for (unsigned i(0) ; i < m.w ; i++)
        for (unsigned j(0) ; j < m.h ; j++)
            result.values[i][j] = a * m.values[i][j];

    return result;
}

Matrix operator+(const Matrix& a, const Matrix& b)
{
    Matrix result(a.w, b.h);

    for (unsigned i(0) ; i < a.w ; i++)
        for (unsigned j(0) ; j < a.h ; j++)
            result.values[i][j] = a.values[i][j] + b.values[i][j];

    return result;
}

Matrix operator-(const Matrix& a, const Matrix& b)
{
    Matrix result(a.w, a.h);

    for (unsigned i(0) ; i < a.w ; i++)
        for (unsigned j(0) ; j < a.h ; j++)
            result.values[i][j] = a.values[i][j] - b.values[i][j];

    return result;
}

std::vector<double>& Matrix::operator[](unsigned _column)
{
    return values[_column];
}

