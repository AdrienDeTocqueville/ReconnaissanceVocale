#include "SOM.h"
#include "Vector.h"

#include <thread>

#include <algorithm>
#include <fstream>
#include <sstream>


SOM::SOM():
    w(0), h(0), inputSize(0)
{
    srand(time(NULL));
}

SOM::SOM(unsigned _width, unsigned _height, unsigned _inputSize, unsigned _clusterCount):
    w(_width), h(_height), inputSize(_inputSize)
{
    srand(time(NULL));

    grid.resize(w, vector<Vector>(h, Vector(inputSize, 0.0)));
    probas.resize(w, vector<Vector>(h, Vector(_clusterCount, 0.0)));
    ordreProbas.resize(w, vector<vector<unsigned>>(h));


    couleurs.resize(_clusterCount, sf::Color::Blue);
    labels.resize(_clusterCount, "Bleu");


    randomize();
}

SOM::~SOM()
{ }

void SOM::randomize()
{
    for (unsigned i(0) ; i < w ; i++)
        for (unsigned j(0) ; j < h ; j++)
            grid[i][j].randomize(0, 1);
}

void SOM::initTraining()
{
    db.computeCoefs();

    lr = db.la * pow(db.dL, db.step);
    rad = db.ra * pow(db.dR, db.step);

    if (inputSize != db.inputSize())
        cout << "/!\\ Vector sizes do not match /!\\" << endl;
}

bool SOM::epoch()
{
    if (getTrained())
        return false;

    double sRad = rad * rad;
    double sig = - 1.0 / (2.0*sRad);


    random_shuffle(db.permutation.begin(), db.permutation.end());

    for (unsigned t: db.permutation)
    {
        Node BMU = getBMU(db[t]);

        for (unsigned i(0) ; i < w ; i++)
        {
            for (unsigned j(0) ; j < h ; j++)
            {
                unsigned a = BMU.first - i, b = BMU.second - j;
                double l = a*a + b*b;

                Vector& NWeight = grid[i][j];

                NWeight += lr * exp(l * sig) * ( db[t] - NWeight );
            }
        }
    }


    if (++db.step == db.maxSteps)
    {
        db.step = 0;
        return false;
    }

    lr *= db.dL;
    rad *= db.dR;

    return true;
}

void SOM::sortProbas()
{
    auto sortFunc = [](const pair<unsigned, double>& a, const pair<unsigned, double>& b) { return a.second > b.second; };

    for (unsigned i(0) ; i < w ; i++)
    {
        for (unsigned j(0) ; j < h ; j++)
        {
            vector< pair<unsigned, double> > tri(couleurs.size());

            for (unsigned k(0) ; k < couleurs.size() ; k++)
                tri[k] = pair<unsigned, double>(k, probas[i][j][k]);

            sort(tri.begin(), tri.end(), sortFunc);

            ordreProbas[i][j].clear();

            for (auto& element: tri)
            {
                if (element.second == 0.0)
                    break;

                ordreProbas[i][j].push_back(element.first);
            }
        }
    }
}

bool SOM::getTrained() const
{
    return db.step == db.maxSteps;
}

Node SOM::getBMU(Vector _input) const
{
    double minDist = DBL_MAX;
    Node index = Node(0, 0);

    for (unsigned i(0) ; i < w ; i++)
    {
        for (unsigned j(0) ; j < h ; j++)
        {
            double len2 = (grid[i][j] - _input).length2();
            if (len2 < minDist)
            {
                minDist = len2;
                index = Node(i, j);
            }
        }
    }

    return index;
}

Vector SOM::run(Vector _input) const
{
    Node coord = getBMU(_input);

    return probas[coord.first][coord.second];
}

