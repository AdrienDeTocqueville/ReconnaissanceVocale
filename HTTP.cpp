#include "HTTP.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cctype>

#include <windows.h>

string percentEncoding(string value)
{
    ostringstream escaped;
    escaped.fill('0');
    escaped << hex;

    for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i)
    {
        string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '=' || c == '&' || c == '.' || c == '*' || c == '/' || c == ':' || c == '?' || c == '-' || c == '_' || c == '~')
        {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << uppercase;
        escaped << '%' << setw(2) << int((unsigned char) c);
        escaped << nouppercase;
    }

    return escaped.str();
}

double getNGramProba(vector<string> mots)
{
    string suite(mots[0]);
    for (unsigned i(1) ; i < mots.size() ; i++)
        suite += "+" + mots[i];

    string URL = "https://books.google.com/ngrams/graph?content=" + suite + "&case_insensitive=on&year_start=2000&year_end=2008&corpus=19&smoothing=4";

    string command = "GnuWin32\\bin\\wget.exe -q -O ngram.html --no-check-certificate \"" + URL + "\" > nul 2>&1";

    system(command.c_str());

    ifstream file("ngram.html");
    string line;

    if (!file)
    {
        cout << "Site non accessible" << endl;
        return 0.0;
    }

    while (getline(file, line))
    {
        if (line.size() < 2)
            continue;

        if (line[2] != 'v')
            continue;
        if (line.substr(2, 8) != "var data")
            continue;

        if (line.substr(13, 2) == "[]") // Aucune donnée
            return 0.0;

        if (line[27 + suite.size()] == ',') // Si il n'y a pas de resultats avec majuscules
            line = line.substr(60 + suite.size(), line.size()-1);
        else
            line = line.substr(77 + suite.size(), line.size()-1);

        istringstream iss(line);
        string temp;   double proba;

        iss >> temp >> temp >> temp >> temp >> proba; // On recupere la valeur de l'annee 2004 qui est la moyenne entre 2000 et 2008 (plus representatif du language actuel)

        return proba;
    }

    cout << "Donnéees non accessibles" << endl;

    return 0.0;
}

void accessLexique(string searchTerm)
{
    const string first = "http://www.lexique.org/moteur/complexe.php?regex=0&nom_champs[1]=lexique3.phon&champs[1]=";
    const string second = "&tri=lexique3.freqlemfilms2&sens=DESC&pers[0]=lexique3.phon&pers[1]=lexique3.freqlemfilms2&pers[2]=lexique3.cgram&pers[3]=lexique3.ortho&database=lexique.lexique3&Rechercher=Rechercher&nblignes=500&format=texte";

    string URL = first + percentEncoding(searchTerm) + second;

    cout << URL << endl;

    string command = "GnuWin32\\bin\\wget.exe -q -O lexique.xml --no-check-certificate \"" + URL + "\" > nul 2>&1";

    system(command.c_str());

//    ifstream file("ngram.html");
//    string line;
//
//    while (getline(file, line))
}
