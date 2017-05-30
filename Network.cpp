#include "Network.h"

#include <algorithm>

#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>

namespace rna
{

/// LAYER CLASS

Layer::Layer(Activation _activation, Activation _derivative, unsigned _neuronCount, unsigned _inputSize):
    neuronCount(_neuronCount),
    weights(_inputSize, _neuronCount), bias(1, _neuronCount),
    input(1, _neuronCount), h(1, _neuronCount), output(1, _neuronCount),
    f(_activation), df(_derivative)
{ }

void Layer::randomize()
{
    for (unsigned i(0) ; i < weights.h ; i++)
    {
        bias[0][i] = random(0, 2) - 1.0;

        for (unsigned j(0) ; j < weights.w ; j++)
            weights[j][i] = random(0, 2) - 1.0;
    }
}

void Layer::feedForward(Matrix& _input)
{
    input = _input;
    h = weights * input - bias;
    output = h.apply(f);
}

Matrix Layer::getError(Matrix _error)
{
    Matrix S(neuronCount, neuronCount);
    S.setDiagonal(h.apply(df).getColumn(0));

    return S * _error;
}

/// NETWORK CLASS

Network::Network(unsigned _inputSize, std::vector<unsigned> _layers):
    inputSize(_inputSize), outputSize(_layers.back())
{
    srand(time(NULL));

    if (_layers.empty())
    {
        std::cout << "Unable to create an empty network" << std::endl;
        return;
    }

    layers.reserve(_layers.size());

    // First layer
    layers.push_back( Layer(sigmoid, dSigmoid, _layers[0], inputSize) );

    // Other layers
    for (unsigned i(1) ; i < _layers.size() ; i++)
        layers.push_back( Layer(sigmoid, dSigmoid, _layers[i], _layers[i-1]) );
}

Network::Network(unsigned _inputSize, std::vector<unsigned> _layers, std::vector<Activation> _activations, std::vector<Activation> _derivatives):
    inputSize(_inputSize), outputSize(_layers.back())
{
    srand(time(NULL));

    if (_layers.empty())
    {
        std::cout << "Unable to create an empty network" << std::endl;
        return;
    }

    layers.reserve(_layers.size());

    // First layer
    layers.push_back( Layer(_activations[0], _derivatives[0], _layers[0], inputSize) );

    // Other layers
    for (unsigned i(1) ; i < _layers.size() ; i++)
        layers.push_back( Layer(_activations[i], _derivatives[i], _layers[i], _layers[i-1]) );
}

Network::~Network()
{ }

TrainData Network::loadData(std::string _data)
{
    TrainData training;

    std::ifstream data(_data.c_str());

    if (!data)
    {
        std::cout << "Unable to open file: " << _data << std::endl;
        return training;
    }

    std::istringstream stream;

    std::string line;
    unsigned i;

    while (getline(data, line))
    {
        if (line.size() <= 1)
            continue;

        if (line[0] == '/' && line[1] == '/')
            continue;

        stream.clear();
        stream.str(line);

        // Load example
            std::pair<Matrix, Matrix> ex = std::make_pair( Matrix(1, inputSize), Matrix(1, outputSize) );

            for (i = 0 ; i < inputSize ; i++)
                stream >> ex.first[0][i];

            for (i = 0 ; i < outputSize ; i++)
                stream >> ex.second[0][i];

        // Save it
            training.examples.push_back(ex);
    }

    return training;
}

void Network::train(TrainData _data, unsigned _maxEpochs, double _desiredError, double _learningRate)
{
    for (Layer& layer: layers)
        layer.randomize();

    auto examples = _data.examples;
    unsigned k = 0;
    float err;

    float lambda = _learningRate;
    float iS = 1.0 / examples.size();

    do
    {
        std::random_shuffle(examples.begin(), examples.end());

        k++;
        bool report = (k%20 == 0);

        err = 0.0;

        for (auto& example: examples)
        {
            Matrix calculation(1, outputSize);
            Data output = feedForward( example.first.getColumn(0) );

            calculation.setColumn( output, 0 );

            Matrix error = example.second - calculation;
            err += error.length();


            std::vector<Matrix> errors(layers.size());

            // Correct last layer
                Layer& last = layers.back();
                error = 2.0 * last.getError(error);
                errors.back() = error;

            // Recursivly correct other layers
                for (int i(layers.size()-2) ; i >= 0 ; i--)
                {
                    error = layers[i].getError(layers[i+1].weights.getTranspose() * error);
                    errors[i] = error;
                }

            for (unsigned i(0) ; i < layers.size() ; i++)
            {
                layers[i].weights += lambda * errors[i] * layers[i].input.getTranspose();
                layers[i].bias    += lambda * errors[i];
            }
        }

        err *= iS;

        if (report)
        {
            std::cout << "Error: " << err << std::endl;
            report = false;
        }

//        lambda = 10.0 * _learningRate * _desiredError / err;

    } while (err > _desiredError && k < _maxEpochs);

    std::cout << "Convergence en : " << k << " iterations" << std::endl;

    // uniformise
//    double x = 1 / layers.back().weights[0][0];
//
//    for (unsigned i(0) ; i < layers.back().weights.w ; i++)
//    for (unsigned j(0) ; j < layers.back().weights.h ; j++)
//    layers.back().weights[i][j] *= x;
//
//    for (unsigned j(0) ; j < layers.back().bias.h ; j++)
//    layers.back().bias[0][j] *= x;
}

Data Network::feedForward(Data _input)
{
    if (_input.size() != inputSize)
    {
        std::cout << "Input size not valid" << std::endl;
        return Data();
    }

    Matrix i(1, inputSize);
    i.setColumn(_input, 0);

    // Fill first layer neurons
        layers.front().feedForward(i);

    // Compute other layers
        for (unsigned i(1) ; i < layers.size() ; i++)
            layers[i].feedForward(layers[i-1].output);

    // get last layer
        return layers.back().output.getColumn(0);
}

bool Network::loadFromFile(string _file)
{
    std::ifstream file(_file.c_str());

    if (!file)
    {
        std::cout << "Fichier introuvable: " << _file << std::endl;
        return false;
    }

    // Read header
        unsigned layerCount;
        vector<unsigned> _layers;

        file >> inputSize >> outputSize >> layerCount;

        _layers.resize(layerCount);
        layers.reserve(layerCount);

        // First layer
        file >> _layers[0];
        layers.push_back( Layer(sigmoid, dSigmoid, _layers[0], inputSize) );

        // Other layers
        for (unsigned i(1) ; i < _layers.size() ; i++)
        {
            file >> _layers[i];
            layers.push_back( Layer(sigmoid, dSigmoid, _layers[i], _layers[i-1]) );
        }


    // Read layers data
        for (unsigned i(0) ; i < layers.size() ; i++)
        {
            for (unsigned y(0) ; y < layers[i].neuronCount ; y++)
            {
                for (unsigned x(0) ; x < layers[i].weights.w ; x++)
                    file >> layers[i].weights[x][y];

                file >> layers[i].bias[0][y];
            }
        }

    return true;
}

void Network::saveToFile(string _file)
{
    if (getExtension(_file) != ".som")
    {
        cout << "Mauvais format de fichier: " << _file << endl;
        return;
    }

    std::ofstream file(_file.c_str(), ios::out | ios::trunc);

    // Write header
        file << inputSize << "  " << outputSize << "  " << layers.size() << '\n' << layers[0].neuronCount;
        for (unsigned i(1) ; i < layers.size() ; i++)
            file << "  " << layers[i].neuronCount;

    // Write layers data
        for (unsigned i(0) ; i < layers.size() ; i++)
        {
            file << endl;

            for (unsigned y(0) ; y < layers[i].neuronCount ; y++)
            {
                file << endl;

                for (unsigned x(0) ; x < layers[i].weights.w ; x++)
                    file << layers[i].weights[x][y] << "   ";

                file << layers[i].bias[0][y];
            }
        }
}

}
