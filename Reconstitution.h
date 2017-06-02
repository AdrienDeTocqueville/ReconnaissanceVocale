#ifndef PTW_H_INCLUDED
#define PTW_H_INCLUDED

#include <vector>
#include <string>
#include <iostream>


using namespace std;

class Fichiers;
extern Fichiers fichiers;

typedef vector<string> Homophones;


void wordfindGregoire(vector<vector<string>>& retour, vector<int> PhonemList);
vector<vector<Homophones>> wordFind(vector<int>& phonemList);

Homophones chercherMot(vector<int>& phonemList, unsigned& i);

void obtenirSequences(vector<vector<string>>& phrase, const vector<vector<Homophones>>& sequence);
void sequenceVide(vector<vector<string>>& Sequence);

void mostProbableSequence(vector<vector<string> >& phrase,const vector<vector<string> >& Sequence);

#endif // PTW_H_INCLUDED