vector<vector<double>> SOM::getUMatrix4(bool _verbose) const
{
    vector<vector<double>> umatrix(w, vector<double>(h, 0));

    // particulier
    umatrix[0][0] = (grid[1][0]-grid[0][0]).length() + (grid[0][1]-grid[0][0]).length();
    umatrix[0][0] /= 2.0;

    for (unsigned x(1) ; x < w-1 ; x++)
    {
        umatrix[x][0] += (grid[x-1][0]-grid[x][0]).length() + (grid[x+1][0]-grid[x][0]).length();
        umatrix[x][0] += (grid[x][1]-grid[x][0]).length();
        umatrix[x][0] /= 3.0;
    }

    umatrix[w-1][0] += (grid[w-2][0]-grid[w-1][0]).length() + (grid[w-1][1]-grid[w-1][0]).length();
    umatrix[w-1][0] /= 2.0;

    // general
    for (unsigned y(1) ; y < h-1 ; y++)
    {
        umatrix[0][y] += (grid[1][y]-grid[0][y]).length();
        umatrix[0][y] += (grid[0][y-1]-grid[0][y]).length() + (grid[0][y+1]-grid[0][y]).length();
        umatrix[0][y] /= 3.0;

        for (unsigned x(1) ; x < w-1 ; x++)
        {
            umatrix[x][y] += (grid[x-1][y]-grid[x][y]).length() + (grid[x+1][y]-grid[x][y]).length();
            umatrix[x][y] += (grid[x][y-1]-grid[x][y]).length() + (grid[x][y+1]-grid[x][y]).length();

            umatrix[x][y] /= 4.0;
        }

        umatrix[w-1][y] += (grid[w-2][y]-grid[w-1][y]).length();
        umatrix[w-1][y] += (grid[w-1][y-1]-grid[w-1][y]).length() + (grid[w-1][y+1]-grid[w-1][y]).length();
        umatrix[w-1][y] /= 3.0;
    }

    // particulier
    umatrix[0][h-1] = (grid[1][h-1]-grid[0][h-1]).length() + (grid[0][h-2]-grid[0][h-1]).length();
    umatrix[0][h-1] /= 2.0;

    for (unsigned x(1) ; x < w-1 ; x++)
    {
        umatrix[x][h-1] += (grid[x-1][h-1]-grid[x][h-1]).length() + (grid[x+1][h-1]-grid[x][h-1]).length();
        umatrix[x][h-1] += (grid[x][h-2]-grid[x][h-1]).length();
        umatrix[x][h-1] /= 3.0;
    }

    umatrix[w-1][h-1] += (grid[w-2][h-1]-grid[w-1][h-1]).length() + (grid[w-1][h-2]-grid[w-1][h-1]).length();
    umatrix[w-1][h-1] /= 2.0;


    double mi = DBL_MAX, ma = DBL_MIN;
    for (unsigned i(0) ; i < w ; i++)
        for (unsigned j(0) ; j < h ; j++)
        {
            if (_verbose)
                cout << umatrix[i][j] << "   " ;

            mi = min(umatrix[i][j], mi);
            ma = max(umatrix[i][j], ma);
        }

    if (_verbose)
    {
        cout << endl << endl;
        cout << "-----------------    " << mi << "    " << ma << "    -----------------";
        cout << endl << endl;
    }

    double iMax = 1.0 / ( ma - mi);

    for (unsigned i(0) ; i < w ; i++)
        for (unsigned j(0) ; j < h ; j++)
        {
            umatrix[i][j] = 1.0 - (umatrix[i][j]-mi)*iMax;

            if (_verbose)
                cout << umatrix[i][j] << "   " ;
        }

    return umatrix;
}

