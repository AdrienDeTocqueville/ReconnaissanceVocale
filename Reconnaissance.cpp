#include "Reconnaissance.h"

#include "Reconstitution.h"
#include "Viterbi.h"
#include "Vector.h"
#include "Graph.h"

#include "windows.h"

#include <thread>
#include <atomic>
#include <fstream>
#include <algorithm>

Fichiers fichiers;


// Cette fonction ne fait que enregistrer le signal capté dans un fichier (via Recorder)
void reconnaissanceVocale(SOM& som, MFCCComputer& computer)
{
    if (!sf::SoundBufferRecorder::isAvailable())
    {
        std::cout << "Aucun microphone detecte" << std::endl;
        system("pause");
        return;
    }
    vector<Node> somOutput; somOutput.reserve(1000);
    vector<vector<Node>> partitions;
    unsigned lastMFCC = 0;


    Recorder recorder(computer);
    recorder.start();
    sf::sleep(sf::seconds(2.0));


    cout << "Appyer sur espace pour arreter" << endl;


//    std::atomic_bool done(false);
//    thread transcriptionThread (transcription, partitions, ref(done));


    while ( !GetAsyncKeyState(VK_SPACE) )
    {
        if (recorder.getTimeBeforeUpdate() == 0.0)
        {
            /// TODO: reparer le calcul des MFCCs en temps reel
            recorder.computeAvailableMFCCs();

            for (unsigned i(lastMFCC) ; i < recorder.mfccs.size() ; i++)
                somOutput.push_back( som.getBMU(recorder.mfccs[i]) );

            lastMFCC = recorder.mfccs.size();
        }
    }


    recorder.stop();

//    done = true;
//    transcriptionThread.join();

    system("pause");
}


void part(SOM& som, vector<vector<Node>>& retour ,vector<Node>& NeuralNetworkOutput)
{
    int k = 0;
    int t = 0;
    for (unsigned i(0) ; i < NeuralNetworkOutput.size() ; i++)
    {
        const Node& bmu = NeuralNetworkOutput[i];

        if (som.probas[bmu.first][bmu.second][0] == 0)
            k = 0;

        else if (k++ == 20)
        {
            if ((int)i != t+20)
                retour.emplace_back(NeuralNetworkOutput.begin()+t, NeuralNetworkOutput.begin()+i-20);

            t = i+1;
            k = 0;
        }
    }

    NeuralNetworkOutput.erase(NeuralNetworkOutput.begin(), NeuralNetworkOutput.begin()+t);
}

void partitionnerOutput(SOM& som, vector<vector<Node>>& partitions, vector<Node>& somOutput)
{
    unsigned consecutiveSpaces = 0;
    unsigned lastPartition = 0;


    for (unsigned i(0) ; i < somOutput.size() ; i++)
    {
        const Node& bmu = somOutput[i];

        if (som.probas[bmu.first][bmu.second][0] != 0.0)
            consecutiveSpaces++;

        else if (consecutiveSpaces && i+1 < som.db.size())
        {
            const Node& nextBmu = somOutput[i+1];

            if (som.probas[nextBmu.first][nextBmu.second][0] != 0.0)
                consecutiveSpaces++;
            else
            {
                if (consecutiveSpaces > 15)
                {
                    partitions.push_back( vector<Node>(somOutput.begin()+lastPartition, somOutput.begin()+i) );
                    lastPartition = i;
                }

                consecutiveSpaces = 0;
            }
        }
    }

    somOutput.erase( somOutput.begin(), somOutput.begin()+lastPartition );
}

//void transcription(const vector<vector<Node>>& partitions, atomic_bool& done)
//{
//    while (!done)
//    {
//        cout << partitions.size() << endl;
//        unsigned lastSize = 0;
//
//        if (partitions.size() != lastSize)
//        {
//            lastSize = partitions.size();
//            cout << "Nouvelle taille: " << lastSize << endl;
//
//            transcrirePartition(/* ARGS */);
//        }
//    }
//}

void transcrirePartition(SOM& som, const vector<Node>& _partition, bool _verbose)
{
    vector<int> resultat;
    vector<vector<double>> listeProbaPhonemes;

    for (unsigned i(0) ; i < _partition.size() ; i++)
        listeProbaPhonemes.push_back( som.probas[_partition[i].first][_partition[i].second].data );


    viterbi(listeProbaPhonemes, resultat);

    if (_verbose)
        convertir(resultat);


    reduction(resultat);

    if (_verbose)
        cout << "-> ";

    convertir(resultat);


    if (resultat.empty())
        return;

    vector<vector<Homophones>> ecritures = wordFind(resultat);

    cout << ": ";

    if (ecritures.size() && ecritures.back().size())
        cout << ecritures.back()[0][0] << " ";

    cout << endl;

//    for (unsigned i(0) ; i < ecritures.size(); i++)
//    {
//        for (unsigned j(0) ; j < ecritures[i].size(); j++)
//            for (unsigned k(0) ; k < ecritures[i][j].size(); k++)
//                cout << ecritures[i][j][k] << "/";
//
//        cout << endl;
//    }
//    cout << endl;

//    vector<vector<string>> ecritures2; wordfindGregoire(ecritures2, resultat);
//    vector<vector<string>> phrase;
//    mostProbableSequence(phrase, ecritures2);
////    sequenceVide(phrase);
//
//    for (unsigned i(0) ; i < phrase.size(); i++)
//    {
//        for (unsigned j(0) ; j < phrase[i].size(); j++)
//            cout << phrase[i][j] << "  ";
//
//        cout << endl;
//    }
//
//    cout << endl;
}

