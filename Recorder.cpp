#include "Recorder.h"
#include "MFCCComputer.h"
#include "Vector.h"

#include <iostream>
#include "SOM.h"

Recorder::Recorder(MFCCComputer& _computer):
    computer(_computer)
{ }

Recorder::~Recorder()
{
    stop();
}

bool Recorder::onStart()
{
    rate = getSampleRate();
    setChannelCount(1);

    samples.reserve(10000);

    forceUpdate = false;
    t = 1;

    return true;
}

void Recorder::onStop()
{
    if (!samples.empty())
    {
        forceUpdate = true;
        computeAvailableMFCCs();
    }

    Signal signal = Signal(samples, rate, sf::Color::Red);

    if (rate != 16000)
        signal.resample(16000);

    signal.saveToFile("Database/ReVo.wav");
    Database db;    db.db = mfccs;
    db.saveToFile("ReVo.db");

    std::cout << "Duree: " << getDuration() << std::endl;
}

bool Recorder::onProcessSamples(const sf::Int16* _samples, std::size_t _sampleCount)
{
    unsigned lastSize = samples.size();

    samples.resize(lastSize + _sampleCount);
    for (unsigned i(0) ; i < _sampleCount ; i ++)
        samples[lastSize + i] = (double)_samples[i];

    return true;
}

void Recorder::calibrate(double _time)
{
    sf::sleep(sf::seconds(1.0));
    return;

    double spent;
    while ((spent = getDuration()) < _time)
        sf::sleep(sf::seconds(_time-spent));


    Signal sig(samples, rate, sf::Color::Red);
    sig.resample(16000);
    sig.initDenoising(_time, 0.0);

    samples.clear();
}

double Recorder::computeAvailableMFCCs()
{
    if (!forceUpdate)
    {
        double left = 5.0 * t - getDuration();
        if (left > 0.0)
            return left;

        t++;
    }

    computer.setSignal(samples, rate, false);

    computer.computeMFCCs(mfccs);

    std::cout << mfccs.size() << "   " << getDuration() << std::endl;

    return 0.0;
}

double Recorder::getDuration() const
{
    return samples.size() / rate;
}
