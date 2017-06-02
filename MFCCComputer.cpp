#include "MFCCComputer.h"
#include "Vector.h"

#include <gsl/gsl_errno.h>
#include <gsl/gsl_fft_complex.h>

#include <limits>

MFCCComputer::MFCCComputer(unsigned _coefCount, unsigned _deltaCount):
    coefCount(_coefCount), deltaCount(_deltaCount),
    useLogEnergy(true)
{
    buildFilterBank(64, 8000);
    buildHammingWindow();
}

MFCCComputer::~MFCCComputer()
{
}

unsigned MFCCComputer::getMFCCsCount()
{
    return mfccsCount;
}

unsigned MFCCComputer::getMFCCsCount(double _time)
{
    int d = _time * 16000 - Nlength;

    if (d < 0)
        return 1;

    return ceil(d / Nstep) +1;
}

void MFCCComputer::getDeltas(const std::vector<Vector>& _src, std::vector<Vector>& _deltas)
{
    unsigned s = _src.size();
    _deltas.resize(s);

    _deltas[0] = 0.5 * (_src[1] - _src[0]);

    for (unsigned i(1) ; i < s-1 ; i++)
        _deltas[i] = 0.5 * (_src[i+1] - _src[i-1]);

    _deltas[s-1] = 0.5 * (_src[s-1] - _src[s-2]);
}

void MFCCComputer::buildFilterBank(double _lower, double _upper)
{
    filterBank.resize(Nfilt, vector<double>(K, 0) );

    vector<double> m(Nfilt+2);

    auto mel  = [] (double _f) { return 1127.0 * log(1.0 + _f/700.0); };
    auto iMel = [] (double _m) { return 700.0 * (exp(_m/1127.0) - 1.0); };

    double lower = mel(_lower);
    double upper = mel(_upper);

    double iS = 1.0 / (m.size()-1);
    for (unsigned i(0) ; i < m.size() ; i++)
    {
        double t = i * iS;
        m[i] = (1.0-t) * lower + t * upper;
        m[i] = iMel(m[i]);
        m[i] = round( 512.0 * m[i] / 16000.0);
    }

    for (unsigned i(1) ; i < m.size()-1 ; i++)
    {
        for (unsigned k(m[i-1]) ; k < m[i]   ; k++)
            filterBank[i-1][k] = (k - m[i-1] + 1) / (m[i] - m[i-1] + 1);

        for (unsigned k(m[i])   ; k <= m[i+1] ; k++)
            filterBank[i-1][k] = (m[i+1] - k + 1) / (m[i+1] - m[i] + 1);
    }
}

void MFCCComputer::buildHammingWindow()
{
    double coef = 2.0*PI / (double)(Nlength-1);
    hamming.resize(Nlength);

    for(unsigned i(0) ; i < Nlength ; i++)
        hamming[i] = 0.54-0.46*cos(coef * i);
}

void MFCCComputer::setCoefs(unsigned _coefCount, unsigned _deltaCount)
{
    coefCount = _coefCount;
    deltaCount = _deltaCount;
}

void MFCCComputer::setSignal(double* _data, unsigned _dataSize, unsigned _sampleRate, bool _denoise)
{
    signal = Signal(_data, _dataSize, _sampleRate, sf::Color::Red);

    if (_sampleRate != 16000)
        signal.resample(16000);

    if (_denoise)
        signal.denoise();

    signal.removeDCOffset();
    signal.preEmphasis();

    mfccsCount = MFCCComputer::getMFCCsCount(signal.getDuration());
}

void MFCCComputer::setSignal(std::string _file, bool _denoise)
{
    signal.loadFromFile(_file);

    if (signal.sampleRate != 16000)
        signal.resample(16000);

    if (_denoise)
        signal.denoise();

    signal.removeDCOffset();
    signal.preEmphasis();

    mfccsCount = MFCCComputer::getMFCCsCount(signal.getDuration());
}

void MFCCComputer::computeMFCCs(std::vector<Vector>& _result)
{
    _result.resize(mfccsCount);

    for (unsigned i(0) ; i < mfccsCount ; i++)
        computeMFCC(_result[i], i);


    vector<Vector> d1, d2;

    MFCCComputer::getDeltas(_result, d1);
    MFCCComputer::getDeltas(d1, d2);

    append(_result, d1);
    append(_result, d2);
}

void MFCCComputer::computeMFCC(Vector& _result, unsigned _frame)
{
    const std::vector<double>& s = signal.data;

    _result.data.resize(coefCount);

    const unsigned firstPoint = Nstep * _frame;
    const unsigned sup = min(Nlength, s.size() - firstPoint);

    double data[Nfft];
    double fft[2*Nfft];


    /// Fenetrage de Hamming
        for (unsigned i(0) ; i < sup ; i++)
            data[i] = s[firstPoint+i] * hamming[i];

    /// Zero padding
        for (unsigned i(sup) ; i < Nfft ; i++)
            data[i] = 0.0;

    /// FFT de longueur 512
        for (unsigned i(0) ; i < Nfft ; i++)
        {
            fft[2*i+0] = data[i];
            fft[2*i+1] = 0.0;
        }

        gsl_fft_complex_radix2_forward (fft, 1, Nfft);

    /// Densite spectrale de puissance
        for (unsigned k(0) ; k < K ; k++)
            data[k] = (fft[2*k]*fft[2*k] + fft[2*k+1]*fft[2*k+1]) * iNfft;

    /// Application de la banque de filtres
        vector<double> logFilterOutput(Nfilt, 0);

        for (unsigned i(0) ; i < filterBank.size() ; i++) // [0, Nfilt[
        {
            for (unsigned j(0) ; j < filterBank[i].size() ; j++) // [0, K[
                logFilterOutput[i] += filterBank[i][j] * data[j];

            logFilterOutput[i] = logFilterOutput[i] < 2e-22 ? -50.0 : log(logFilterOutput[i]);
        }

    /// Calcul de la DCT
        for (unsigned k(0) ; k < coefCount ; k++)
        {
            for (unsigned n(0) ; n < Nfilt ; n++)
                _result[k] += logFilterOutput[n] * cos(PI * k * ((double)n + 0.5) * iNfilt);
        }

    /// Energie
        if (useLogEnergy)
        {
            double energy = 0.0;

            for (unsigned i(0) ; i < sup ; i++)
                energy += s[firstPoint+i] * s[firstPoint+i];

            _result[0] = energy < 2e-22 ? -50.0 : log(energy);
        }
}
