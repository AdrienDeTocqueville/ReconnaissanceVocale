#include "Viterbi.h"

#include "Reconnaissance.h"


void viterbi(const vector<vector<double>>& neuralNetworkOutput, vector<int>& retour)
{
    /// Initialisation
    vector<vector<double> > delta(neuralNetworkOutput.size(),vector<double> (33,0));
    vector<vector<int> > psi(neuralNetworkOutput.size(), vector<int> (33,0));

    for (unsigned int i(0); i < neuralNetworkOutput[0].size();i++)
        delta[0][i] = fichiers.initialisation[i] * neuralNetworkOutput[0][i];


    /// Récurrence
    for (unsigned int t(1) ; t < neuralNetworkOutput.size() ; t++)
    {
        for (unsigned int j(0) ; j < neuralNetworkOutput[t].size() ; j++)
        {
            if (neuralNetworkOutput[t][j])
            {
                for (unsigned int i(0) ; i < neuralNetworkOutput[t-1].size() ; i++)
                {
                    if (delta[t-1][i] * fichiers.phonemTransitionMatrix[i][j] > delta[t][j])
                    {
                        delta[t][j] = delta[t-1][i] * fichiers.phonemTransitionMatrix[i][j];
                        psi[t][j] = i;
                    }
                }
                delta[t][j] *= neuralNetworkOutput[t][j];
            }
        }
    }


    /// Retropropagation
    retour.clear();
    retour.reserve(psi.size());

    int it = 0;
    double buf =0;
    for (unsigned int i(0) ; i < delta[neuralNetworkOutput.size()-1].size() ; i++)
    {
        if (buf < delta[neuralNetworkOutput.size()-1][i])
        {
            buf = delta[neuralNetworkOutput.size()-1][i];
            it = i;
        }
    }
    retour.push_back(it);

    for (int t(neuralNetworkOutput.size()-1) ; t >= 0 ; t--)
        retour.insert(retour.begin(), psi[t][retour[0]]);
}

void reduction(vector<int>& listePhonemes)
{
    vector<int> pos;

    for (unsigned i(0) ; i < listePhonemes.size() ; i++)
    {
        if (listePhonemes[i] == 0)
        {
            pos.push_back(i);
        }
        else
        {
            int k = 0;
            const int& longueurMin = fichiers.longueursPhonemes[listePhonemes[i]].first;
            const int& longueurMax = fichiers.longueursPhonemes[listePhonemes[i]].second;

            for (unsigned j( max((unsigned)0, i-longueurMin) ) ; j < min( i+longueurMin, listePhonemes.size() ) ; j++)
            {
                if (listePhonemes[j] == listePhonemes[i])
                    k++;
            }

            if (k < longueurMin)
                pos.push_back(i);

            else if (k > longueurMax)
                pos.push_back(i);

        }
    }

    for (unsigned i(0) ; i < pos.size() ; i++)
        listePhonemes.erase( listePhonemes.begin() + pos[pos.size()-i-1] );


    for (unsigned i(0) ; i+1 < listePhonemes.size() ; i++)
    {
        while (i+1 < listePhonemes.size() && listePhonemes[i] == listePhonemes[i+1])
        {
            listePhonemes.erase( listePhonemes.begin()+i+1 );
        }
    }

}
