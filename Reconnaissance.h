#ifndef RECONNAISSANCE_H_INCLUDED
#define RECONNAISSANCE_H_INCLUDED

#include "SOM.h"
#include "HTTP.h"
#include "Recorder.h"
#include "MFCCComputer.h"

typedef vector<string> Homophones;
struct Dictionnaire
{
    vector<unsigned> flags;

    vector<Homophones> mots;
    vector<vector<int>> transcriptions;
};

class Fichiers
{
    public:
        Fichiers();

        void chargerDictionnaire();
        void chargerInitialisation();
        void chargerLongueurPhonemes();
        void chargerPhonemTransitionMatrix();

        Dictionnaire dictionnaire;
        vector<double> initialisation;
        vector<pair<int, int>> longueursPhonemes;
        vector<vector<double>> phonemTransitionMatrix;
};

void reconnaissanceVocale(SOM& som, MFCCComputer& computer);

void part(SOM& som, vector<vector<Node>>& retour ,vector<Node>& NeuralNetworkOutput);
void partitionnerOutput(SOM& som, vector<vector<Node>>& partitions, vector<Node>& somOutput);

//void transcription(const vector<vector<Node>>& partitions, atomic_bool& done);

void transcrirePartition(SOM& som, const vector<Node>& _partition, bool _verbose = false);

void tester_wordfinds();
void convertir(vector<int>);

#endif // RECONNAISSANCE_H_INCLUDED
