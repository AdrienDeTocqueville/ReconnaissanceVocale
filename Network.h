#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include <fstream>
#include <sstream>

#include "Util.h"

namespace rna
{

typedef std::vector<double> Data;

struct TrainData
{
    std::vector< std::pair<Matrix, Matrix> > examples;
};

struct Layer
{
    Layer(Activation _activation, Activation _derivative, unsigned _neuronCount, unsigned _inputSize);

    void randomize();
    void feedForward(Matrix& _input);
    Matrix getError(Matrix _error);

    unsigned neuronCount;

    Matrix weights;
    Matrix bias;

    Matrix input, h, output;

    Activation f, df;
};

struct Network
{
    Network(unsigned _inputSize, std::vector<unsigned> _layers);
    Network(unsigned _inputSize, std::vector<unsigned> _layers, std::vector<Activation> _activations, std::vector<Activation> _derivatives);
    ~Network();

    bool loadFromFile(string _file);
    void saveToFile(string _file);

    TrainData loadData(std::string _data);
    void train(TrainData _data, unsigned _maxEpochs, double _desiredError = 0.01, double _learningRate = 0.01);
//    void train(TrainData _data, TrainData _model, double _maxError, double _learningRate = 0.01);

    Data feedForward(Data _input);

    std::vector<Layer> layers;

    unsigned inputSize;
    unsigned outputSize;
};

}

#endif // NETWORK_H
