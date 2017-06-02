#ifndef UTIL_H
#define UTIL_H

#include <functional>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>

using namespace std;



int toInt(string _string);
double toDouble(string _string);

string toString(int _number);
string toStringd(double _number);

vector<string> ssplit(const string& _string, char _separation);

string askFile(string _default, string _description = "Entrez un fichier");
string askFile(string _default, string _description, string _extension);

string getExtension(string file);
string removeExtension(string file);
string setExtension(string file, string _extension);


double clamp(double a, double b, double c);
double random(int _min, int _max);


#endif // UTIL_H