vector<vector<double>> SOM::getUMatrix8(bool _verbose) const
{
    vector<vector<double>> umatrix(w, vector<double>(h, 0));

    // particulier
    umatrix[0][0] = (grid[1][0]-grid[0][0]).length() + (grid[0][1]-grid[0][0]).length() + (grid[1][1]-grid[0][0]).length();
    umatrix[0][0] /= 3.0;

    for (unsigned x(1) ; x < w-1 ; x++)
    {
        umatrix[x][0] += (grid[x-1][0]-grid[x][0]).length() + (grid[x+1][0]-grid[x][0]).length();
        umatrix[x][0] += (grid[x-1][1]-grid[x][0]).length() + (grid[x+1][1]-grid[x][0]).length();
        umatrix[x][0] += (grid[x][1]-grid[x][0]).length();
        umatrix[x][0] /= 5.0;
    }

    umatrix[w-1][0] += (grid[w-2][0]-grid[w-1][0]).length() + (grid[w-1][1]-grid[w-1][0]).length() + (grid[w-2][1]-grid[w-1][0]).length();
    umatrix[w-1][0] /= 3.0;

    // general
    for (unsigned y(1) ; y < h-1 ; y++)
    {
        umatrix[0][y] += (grid[1][y]-grid[0][y]).length();
        umatrix[0][y] += (grid[1][y-1]-grid[0][y]).length() + (grid[1][y+1]-grid[0][y]).length();
        umatrix[0][y] += (grid[0][y-1]-grid[0][y]).length() + (grid[0][y+1]-grid[0][y]).length();
        umatrix[0][y] /= 5.0;

        for (unsigned x(1) ; x < w-1 ; x++)
        {
            umatrix[x][y] += (grid[x-1][y]-grid[x][y]).length() + (grid[x+1][y]-grid[x][y]).length();
            umatrix[x][y] += (grid[x][y-1]-grid[x][y]).length() + (grid[x][y+1]-grid[x][y]).length();

            umatrix[x][y] += (grid[x-1][y-1]-grid[x][y]).length() + (grid[x+1][y-1]-grid[x][y]).length();
            umatrix[x][y] += (grid[x-1][y+1]-grid[x][y]).length() + (grid[x+1][y+1]-grid[x][y]).length();

            umatrix[x][y] /= 8.0;
        }

        umatrix[w-1][y] += (grid[w-2][y]-grid[w-1][y]).length();
        umatrix[w-1][y] += (grid[w-2][y-1]-grid[w-1][y]).length() + (grid[w-2][y+1]-grid[w-1][y]).length();
        umatrix[w-1][y] += (grid[w-1][y-1]-grid[w-1][y]).length() + (grid[w-1][y+1]-grid[w-1][y]).length();
        umatrix[w-1][y] /= 5.0;
    }

    // particulier
    umatrix[0][h-1] = (grid[1][h-1]-grid[0][h-1]).length() + (grid[0][h-2]-grid[0][h-1]).length() + (grid[1][h-2]-grid[0][h-1]).length();
    umatrix[0][h-1] /= 3.0;

    for (unsigned x(1) ; x < w-1 ; x++)
    {
        umatrix[x][h-1] += (grid[x-1][h-1]-grid[x][h-1]).length() + (grid[x+1][h-1]-grid[x][h-1]).length();
        umatrix[x][h-1] += (grid[x-1][h-2]-grid[x][h-1]).length() + (grid[x+1][h-2]-grid[x][h-1]).length();
        umatrix[x][h-1] += (grid[x][h-2]-grid[x][h-1]).length();
        umatrix[x][h-1] /= 5.0;
    }

    umatrix[w-1][h-1] += (grid[w-2][h-1]-grid[w-1][h-1]).length() + (grid[w-1][h-2]-grid[w-1][h-1]).length() + (grid[w-2][h-2]-grid[w-1][h-1]).length();
    umatrix[w-1][h-1] /= 3.0;


    double mi = DBL_MAX, ma = DBL_MIN;
    for (unsigned i(0) ; i < w ; i++)
        for (unsigned j(0) ; j < h ; j++)
        {
            if (_verbose)
                cout << umatrix[i][j] << "   " ;

            mi = min(umatrix[i][j], mi);
            ma = max(umatrix[i][j], ma);
        }

    if (_verbose)
    {
        cout << endl << endl;
        cout << "-----------------    " << mi << "    " << ma << "    -----------------";
        cout << endl << endl;
    }

    double iMax = 1.0 / ( ma - mi);

    for (unsigned i(0) ; i < w ; i++)
        for (unsigned j(0) ; j < h ; j++)
        {
            umatrix[i][j] = 1.0 - (umatrix[i][j]-mi)*iMax;

            if (_verbose)
                cout << umatrix[i][j] << "   " ;
        }

    return umatrix;
}

Vector& SOM::getWeight(Node _n)
{
    return grid[_n.first][_n.second];
}

unsigned SOM::getIndex(Node _n) const
{
    return _n.first + _n.second*grid.size();
}

unsigned SOM::getPhoneme(unsigned i, unsigned j, unsigned _rank)
{
    if (_rank >= ordreProbas[i][j].size())
        return couleurs.size();

    return ordreProbas[i][j][_rank];
}

void SOM::setDatabase(string _file)
{
    db.loadFromFile(_file);

    if (inputSize != db.inputSize())
        cout << "/!\\ Vector sizes do not match /!\\" << endl;
}

bool SOM::loadFromFile(string _file)
{
    srand(time(NULL));

    std::ifstream file(_file.c_str());

    if (!file)
    {
        std::cout << "Fichier introuvable: " << _file << std::endl;
        return false;
    }

    // Read header
        unsigned clusterCount;
        file >> w >> h >> inputSize >> clusterCount;

        grid.resize(w, vector<Vector>(h, Vector(inputSize, 0.0)));
        probas.resize(w, vector<Vector>(h, Vector(clusterCount, 0.0)));
        ordreProbas.resize(w, vector<vector<unsigned>>(h));
        couleurs.resize(clusterCount);
        labels.resize(clusterCount);


    // Read grid data
        for (unsigned i(0) ; i < w ; i++)
        {
            for (unsigned j(0) ; j < h ; j++)
            {
                Vector& v = grid[i][j];

                for (unsigned k(0) ; k < inputSize ; k++)
                    file >> v[k];
            }
        }

    // Read probas
        for (unsigned i(0) ; i < w ; i++)
        {
            for (unsigned j(0) ; j < h ; j++)
            {
                Vector& v = probas[i][j];

                for (unsigned k(0) ; k < clusterCount ; k++)
                    file >> v[k];
            }
        }
        sortProbas();

    // Read clusters data
        int a, b, c;

        for (unsigned i(0) ; i < clusterCount ; i++)
        {
            file >> a >> b >> c;
                couleurs[i].r = a;
                couleurs[i].g = b;
                couleurs[i].b = c;
        }

        string line;
        getline(file, line);
        getline(file, line);

        for (unsigned i(0) ; i < clusterCount ; i++)
             getline(file, labels[i]);


    return true;
}

