#ifndef MFCCCOMPUTER_H
#define MFCCCOMPUTER_H

#include "Signal.h"

const unsigned Nfft = 512, Nfilt = 23;

const double windowLength = 0.025, windowStep = 0.01;
const unsigned Nlength = windowLength*16000, Nstep = windowStep*16000;

const unsigned K = Nfft*0.5 + 1;
const double iNfft = 1.0 / (double)Nfft, iNfilt = 1.0 / (double)Nfilt;


class Vector;
class MFCCComputer
{
    typedef vector< vector<double> > Filterbank;

    public:
        MFCCComputer(unsigned _coefCount, unsigned _deltaCount);
        ~MFCCComputer();

        unsigned getMFCCsCount();

        static unsigned getMFCCsCount(double _time);
        static void getDeltas(const std::vector<Vector>& _src, std::vector<Vector>& _deltas);


        void buildFilterBank(double _lower, double _upper);
        void buildHammingWindow();

        void setCoefs(unsigned _coefCount, unsigned _deltaCount);

        void setSignal(double* _data, unsigned _dataSize, unsigned _sampleRate, bool _denoise);
        void setSignal(std::string _file, bool _denoise);


        void computeMFCCs(::vector<Vector>& _result);
        void computeMFCC(Vector& _result, unsigned _frame);

//    private:
        Signal signal;

        unsigned mfccsCount;

        unsigned coefCount;
        unsigned deltaCount;

        bool useLogEnergy;

        Filterbank filterBank;
        vector<double> hamming;
};

#endif // MFCCCOMPUTER_H
