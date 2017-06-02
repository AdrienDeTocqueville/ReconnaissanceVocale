#ifndef VITERBI_H_INCLUDED
#define VITERBI_H_INCLUDED

#include <vector>
#include <iostream>

using namespace std;


class Fichiers;
extern Fichiers fichiers;


void viterbi(const vector<vector<double>>& neuralNetworkOutput, vector<int>& retour);

void reduction(vector<int>& listePhonemes);

#endif // VITERBI_H_INCLUDED