void SOM::saveToFile(string _file)
{
    if (getExtension(_file) != ".som")
    {
        cout << "Mauvais format de fichier: " << _file << endl;
        return;
    }

    std::ofstream file(_file.c_str(), ios::out | ios::trunc);

    // Write header
        file << w << "  " << h << "  " << inputSize << "  " << couleurs.size();

    // Write grid data
        for (unsigned i(0) ; i < w ; i++)
        {
            file << endl;

            for (unsigned j(0) ; j < h ; j++)
            {
                file << endl;

                Vector& v = grid[i][j];

                file << v[0];
                for (unsigned k(1) ; k < inputSize ; k++)
                    file << " " << v[k];
            }
        }

    // Write categories
        file << endl;

        for (unsigned i(0) ; i < w ; i++)
        {
            file << endl;

            for (unsigned j(0) ; j < h ; j++)
            {
                file << endl;

                Vector& v = probas[i][j];

                file << v[0];
                for (unsigned k(1) ; k < couleurs.size() ; k++)
                    file << " " << v[k];
            }
        }


    // Write clusters
        file << endl << endl << endl;

        file << (int)couleurs[0].r << " " << (int)couleurs[0].g << " " << (int)couleurs[0].b;
        for (unsigned i(1) ; i < couleurs.size() ; i++)
            file << "   " << (int)couleurs[i].r << " " << (int)couleurs[i].g << " " << (int)couleurs[i].b;


        file << endl;

        for (unsigned i(0) ; i < labels.size(); i++)
             file << endl << labels[i];
}

/// Database
Database::Database(unsigned _size, unsigned _inputSize, double _la, double _lb, double _ra, double _rb):
    la(_la), lb(_lb), ra(_ra), rb(_rb), step(0), maxSteps(0), iMaxSteps(0.0)
{
    db.resize(_size, Vector(_inputSize, 0.0));

    if (la == 0.0)
        la = 0.1;
    if (ra == 0.0)
        ra = 5.0;
}

bool Database::loadFromFile(string _file)
{
    ifstream file(_file);

    if (!file)
    {
        std::cout << "Fichier introuvable: " << _file << std::endl;
        return false;
    }

    istringstream stream;
    unsigned inputSize;
    string line;

    file >> inputSize >> la >> lb >> ra >> rb >> step >> maxSteps;

    computeCoefs();


    db.clear();

    while (getline(file, line))
    {
        if (!line.size())
            continue;

        stream.clear();
        stream.str(line);

        db.push_back(Vector(inputSize, 0.0));

        Vector& v = db.back();

        for (unsigned i(0) ; i < inputSize ; i++)
            stream >> v[i];
    }


    permutation.resize(db.size());
    for (unsigned i(0) ; i < db.size() ; i++)
        permutation[i] = i;

    return true;
}

void Database::saveToFile(string _file)
{
    if (!db.size())
    {
        cout << "La DB est vide" << endl;
        return;
    }

    ofstream file(_file, ios::out | ios::trunc);
    unsigned inputSize = db[0].size();

    file << inputSize << "  " << la  << "  " << lb  << "  " << ra  << "  " << rb  << "  " << step  << "  " << maxSteps << endl;

    for (const Vector& v: db)
    {
        file << endl << v.data[0];
        for (unsigned i(1) ; i < inputSize ; i++)
            file << " " << v.data[i];
    }
}

void Database::computeCoefs()
{
    iMaxSteps = 1.0 / (double)maxSteps;

    dL = pow(lb/la, iMaxSteps);
    dR = pow(rb/ra, iMaxSteps);

    step = min(step, maxSteps);
}

unsigned Database::size()
{
    return db.size();
}

unsigned Database::inputSize()
{
    return db.size()? db[0].size(): 0;
}
