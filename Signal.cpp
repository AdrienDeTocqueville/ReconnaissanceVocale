#include "Signal.h"


Signal::Signal(sf::Color _color, unsigned _sampleRate):
    color(_color), sampleRate(_sampleRate)
{ }

Signal::Signal(std::string _audioSource, sf::Color _color):
    Signal(_color)
{
    loadFromFile(_audioSource);
}

Signal::Signal(sf::SoundBuffer _buf, sf::Color _color):
    Signal(_color)
{
    unsigned s = _buf.getSampleCount();
    unsigned c = _buf.getChannelCount();

    const sf::Int16* samples = _buf.getSamples();

    data.resize( s/c );

    sampleRate = _buf.getSampleRate()*c;

    if (sampleRate != 16000)
    {
        std::cout << "Le signal n'est pas frequence a 16 kHz mais a: " << sampleRate << std::endl;
        std::cout << std::endl;
    }


    for (unsigned i(0) ; i < data.size() ; i++)
        data[i] = samples[i*c];
}

Signal::Signal(const std::vector<double>& _data, unsigned _sampleRate, sf::Color _color):
    Signal(_color, _sampleRate)
{
    data = _data;
}

Signal::~Signal()
{ }

/// Methods
bool Signal::loadFromFile(std::string _audioSource)
{
    sf::SoundBuffer buf;
        if (!buf.loadFromFile(_audioSource))
            return false;

    unsigned s = buf.getSampleCount();
    unsigned c = buf.getChannelCount();

    const sf::Int16* samples = buf.getSamples();

    data.resize( s/c );

    sampleRate = buf.getSampleRate()*c;

    if (sampleRate != 16000)
    {
        std::cout << _audioSource << std::endl;
        std::cout << "Le signal n'est pas frequence a 16 kHz mais a: " << sampleRate << std::endl;
        std::cout << std::endl;
    }


    for (unsigned i(0) ; i < data.size() ; i++)
        data[i] = samples[i*c];

    return true;
}

void Signal::saveToFile(std::string _file) const
{
    sf::Int16* samples = new sf::Int16[data.size()];
    for (unsigned i(0) ; i < data.size() ; i++)
        samples[i] = (sf::Int16)data[i];

    sf::SoundBuffer buf;
        buf.loadFromSamples(samples, data.size(), 1, sampleRate);

    buf.saveToFile(_file);

    delete [] samples;
}

double& Signal::operator[] (unsigned i) const
{
    return data[i];
}

double Signal::getDuration() const
{
    return data.size() / sampleRate;
}

void Signal::initDenoising(double _noiseLength, double _noiseStart)
{
    std::string command =   "SoX\\sox.exe SoX\\noise.wav -n trim " +
                            toString(_noiseStart) + " " + toString(_noiseStart+_noiseLength) +
                            " noiseprof SoX\\noise.prof";

    saveToFile("SoX/noise.wav");
    system(command.c_str());
}

void Signal::denoise(unsigned _step)
{
    string command = "SoX\\sox.exe SoX\\noise.wav SoX\\clean.wav noisered SoX\\noise.prof 0.21";

    if (_step == 0)
          _step = data.size();

    unsigned step = _step;
    unsigned iMax = data.size() / _step;

    sf::Int16* samples = new sf::Int16[_step];


    for (unsigned i(0) ; i <= iMax ; i++)
    {
        if (i == iMax)
            step = data.size() - iMax * _step;

        if (step == 0)
            break;

        // Create a portion of audio file
        for (unsigned j(0) ; j < step ; j++)
            samples[j] = (sf::Int16)data[i*_step + j];

        sf::SoundBuffer buf;
            buf.loadFromSamples(samples, step, 1, sampleRate);

        buf.saveToFile("SoX/noise.wav");

        // Denoise the file using SoX
        system(command.c_str());

        // Load cleaned audio
        if (!buf.loadFromFile("SoX/clean.wav"))
        {
            std::cout << std::endl << "Unable to load file" << std::endl;
            continue;
        }

        const sf::Int16* cleanSamples = buf.getSamples();

        for (unsigned j(0) ; j < step ; j++)
        data[i*_step+j] = cleanSamples[j];
    }

    delete [] samples;
}

void Signal::resample(unsigned _sampleRate)
{
    std::string command =   "SoX\\sox.exe SoX\\oldRate.wav SoX\\newRate.wav rate " + toString(_sampleRate);

    saveToFile("SoX/oldRate.wav");
    system(command.c_str());

    loadFromFile("SoX/newRate.wav");
}

void Signal::removeDCOffset(double _coef)
{
    vector<double> data_of(data.size());

    data_of[0] = data[0];

    for (unsigned i(1) ; i < data.size() ; i++)
        data_of[i] = data[i] - data[i-1] + _coef * data_of[i-1];

    data = data_of;
}

void Signal::preEmphasis(double _coef)
{
    vector<double> data_pe(data.size());

    data_pe[0] = data[0];

    for (unsigned i(1) ; i < data.size() ; i++)
        data_pe[i] = data[i] - _coef * data[i-1];

    data = data_pe;
}
