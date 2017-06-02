#ifndef RECORDER_H
#define RECORDER_H

#include <SFML/Audio.hpp>
#include "Signal.h"

class Vector;
class MFCCComputer;
class Recorder : public sf::SoundRecorder
{
    public:
        Recorder(MFCCComputer& _computer);
        virtual ~Recorder();

        virtual bool onStart();
        virtual bool onProcessSamples(const sf::Int16* _samples, std::size_t _sampleCount);
        virtual void onStop();

        void calibrate(double _time);
        double getTimeBeforeUpdate();
        void computeAvailableMFCCs();

        double getDuration() const;

//    private:
        MFCCComputer& computer;
        std::vector<double> samples;
        std::vector<Vector> mfccs;

        double rate;
        unsigned t;
        unsigned lastSampleCount;
};

#endif // RECORDER_H
