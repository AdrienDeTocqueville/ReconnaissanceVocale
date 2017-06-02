#include "Reconstitution.h"

#include "Reconnaissance.h"

#include <fstream>

void wordfindGregoire(vector<vector<string> >& retour,vector<int> PhonemList)
{
    ifstream Flux("Data/dictionnaire.txt");
    if (Flux)
    {
        vector<pair<vector<string>,vector<int> > > Path;
        string buf;
        vector<vector<string> > ligne;
        while (getline(Flux,buf))
        {
            ligne.push_back(ssplit(buf,','));
        }
        Flux.close();
        pair<vector<string>,vector<int> > temp;
        temp.first = {""};
        temp.second = PhonemList;
        Path.push_back(temp);

        for (int j(0);j<(signed)Path.size();j++)
        {
            unsigned n = 0;
            for (unsigned l(0);l<ligne.size();l++)
            {
                if (Path[j].second.size() == 0)
                    break;

                if (n>(ligne[l].size()-2))
                {
                    Path[j].second.erase(Path[j].second.begin());
                    j -=1;
                    break;
                }

                if (toInt(ligne[l][n+1]) > Path[j].second[n])
                {

                    Path[j].second.erase(Path[j].second.begin());
                    j -=1;
                    break;
                }
                while (toInt(ligne[l][n+1]) == Path[j].second[n])
                {
                    n += 1;
                    if ((ligne[l].size() == n+1))
                    {
                        vector<string> temp1 = Path[j].first;
                        temp1.push_back(ligne[l][0]);
                        vector<int> temp2;
//                        cout <<  n << " " << Path[j].second.size() << " " << PhonemList.size() << endl;
                        for (unsigned i(PhonemList.size()+n-Path[j].second.size());i<PhonemList.size();i++)
                        {
                            temp2.push_back(PhonemList[i]);
                        }
                        Path.push_back(make_pair(temp1,temp2));

                        if (ligne[l].size() < n+2)
                            break;
                    }
                }

            }
        }
        Path.erase(Path.begin());
        for (unsigned i(0);i<Path.size();i++)
        {
            Path[i].first.erase(Path[i].first.begin());
            retour.push_back(Path[i].first);
        }
    }
    else
    {
        cout << "impossible d'ouvrir leximp3.txt" << endl;
        Flux.close();
    }
}


vector<vector<Homophones>> wordFind(vector<int>& phonemList)
{
    vector<vector<Homophones>> retour;
    unsigned departRecherche = 0;

    for (unsigned i(0) ; i < phonemList.size() ; i++)
    {
        vector<int> debut(phonemList.begin(), phonemList.begin()+i+1);
        vector<int> reste(phonemList.begin()+i+1, phonemList.end());

        Homophones mots = chercherMot(debut, departRecherche);


        vector<vector<Homophones>> wl = wordFind(reste);

        if (wl.empty())
        {
            if (mots.empty())
                continue;

            retour.push_back(vector<Homophones>(1, mots));
        }
        else
        {
            for (unsigned j(0) ; j < wl.size() ; j++)
            {
                if (!mots.empty())
                    retour.push_back(vector<Homophones>(1, mots));
                else
                    retour.push_back(vector<Homophones>());

                retour.back().reserve(wl[j].size()+1);


                for (unsigned k(0) ; k < wl[j].size() ; k++)
                {
                    retour.back().push_back(wl[j][k]);
                }
            }
        }
    }

    return retour;
}

void convertir1(vector<int> a)
{
    vector<string> f = {" ","I","ET","AI","A","AU","O","OU","U","E","EU","IN","AN","ON","Y","WI","UI","P","T","K","B","D","G","F","S","CH","V","Z","J","L","R","M","N"};
    for (unsigned i(0) ; i < a.size() ; i++)
        cout << f[a[i]] << " ";
}

Homophones chercherMot(vector<int>& phonemList, unsigned& i)
{
    const Dictionnaire& dico = fichiers.dictionnaire;

    if (!i) i = dico.flags[phonemList[0]];

    unsigned s = phonemList.size();

    for (; i < dico.flags[phonemList[0]+1] ; i++)
    {
        if (dico.transcriptions[i].size() != s)
            continue;

        int diff = 0;
        for (unsigned n(0); n < s; n++)
        {
            if (dico.transcriptions[i][n] < phonemList[n])
                diff = 1;
            if (dico.transcriptions[i][n] > phonemList[n])
                diff = 2;

            if (diff)
                break;
        }
        if (diff == 0)
            return dico.mots[i];

        else if (diff == 2)
            break;
    }

    i = dico.flags[phonemList[0]];

    return {};
}

void obtenirSequences(vector<vector<string>>& phrase, const vector<vector<Homophones>>& sequence)
{
    unsigned l = 0;

    for (unsigned j(0) ; j < sequence.size() ; j++)
    {
        const Homophones& mots = sequence[j][0];

        for (unsigned k(0) ; k < mots.size() ; k++)
        {
            if (phrase.size() < l+1)
                phrase.resize(l+1);

            if (sequence[j].size() == 1)
            {
                phrase[l].push_back(mots[k]);
                l++;
            }
            else
            {
                for (unsigned i(1);i<sequence[j].size();i++)
                {
                    const Homophones& mots2 = sequence[j][i];

                    for (unsigned p(0) ; p < mots2.size() ; p++)
                    {
                        if (phrase.size() < l+1)
                            phrase.resize(l+1);

                        phrase[l].push_back(mots[k]);
                        phrase[l].push_back(mots2[p]);
                        l++;
                    }
                }
            }
        }
    }
}

void sequenceVide(vector<vector<string> >& Sequence)
{
    cout << "a";
    for (unsigned i(0);i<Sequence.size();i++)
    {
        cout << i << "  " << Sequence.size() << endl;
        double proba= 0;
        if (Sequence[i].size() <6)
        {
            proba = getNGramProba(Sequence[i]);
        }
        else
        {
            for (unsigned j(0);j<Sequence[i].size();j+=5)
            {
                 vector<string> copie(Sequence[i].begin()+j, Sequence[i].begin()+j+5);
                 proba *= getNGramProba(copie);
            }
        }
        cout << i << "  " << proba << "  " << Sequence[i][0] << endl;
        if (!proba)
        {
            Sequence.erase(Sequence.begin()+i);
            i -=1;
        }
        else
            cout << Sequence[i][0] << endl;
    }
}

void mostProbableSequence(vector<vector<string> >& phrase,const vector<vector<string> >& Sequence)
{
    int l =0;
    for (unsigned j(0);j<Sequence.size();j++)
    {
        vector<string> temp = ssplit(Sequence[j][0],'/');
        for (unsigned k(0);k<temp.size();k++)
        {
            if (phrase.size() < l+1)
                phrase.resize(l+1);
            if (Sequence[j].size() == 1)
            {
                phrase[l].push_back(temp[k]);
                l+=1;
            }
            else
            {
                for (unsigned i(1);i<Sequence[j].size();i++)
                {
                    vector<string> temp2 = ssplit(Sequence[j][i],'/');
                    for (unsigned p(0);p<temp2.size();p++)
                    {
                        if (phrase.size() < l+1)
                        phrase.resize(l+1);
                        phrase[l].push_back(temp[k]);
                        phrase[l].push_back(temp2[p]);
                        l+=1;
                    }
                }
            }
        }
    }
}

