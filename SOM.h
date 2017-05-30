#ifndef SOM_H
#define SOM_H

#include <SFML/Graphics.hpp>

#include "Util.h"
#include <vector>

using namespace std;

class Vector;

struct Database
{
    Database(unsigned _size = 0, unsigned _inputSize = 0, double _la = 0.1, double _lb = 0.001, double _ra = 5, double _rb = 0.001);

    bool loadFromFile(string _file);
    void saveToFile(string _file);

    Vector& operator[](unsigned _index) { return db[_index]; }
    void computeCoefs();

    unsigned size();
    unsigned inputSize();


    vector<Vector> db;
    double la, lb,  dL;
    double ra, rb,  dR;

    unsigned step, maxSteps;
    double iMaxSteps;

    vector<unsigned> permutation;

};

typedef pair<unsigned, unsigned> Node;

class SOM
{
    public:
        SOM();
        SOM(unsigned _width, unsigned _height, unsigned _inputSize, unsigned _clusterCount);

        ~SOM();

        /// Methods
            void randomize();

            void initTraining();
            bool epoch();

            bool loadFromFile(string _file);
            void saveToFile(string _file);

            void sortProbas();

        /// Getters
            bool getTrained() const;

            Node getBMU(Vector _input) const;
            Vector run(Vector _input) const;

            vector<vector<double>> getUMatrix4(bool _verbose = false) const; // Blanc = similarité
            vector<vector<double>> getUMatrix8(bool _verbose = false) const; // Blanc = similarité

            Vector& getWeight(Node _n);
            unsigned getIndex(Node _n) const;

            unsigned getPhoneme(unsigned i, unsigned j, unsigned _rank);

        /// Setter
            void setDatabase(string _file);

        /// Attributes (public)
            unsigned w, h;
            unsigned inputSize;

            vector<vector<Vector>> grid;
            vector<vector<Vector>> probas;
            vector<vector< vector<unsigned> >> ordreProbas;

            vector<sf::Color> couleurs;
            vector<string> labels;

            // Training utils
                Database db;
                double lr, rad;
};

#endif // SOM_H
