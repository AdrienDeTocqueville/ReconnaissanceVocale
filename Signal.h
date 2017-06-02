#ifndef SIGNAL_H
#define SIGNAL_H

#include <iostream>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>

#include <cmath>
#include <complex>

#include "Util.h"

#define PI 3.14159265358979

using namespace std;

class Signal
{
    public:
        Signal(sf::Color _color = sf::Color::Black, unsigned _sampleRate = 1);
        Signal(std::string _audioSource, sf::Color _color = sf::Color::Black);
        Signal(sf::SoundBuffer _buf, sf::Color _color = sf::Color::Black);
        Signal(const std::vector<double>& _data, unsigned _sampleRate, sf::Color _color = sf::Color::Black);
        Signal(double* _data, unsigned _dataSize, unsigned _sampleRate, sf::Color _color = sf::Color::Black);

        ~Signal();

        /// Methods
            bool loadFromFile(std::string _audioSource);

            void saveToFile(std::string _file) const;

            double& operator[] (unsigned i) const;

            double getDuration() const;


            void initDenoising(double _noiseLength = 0.0, double _noiseStart = 2.0);

            void denoise(unsigned _step = 0);

            void resample(unsigned _sampleRate);

            void removeDCOffset(double _coef = 0.999);

            void preEmphasis(double _coef = 0.97);


        /// Attributes
            mutable std::vector<double> data;

            sf::Color color;

            double sampleRate;
};

#endif // SIGNAL_H