void tester_wordfinds()
{
    vector<int> b = {20,13,28,7,30};

    vector<vector<Homophones>> s = wordFind(b);

    cout << "Adrien: " << s.size() << " possibilites" << endl;
    for (unsigned i(0) ; i < s.size() ; i++)
    {
        for (unsigned j(0) ; j < s[i].size() ; j++)
        {
            for (unsigned k(0) ; k < s[i][j].size() ; k++)
                cout << s[i][j][k] << "/";

            cout << " ";
        }

        cout << endl;
    }

    vector<vector<string>> s2;
    wordfindGregoire(s2, b);

    cout << "Gregoire: " << s2.size() << " possibilites" << endl;
    for (unsigned i(0) ; i < s2.size() ; i++)
    {
        for (unsigned j(0) ; j < s2[i].size() ; j++)
            cout << s2[i][j] << " - ";

        cout << endl;
    }

    system("pause");
}

void convertir(vector<int> a)
{
    vector<string> f = {" ","I","ET","AI","A","AU","O","OU","U","E","EU","IN","AN","ON","Y","WI","UI","P","T","K","B","D","G","F","S","CH","V","Z","J","L","R","M","N"};
    for (unsigned i(0) ; i < a.size() ; i++)
        cout << f[a[i]] << " ";

    cout << endl;
}


Fichiers::Fichiers()
{
    chargerDictionnaire();
    chargerInitialisation();
    chargerLongueurPhonemes();
    chargerPhonemTransitionMatrix();
}

void Fichiers::chargerDictionnaire()
{
    ifstream fichier("Data/dictionnaire.txt");
    if (!fichier)
    {
        cout << "Fichier introuvable: Data/dictionnaire.txt" << endl;
        return;
    }

    dictionnaire.flags.reserve(33);
    dictionnaire.mots.reserve(71196);
    dictionnaire.transcriptions.reserve(71196);

    dictionnaire.flags.push_back(0);

    int n = 1;
    string ligne;
    while (getline(fichier, ligne))
    {
        auto splits = ssplit(ligne, ',');
        dictionnaire.mots.push_back(ssplit(splits[0], '/'));

        dictionnaire.transcriptions.push_back(vector<int>(splits.size()-1));
        for (unsigned i(1) ; i < splits.size() ; i++)
            dictionnaire.transcriptions.back()[i-1] = toInt(splits[i]);


        while (dictionnaire.transcriptions.back()[0] > n)
        {
            n++;
            dictionnaire.flags.push_back(dictionnaire.mots.size()-1);
        }


        if (dictionnaire.transcriptions.back()[0] == n)
        {
            n++;
            dictionnaire.flags.push_back(dictionnaire.mots.size()-1);
        }
    }

    dictionnaire.flags.push_back(dictionnaire.mots.size());
}

void Fichiers::chargerInitialisation()
{
    ifstream fichier("Data/phonemInitialisationProba.txt");
    if (!fichier)
    {
        cout << "Fichier introuvable: Data/phonemInitialisationProba.txt" << endl;
        return;
    }

    initialisation.resize(33, 0);

    for (unsigned i(0) ; i < 33 ; i++)
        fichier >> initialisation[i];
}

void Fichiers::chargerLongueurPhonemes()
{
    ifstream fichier("Data/phonemLenght.txt");
    if (!fichier)
    {
        cout << "Fichier introuvable: Data/phonemLenght.txt" << endl;
        return;
    }

    longueursPhonemes.resize(33, pair<int, int>(0, 0));

    for (unsigned i(0) ; i < 33 ; i++)
        fichier >> longueursPhonemes[i].first;

    for (unsigned i(0) ; i < 33 ; i++)
        fichier >> longueursPhonemes[i].second;
}

void Fichiers::chargerPhonemTransitionMatrix()
{
    ifstream fichier("Data/phonemTransitionMatrix.txt");
    if (!fichier)
    {
        cout << "Fichier introuvable: Data/phonemTransitionMatrix.txt" << endl;
        return;
    }

    phonemTransitionMatrix.resize(33, vector<double>(33, 0));

    for (unsigned i(0) ; i < 33 ; i++)
        for (unsigned j(0) ; j < 33 ; j++)
            fichier >> phonemTransitionMatrix[i][j];
}
