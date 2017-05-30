#ifndef HTTP_H_INCLUDED
#define HTTP_H_INCLUDED

#include <vector>
#include <string>

using namespace std;

string percentEncoding(string src);

double getNGramProba(vector<string> mots);

void accessLexique(string searchTerm);

#endif // HTTP_H_INCLUDED
