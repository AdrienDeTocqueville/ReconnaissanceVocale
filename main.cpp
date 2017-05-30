#include "MFCCComputer.h"
#include "Recorder.h"
#include "Vector.h"
#include "Graph.h"
#include "HTTP.h"
#include "SOM.h"

#include "floatfann.c"

#define _WIN32_WINNT 0x0502
#define WINVER 0x0500
#include <windows.h>

#include <fstream>
#include <algorithm>

string fileName = "doubleFinal";

void traiterArguments(int argc, char* argv[], SOM& som);

void entrainerSOM(SOM& som);
void classerSOM(SOM& som, const vector<Vector>& _mfccs, string _file);

void classerRNA(SOM& som, string _output, const vector<Vector>& _mfccs, string _file);

void createFANNData(const SOM& _som, string _file = "FANNdata.txt", string _tr = "rna.tr");

int main(int argc, char* argv[])
{
    SetConsoleTitle("Reconnaissance vocale");

    SOM som;
    struct fann *ann = NULL;

    MFCCComputer computer(13, 2);

	unsigned input = 0;

    if (argc > 1)
        traiterArguments(argc, argv, som);
    else
    {
        som.loadFromFile("revo.som");
        som.setDatabase("doubleFinal.db");
    }

	while (1)
	{
		#ifdef DEBUG
            cout << "DEBUG MODE" << endl << endl;
		#endif // DEBUG

        cout << "1. Creer SOM" << endl;
		cout << "2. Charger SOM" << endl;
		cout << "3. Entrainer SOM" << endl;
		cout << "4. Enregistrer SOM" << endl << endl;

		cout << "5. Charger DB" << endl;
		cout << "6. Construire DB" << endl << endl;

		cout << "7. Afficher U-Matrix" << endl;
		cout << "8. Classer SOM" << endl;
		cout << "9. Tester DB avec le SOM" << endl << endl;

		cout << "10. Reduire le bruit d'un signal" << endl << endl;

		cout << "11. Reconnaissance vocale" << endl << endl;

        cout << "12. Creer MLP" << endl;
		cout << "13. Charger MLP" << endl;
		cout << "14. Entrainer MLP" << endl;
		cout << "15. Enregistrer MLP" << endl << endl;

		cout << "16. Tester DB avec le MLP" << endl << endl;

		cout << "17. Generer transcription" << endl << endl;

		cout << "18. Modifier transcription" << endl << endl;

		cout << "19. Generer categories SOM" << endl << endl;

		cout << "20. Exporter resultats SOM" << endl << endl;

		cout << endl << "0. Quitter" << endl << endl;

		cout << "Dimensions DB: " << som.db.size() << " x " << som.db.inputSize() << endl;
		cout << "Dimensions SOM: " << som.w << " x " << som.h << " x " << som.inputSize << endl;

		cout << "Choix: ";
        cin >> input;
		system("cls");

		switch (input)
		{
            case 0:
                fann_destroy(ann);
                return EXIT_SUCCESS;
            break;

			case 1: {
			    unsigned w, h, d, c;
			    cout << "Largeur: "; cin >> w;
			    cout << "Hauteur: "; cin >> h;
			    cout << "Profondeur: "; cin >> d;
			    cout << "Categories: "; cin >> c;

                som = SOM(w, h, d, c);

                system("pause");
			} break;

			case 2:
			    fileName = askFile(fileName, "Charger SOM depuis", ".som");
                som.loadFromFile(fileName);
			break;

			case 3: /// ENTRAINEMENT
                entrainerSOM(som);
            break;

			case 4:
			    fileName = askFile(fileName, "Enregistrer SOM dans", ".som");
                som.saveToFile(fileName);
			break;

			case 5:
			    fileName = askFile(fileName, "Charger DB depuis", ".db");
                som.setDatabase(fileName);

                system("pause");
			break;

			case 6: { /// CONSTRUCTION
                bool remNoise;
                unsigned mfccCount, deltaCount;
                string nom;

			    nom = askFile(fileName, "Fichier audio source", ".wav");
                cout << "Reduire le bruit ? "; cin >> remNoise;
                cout << "Taille des MFCC (Aide: 13): "; cin >> mfccCount;
                cout << "Nombre de deltas (Aide: 2): "; cin >> deltaCount;
			    fileName = askFile(nom, "Enregistrer DB dans", ".db");


                computer.setCoefs(mfccCount, deltaCount);
                computer.setSignal("Database/" + nom, remNoise);

                Database DB;
                computer.computeMFCCs(DB.db);

                DB.saveToFile(fileName);
            } break;

			case 7: { /// U-MATRIX

			    vector<vector<double>> um;
			    unsigned uS;
			    cout << "Nombre de voisins (4/8): "; cin >> uS;
			    if (uS == 4)
                    um = som.getUMatrix4(true);
			    else if (uS == 8)
                    um = som.getUMatrix8(true);
                else
                    break;

                unsigned squareSize = 16, space = squareSize+1;

                sf::RenderWindow app(sf::VideoMode(som.w*space, som.h*space), "U-Matrix " + toString(uS));
                sf::RectangleShape rect(sf::Vector2f(squareSize, squareSize));

                app.clear();

                for (unsigned i(0) ; i < som.w ; i++)
                {
                    for (unsigned j(0) ; j < som.h ; j++)
                    {
                        unsigned c = 255 * um[i][j];
                        rect.setFillColor( sf::Color(c, c, c) );
                        rect.setPosition(i*space, j*space);

                        app.draw(rect);
                    }
                }

                // Update the window
                app.display();

                while (app.isOpen())
                {
                    sf::Event event;
                    while (app.pollEvent(event))
                        if (event.type == sf::Event::Closed)
                            app.close();
                }

			} break;

			case 8: /// CREATION IMAGE
                fileName = askFile(fileName, "Fichier audio de la DB", ".wav");

                if (!som.w)
                    som.loadFromFile(removeExtension(fileName) + ".som");
                if (!som.db.size())
                    som.setDatabase(removeExtension(fileName) + ".db");

			    classerSOM(som, som.db.db, "Database/" + fileName);
            break;

			case 9: { /// Tester DB
                Signal resultats[] = { Signal(sf::Color::Red), Signal(sf::Color::Green), Signal(sf::Color::Blue) };


			    vector<Node> outputs(som.db.size());
			    for (unsigned i(0) ; i < outputs.size() ; i++)
                    outputs[i] = som.getBMU(som.db[i]);

                cout << "Outputs: " << outputs.size() << endl;


                unsigned resCount;
                cout << "Nombre de resultats par output: ";    cin >> resCount;
                resCount = min(resCount, som.labels.size());


                unsigned maxI = outputs.size()/15;


			    for (unsigned i(0) ; i < maxI ; i++)
                {
                    for (unsigned k(0) ; k < resCount ; k++)
                    {
                        for (unsigned j(0) ; j < 15 ; j++)
                        {
                            Node bmu = outputs[i*15+j];
                            unsigned phoneme = som.getPhoneme(bmu.first, bmu.second, k);

                            if (k < 3)
                                resultats[k].data.push_back(phoneme);

                            if (phoneme == som.labels.size())
                                cout << "*__";
                            else
                            {
                                cout << som.labels[phoneme] << '_';
                                if (som.labels[phoneme].size() == 1)
                                    cout << '_';
                            }
                        }
                        cout  << endl;
                    }
                    cout << endl << endl;
                }

                sf::RenderWindow GraphWindow(sf::VideoMode(1280, 720), "Graph");
                Graph g(GraphWindow, 0.2, 2);
                g.addSignal(resultats);
                g.addSignal(resultats+1);
                g.addSignal(resultats+2);

                while (GraphWindow.isOpen())
                {
                    sf::Event event;
                    while (GraphWindow.pollEvent(event))
                    {
                        if (event.type == sf::Event::Closed)
                            GraphWindow.close();

                        g.update(event);
                    }

                    GraphWindow.clear(sf::Color::White);
                    g.draw(false);
                    GraphWindow.display();
                }

			} break;

            case 10: {
                fileName = askFile(fileName, "Fichier audio", ".wav");

                Signal audio(fileName);

                removeNoiseParams(audio);

                string res = askFile("clean", "Fichier audio", ".wav");

                audio.saveToFile(res);
            } break;

            case 11: {
                if (!sf::SoundBufferRecorder::isAvailable())
                {
                    std::cout << "Aucun microphone detecte" << std::endl;
                    system("pause");
                    break;
                }


                // create the recorder
                Recorder recorder(computer);

                // start the capture
                recorder.start();

                sf::sleep(sf::seconds(2.0));


                sf::RenderWindow GraphWindow(sf::VideoMode(1280, 720), "Enregistrement (espace pour arreter)");
                Graph g(GraphWindow, 0.00256, 20);
                g.addSignal(&computer.signal);



                while (GraphWindow.isOpen())
                {
                    sf::Event event;
                    while (GraphWindow.pollEvent(event))
                    {
                        if (event.type == sf::Event::Closed)
                            GraphWindow.close();

                        g.update(event);
                    }

                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
                        GraphWindow.close();


                    recorder.computeAvailableMFCCs();
                    g.signals[0] = &computer.signal;


                    GraphWindow.clear(sf::Color::White);

                    g.draw();

                    GraphWindow.display();
                }

                // stop the capture
                std::cout << std::endl << std::endl << "Fin de l'enregistrement" << std::endl;
                recorder.stop();




                std::ofstream file("output.txt", ios::out | ios::trunc);

                file << som.db.size() << endl;

			    for (unsigned i(0) ; i < som.db.size() ; i++)
                {
                    Node bmu = som.getBMU(som.db[i]);

                    unsigned k = 0, phoneme;
                    while ( (phoneme = som.getPhoneme(bmu.first, bmu.second, k)) != som.labels.size() )
                    {
                        if (k)
                            file << ",";

                        file << phoneme << " " << som.probas[bmu.first][bmu.second][phoneme];

                        k++;
                    }

                    file << endl;
                }




                std::cout << std::endl << "Classement" << std::endl;
                classerSOM(som, recorder.mfccs, "Database/ReVo.wav");

                std::cout << std::endl << "Fini" << std::endl;
                system("pause");

            } break;

			case 12: {
			    unsigned hidden = 55;
			    cout << "Hidden layers: "; cin >> hidden;
                ann = fann_create_standard(3, 39, hidden, som.couleurs.size());

                system("pause");
			} break;

			case 13:
			    fileName = askFile(fileName, "Charger MLP depuis", ".rna");
                ann = fann_create_from_file(fileName.c_str());

                system("pause");
			break;

			case 14: /// ENTRAINEMENT
            {
                string tr = askFile("rna", "Transcription", ".tr");

                createFANNData(som, "FANNdata.data", tr);

                fann_train_data* train_data = fann_read_train_from_file("FANNdata.data");

                fann_set_training_algorithm(ann, FANN_TRAIN_QUICKPROP);

                fann_init_weights(ann, train_data);

                fann_train_on_data(ann, train_data, 10000, 100, 0.009);

                cout << endl;

                cout << fann_test_data(ann, train_data) << endl;

                system("pause");
                break;
            }

			case 15:
			    fileName = askFile(fileName, "Enregistrer RNA dans", ".rna");
                fann_save(ann, fileName.c_str());
			break;

			case 16:
            {
                unsigned maxI = som.db.size()/15;

			    for (unsigned i(0) ; i < maxI ; i++)
                {
                    vector<Vector> results(15, Vector(33));

                    for (unsigned j(0) ; j < 15 ; j++)
                    {
                        Vector mfcc = som.db[i*15+j];

                        fann_type input[39];
                        for (unsigned k(0) ; k < 39 ; k++)
                            input[k] = (fann_type)mfcc[k];

                        fann_type* output = fann_run(ann, input);

                        for (unsigned k(0) ; k < 33 ; k++)
                            results[j][k] = (double)output[k];
                    }

                    vector<unsigned> index(15, 0);

                    for (unsigned j(0) ; j < 15 ; j++)
                    {
                        for (unsigned k(1) ; k < 33 ; k++)
                            if (results[j][k] > results[j][index[j]])
                                index[j] = k;

                        cout << som.labels[index[j]] << '_';
                    }
                    cout  << endl;

                    vector<unsigned> index2(15, 0);
                    for (unsigned j(0) ; j < 15 ; j++)
                    {
                        for (unsigned k(1) ; k < 33 ; k++)
                            if (results[j][k] > results[j][index2[j]] && k != index[j])
                                index2[j] = k;

                        cout << som.labels[index2[j]] << '_';
                    }
                    cout  << endl;

                    vector<unsigned> index3(15, 0);
                    for (unsigned j(0) ; j < 15 ; j++)
                    {
                        for (unsigned k(1) ; k < 33 ; k++)
                            if (results[j][k] > results[j][index3[j]] && k != index[j] && k != index2[j])
                                index3[j] = k;

                        cout << som.labels[index3[j]] << '_';
                    }
                    cout  << endl;

                    vector<unsigned> index4(15, 0);
                    for (unsigned j(0) ; j < 15 ; j++)
                    {
                        for (unsigned k(1) ; k < 33 ; k++)
                            if (results[j][k] > results[j][index4[j]] && k != index[j] && k != index2[j] && k != index3[j])
                                index4[j] = k;

                        cout << som.labels[index4[j]] << '_';
                    }

                    cout << endl << endl << endl;
                }

                unsigned input = 0;

                do
                {
                    cin >> input;

                    Vector mfcc = som.db[input];

                    fann_type input[39];
                    for (unsigned k(0) ; k < 39 ; k++)
                        input[k] = (fann_type)mfcc[k];

                    fann_type* output = fann_run(ann, input);

                    for (unsigned k(0) ; k < 31 ; k++)
                        cout << output[k] << " ";

                    cout << endl << endl;

                } while (input != 0);

                system("pause");

                break;
            }

			case 17:
            {
                string tr = askFile("transcription", "Transcription", ".tr");
                std::ofstream file(tr, ios::out | ios::trunc);

                file << som.db.size() << endl;

			    for (unsigned i(0) ; i < som.db.size() ; i++)
                {
                    Node bmu = som.getBMU(som.db[i]);
                    unsigned p = som.getPhoneme(bmu.first, bmu.second, 0);

                    if (p == som.couleurs.size())
                        file << -1 << endl;
                    else
                        file << som.getPhoneme(bmu.first, bmu.second, 0) << endl;
                }
                break;
            }

            case 18:
            {
                string tr;
                cout << "Transcription: "; cin >> tr;   tr = setExtension(tr, ".tr");
                fileName = askFile(fileName, "Audio file", ".wav");

                classerRNA(som, tr, som.db.db, "Database/" + fileName);

                break;
            }

            case 19:
            {
                vector<int> results;

                string tr = askFile(fileName, "Transcription", ".tr");
                std::ifstream file(tr);
                if (!file)
                {
                    cout << "Impossible d'ouvrir le fichier" << endl;
                    system("pause");
                    break;
                }

                unsigned _size;
                file >> _size; results.resize(_size, -1);

                cout << _size << endl;

                vector<vector<vector<unsigned>>> hits(som.w, vector<vector<unsigned>>(som.h, vector<unsigned>(som.couleurs.size(), 0)));
                vector<vector<unsigned>> hitCount(som.w, vector<unsigned>(som.h, 0));

                cout << "DB: " << som.db.db.size() << endl;

                // Load data
                for (unsigned i(0) ; i < som.db.db.size() ; i++)
                {
                    file >> results[i];

                    if ( results[i] == -1 )
                        continue;

                    if ( results[i] >= som.couleurs.size() )
                        continue;

                    Node node = som.getBMU(som.db.db[i]);

                    hits[node.first][node.second][results[i]]++;
                    hitCount[node.first][node.second]++;
                }

                cout << "Fichier chargé" << endl << endl;

                // Write to SOM
                for (unsigned i(0) ; i < som.w ; i++)
                {
                    for (unsigned j(0) ; j < som.h ; j++)
                    {
                        double scaleFactor = hitCount[i][j]? 1.0 / (double)hitCount[i][j]: 0.0;

                        for (unsigned k(0) ; k < som.couleurs.size() ; k++)
                            som.probas[i][j][k] = (double)hits[i][j][k] * scaleFactor;
                    }
                }

                cout << "Comptage termine" << endl;

                system("pause");

                som.sortProbas();

                break;
            }

            case 20:
            {
                std::ofstream file("output.txt", ios::out | ios::trunc);

                file << som.db.size() << endl;

			    for (unsigned i(0) ; i < som.db.size() ; i++)
                {
                    Node bmu = som.getBMU(som.db[i]);

                    unsigned k = 0, phoneme;
                    while ( (phoneme = som.getPhoneme(bmu.first, bmu.second, k)) != som.labels.size() )
                    {
                        if (k)
                            file << ",";

                        file << phoneme << " " << som.probas[bmu.first][bmu.second][phoneme];

                        k++;
                    }

                    file << endl;
                }

                break;
            }
		}

		system("cls");
	}
}

void traiterArguments(int argc, char* argv[], SOM& som)
{
    string somFile(""), dbFile("");

    for (int i(1) ; i < argc ; i++)
    {
        string format = getExtension(argv[i]);

        if (format == ".som")
            somFile = argv[i];

        else if (format == ".db")
            dbFile = argv[i];

        else if (format == ".wav")
        {
            Database DB;
            string audioFile = argv[i];

            MFCCComputer computer(13, 2);
            computer.setSignal(audioFile, 0);
            computer.computeMFCCs(DB.db);

            DB.saveToFile(setExtension(audioFile, ".db"));

            system("cls");
        }
    }

    if (somFile.size())
    {
        som.loadFromFile(somFile);

        somFile = removeExtension(somFile);
        size_t pos = somFile.rfind("\\");
        if (pos != std::string::npos)
            fileName = somFile.substr(pos+1, somFile.size()-1);
    }

    if (dbFile.size())
    {
        som.setDatabase(dbFile);

        dbFile = removeExtension(dbFile);
        size_t pos = dbFile.rfind("\\");
        if (pos != std::string::npos)
            fileName = dbFile.substr(pos+1, dbFile.size()-1);
    }
}

void entrainerSOM(SOM& som)
{
    Database& db = som.db;

    unsigned option = 0;
    do
    {
        system("cls");

        cout << "Coefficient d'apprentissage: " << db.la << " - " << db.lb << endl;
        cout << "Rayon: " << db.ra << " - " << db.rb << endl;
        cout << "Iterations : " << db.step << " / " << db.maxSteps << endl << endl;

        if (!db.size())
        {
            cout << endl << endl << "   /!\\ La DB est vide /!\\" << endl << endl;
        }

        cout << "1. Enregistrer DB & Entrainer" << endl;

        cout << "2. Enregistrer DB" << endl << endl;

        cout << "3. Reinitialiser le nombre d'iterations" << endl;
        cout << "4. Definir le nombre d'iterations" << endl << endl;

        cout << "5. la = 0.5 ; lb = 0.1   ; ra = 0.8*max(w, h) ; rb = 0.4*max(w, h)" << endl;
        cout << "6. la = 0.1 ; lb = 0.001 ; ra = 0.3*max(w, h) ; rb = 0.001" << endl;
        cout << "7. la = 0.3 ; lb = 0.001 ; ra = 0.4*max(w, h) ; rb = 0.001" << endl << endl;

        cout << "8. Randomiser SOM" << endl << endl;

        cout << "0. Annuler" << endl << endl;

        cout << "Choix: ";
        cin >> option;

        switch (option)
        {
            case 0:
                return;

            case 2:
                db.computeCoefs();
                db.saveToFile(setExtension(fileName, ".db"));
				break;

            case 3:
                db.step = 0;
				break;

            case 4:
                system("cls");
                cout << "Nombre d'iterations: "; cin >> db.maxSteps;
				break;

            case 5:
                db.la = 0.5;                    db.lb = 0.1;
                db.ra = 0.8*max(som.w, som.h);  db.rb = 0.4*max(som.w, som.h);
				break;

            case 6:
                db.la = 0.1;                    db.lb = 0.001;
                db.ra = 0.3*max(som.w, som.h);  db.rb = 0.001;
				break;

            case 7:
                db.la = 0.3;                    db.lb = 0.001;
                db.ra = 0.4*max(som.w, som.h);  db.rb = 0.001;
				break;

            case 8:
                som.randomize();
				break;
        }

    } while (option != 1);

    system("cls");

    som.initTraining();


    sf::Clock timer;
    unsigned delta(db.maxSteps/100);
    unsigned compteur(db.step / delta);


    time_t t = time(0);
    struct tm * now = localtime(&t);
    cout << "Debut: "
         << now->tm_hour << ':'
         << now->tm_min << ':'
         <<  now->tm_sec << endl << endl;

    cout << "Appris: " << compteur << "%";
    while (som.epoch())
    {
        if (db.step % delta == 0)
            cout << '\r' << "Appris: " << ++compteur << "%";

        if (GetAsyncKeyState(VK_ESCAPE) && GetForegroundWindow() == GetConsoleWindow())
        {
            bool stop = false;
            cout << endl << "Arreter l'entraienment ?";
            cin >> stop;
            if (stop)
            {
                cout << endl << "Entrainement arrete" << endl;
                break;
            }
            else
            {
                system("cls");
                cout << "Appris: " << compteur << "%";
            }
        }
    }

    db.saveToFile(setExtension(fileName, ".db"));


    if (db.step == 0)
    {
        cout << '\r' << "Appris: 100%" << endl << endl;

        t = time(0);
        now = localtime(&t);
        cout << "Fin: "
             << now->tm_hour << ':'
             << now->tm_min << ':'
             <<  now->tm_sec << endl << endl;

        FlashWindow(GetConsoleWindow(), TRUE);
    }

    cout << "Duree: " << toString(timer.getElapsedTime().asSeconds()) << " secondes" << endl << endl;
    system("pause");
}

void classerSOM(SOM& som, const vector<Vector>& _mfccs, string _file)
{
    if (!_mfccs.size())
    {
        cout << "   /!\\ Aucun MFCC /!\\" << endl << endl;
        system("pause");
        return;
    }
    if (!som.w || !som.h)
    {
        cout << "   /!\\ Le SOM est vide /!\\" << endl << endl;
        system("pause");
        return;
    }

    auto vm = sf::VideoMode::getDesktopMode();
    double ratioX = vm.width/1920.0;
    double ratioY = vm.height/1080.0;

    vector<vector<double>> um = som.getUMatrix4();
    unsigned squareSize = 27 * ratioX, space = squareSize+1;
    unsigned iconSize = 50 * ratioX;
    unsigned selected = 0;

    /// Chargements
        sf::SoundBuffer buf;
        if (!buf.loadFromFile(_file))
        {
            cout << "Fichier introuvable: " << _file << endl;
            system("pause");

            return;
        }

        Signal phrase(buf, sf::Color::Red);

        sf::RectangleShape rect(sf::Vector2f(1, 1));
        sf::CircleShape circle = sf::CircleShape(squareSize/2);
            circle.setFillColor( sf::Color::Yellow );

        sf::Sound soundFile;
        sf::Int16 samples[Nlength];

        sf::Font font;
        if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf"))
        {
            cout << "Echec lors du chargement de la police" << endl;
            system("pause");
            return;
        }

        sf::Text text;
        text.setFont(font);
        text.setCharacterSize(15 * ratioX);
        text.setFillColor(sf::Color::Black);

        sf::Vector2i textpos[] = { sf::Vector2i(0, 0), sf::Vector2i(squareSize/2, squareSize/3), sf::Vector2i(0, 2*squareSize/3) };

        bool moved = true;
        unsigned m = 0;

    // Fenetre 1
    sf::RenderWindow GraphWindow(sf::VideoMode(1280 * ratioX, 720 * ratioY), "Graph");
    Graph g(GraphWindow, 0.00256, 20);
    g.addSignal(&phrase);

    // Fenetre 2
    sf::RenderWindow SOMWindow(sf::VideoMode(som.w*space, som.h*space), "U-Matrix");

    // Fenetre 3
    sf::RenderWindow ImageWindow(sf::VideoMode(som.w*space, som.h*space), "Image");

    // Fenetre 4
    sf::RenderWindow CouleursWindow(sf::VideoMode(som.couleurs.size()*iconSize, iconSize), "Selecteur couleur");

    // Cacher la console
    ShowWindow(GetConsoleWindow(), SW_MINIMIZE);


    //31: hauteur barre de titre, 40: hauteur barre des taches
    unsigned f12w = 1280*ratioX + som.w*space;
    unsigned f4w = CouleursWindow.getSize().x;

    unsigned f14h = 720*ratioY + iconSize + 2*31;
    unsigned f234h = 2.0 * som.h*space + iconSize + 3*31;

    unsigned f1x = (1920*ratioX-f12w)*0.33, f1y = iconSize+31+ (1080*ratioY-40-f14h)*0.33*2.0;
    unsigned f4x = (1920*ratioX-f4w)*0.5, f4y = (1080*ratioY-40-f234h)*0.25;
    unsigned f23x = 2.0*f1x + 1280*ratioX, f2y = 2.0*f4y + 31+iconSize, f3y = 3.0*f4y + 2*31 + iconSize +som.h*space;

    GraphWindow.setPosition(sf::Vector2i(f1x, f1y));
//      SOMWindow.setPosition(sf::Vector2i(f23x, f2y ));
    ImageWindow.setPosition(sf::Vector2i(f23x, f3y ));
//    CouleursWindow.setPosition(sf::Vector2i( f4x-8, f4y ));

    while (GraphWindow.isOpen() && SOMWindow.isOpen() && ImageWindow.isOpen() && CouleursWindow.isOpen())
    {
        sf::Event event;
        rect.setScale(squareSize, squareSize);

        /// PREMIERE FENETRE    -   Graph
        while (GraphWindow.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                GraphWindow.close();

            double lastp = g.selected;
            g.update(event);
            if (lastp != g.selected)
                moved = true;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        {
            if (moved)
            {
                unsigned first = g.selected * 16000;

				if (first >= phrase.data.size())
				{
					for (unsigned i(0) ; i < Nlength ; i++)
						samples[i] = 0;
				}
				else
				{
                    unsigned c = min(Nlength, phrase.data.size() - first);

					for (unsigned i(0) ; i < c ; i++)
						samples[i] = phrase.data[first + i];
					for (unsigned i(c) ; i < Nlength ; i++)
						samples[i] = 0;
				}

                soundFile.stop();

                buf.loadFromSamples(samples, Nlength, 1, 16000);
                soundFile.setBuffer(buf);
                soundFile.setLoop(true);

                soundFile.play();

                moved = false;
            }

            soundFile.setLoop(true);

            if (soundFile.getStatus() == sf::SoundSource::Status::Stopped)
                soundFile.play();
        }
        else if (soundFile.getStatus() == sf::SoundSource::Status::Playing)
            soundFile.setLoop(false);


        GraphWindow.clear(sf::Color::White);

        g.draw();

        GraphWindow.display();

        m = round(100.0*g.selected);
        if (m >= _mfccs.size())
            m = _mfccs.size()-1;


        Node n = som.getBMU(_mfccs[m]);
        circle.setPosition(n.first*space, n.second*space);

        /// DEUXIEME FENETRE    -   UMatrix
        while (SOMWindow.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                SOMWindow.close();
        }

        // Clear screen
        SOMWindow.clear();

        // Draw u matrix
        for (unsigned i(0) ; i < som.w ; i++)
        {
            for (unsigned j(0) ; j < som.h ; j++)
            {
                unsigned c = 255 * um[i][j];
                rect.setFillColor( sf::Color(c, c, c) );
                rect.setPosition(i*space, j*space);

                SOMWindow.draw(rect);
            }
        }

        SOMWindow.draw(circle);

        // Update the window
        SOMWindow.display();

        /// TROISIEME FENETRE   -   Image
        rect.setScale(squareSize, squareSize/3.0);
        text.setCharacterSize(9 * ratioX);

        while (ImageWindow.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                ImageWindow.close();

            else if (event.type == sf::Event::MouseButtonPressed)
            {
                auto mp =  sf::Mouse::getPosition(ImageWindow);

                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1))
                        som.ordreProbas[mp.x/space][mp.y/space][0] = selected;

                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2))
                    {
                        if (som.ordreProbas[mp.x/space][mp.y/space].size() == 1)
                            som.ordreProbas[mp.x/space][mp.y/space].push_back(selected);
                        else
                            som.ordreProbas[mp.x/space][mp.y/space][1] = selected;
                    }

                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3))
                    {
                        if (som.ordreProbas[mp.x/space][mp.y/space].size() == 2)
                            som.ordreProbas[mp.x/space][mp.y/space].push_back(selected);
                        else
                            som.ordreProbas[mp.x/space][mp.y/space][2] = selected;
                    }
                }
            }
        }

        // Clear screen
        ImageWindow.clear();

        // Display Image
        for (unsigned i(0) ; i < som.w ; i++)
        {
            for (unsigned j(0) ; j < som.h ; j++)
            {
                if (!som.ordreProbas[i][j].size())
                    continue;

                rect.setPosition(i*space, j*space);

                for (unsigned k(0) ; k < 3 ; k++)
                {
                    if (k >= som.ordreProbas[i][j].size())
                    {
                        rect.setFillColor(sf::Color::White);
                        text.setString(" ");
                    }
                    else
                    {
                        unsigned index = som.ordreProbas[i][j][k];
                        rect.setFillColor(som.couleurs[index]);
                        text.setString(som.labels[index]);
                    }

                    text.setPosition(i*space+1 + textpos[k].x, j*space + textpos[k].y);

                    ImageWindow.draw(rect);
                    ImageWindow.draw(text);

                    rect.move(0, squareSize/3.0);
                }
            }
        }

        ImageWindow.draw(circle);

        // Update the window
        ImageWindow.display();

        /// QUATRIEME FENETRE   -   Couleurs
        rect.setScale(iconSize, iconSize);
        text.setCharacterSize(15 * ratioX);

        while (CouleursWindow.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                CouleursWindow.close();

            if (event.type == sf::Event::MouseButtonPressed)
                selected = sf::Mouse::getPosition(CouleursWindow).x / iconSize;
        }

        // Clear screen
        CouleursWindow.clear();

        // Display palette
        for (unsigned i(0) ; i < som.couleurs.size() ; i++)
        {
            rect.setFillColor( som.couleurs[i] );
            rect.setPosition(i*iconSize, 0);

            text.setString( som.labels[i] + '\n' + toString(i) );
            text.setPosition(i*iconSize+3, 10);

            if (i == selected)
                rect.setFillColor( sf::Color::Black );

            CouleursWindow.draw(rect);
            CouleursWindow.draw(text);
        }

        rect.setScale(iconSize-6, iconSize-6);
        rect.setFillColor( som.couleurs[selected] );
        rect.setPosition(selected*iconSize+3, 3);

        text.setString( som.labels[selected] );
        text.setPosition(selected*iconSize+5, 10);

        CouleursWindow.draw(rect);
        CouleursWindow.draw(text);

        // Update the window
        CouleursWindow.display();
    }

    GraphWindow.close();
    SOMWindow.close();
    ImageWindow.close();
    CouleursWindow.close();

    // Restore la console
    ShowWindow(GetConsoleWindow(), SW_RESTORE);
    SetFocus(GetConsoleWindow());
}

void classerRNA(SOM& som, string _output, const vector<Vector>& _mfccs, string _file)
{
    if (!_mfccs.size())
    {
        cout << "   /!\\ Aucun MFCC /!\\" << endl << endl;
        system("pause");
        return;
    }

    auto vm = sf::VideoMode::getDesktopMode();
    double ratioX = vm.width/1920.0;
    double ratioY = vm.height/1080.0;

    unsigned squareSize = 21 * ratioX, space = squareSize+1;
    unsigned iconSize = 50 * ratioX;
    unsigned selected = 0;

    /// Chargements
        sf::SoundBuffer buf;
        if (!buf.loadFromFile(_file))
        {
            cout << "Fichier introuvable: " << _file << endl;
            system("pause");

            return;
        }

        Signal phrase(buf, sf::Color::Red);

        sf::RectangleShape rect(sf::Vector2f(1, 1));

        sf::Sound soundFile;
        sf::Int16 samples[Nlength];

        sf::Font font;
        if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf"))
        {
            cout << "Echec lors du chargement de la police" << endl;
            system("pause");
            return;
        }

        sf::Text text;
        text.setFont(font);
        text.setCharacterSize(15 * ratioX);
        text.setFillColor(sf::Color::Black);

        bool moved = true;
        unsigned m = 0;

    // Fenetre 1
    sf::RenderWindow GraphWindow(sf::VideoMode(1280 * ratioX, 720 * ratioY), "Graph");
    Graph g(GraphWindow, 0.00256, 20);
    g.addSignal(&phrase);

    // Fenetre 3
    sf::RenderWindow ImageWindow(sf::VideoMode(som.w*space, som.h*space), "Image");

    // Fenetre 4
    sf::RenderWindow CouleursWindow(sf::VideoMode(som.couleurs.size()*iconSize, iconSize), "Selecteur couleur");

    // Cacher la console
    ShowWindow(GetConsoleWindow(), SW_MINIMIZE);


    //31: hauteur barre de titre, 40: hauteur barre des taches
    unsigned f12w = 1280*ratioX + som.w*space;
    unsigned f4w = CouleursWindow.getSize().x;

    unsigned f14h = 720*ratioY + iconSize + 2*31;
    unsigned f234h = 2.0 * som.h*space + iconSize + 3*31;

    unsigned f1x = (1920*ratioX-f12w)*0.33, f1y = iconSize+31+ (1080*ratioY-40-f14h)*0.33*2.0;
    unsigned f4x = (1920*ratioX-f4w)*0.5, f4y = (1080*ratioY-40-f234h)*0.25;
    unsigned f23x = 2.0*f1x + 1280*ratioX, f3y = 3.0*f4y + 2*31 + iconSize +som.h*space;

    GraphWindow.setPosition(sf::Vector2i(f1x, f1y));
    ImageWindow.setPosition(sf::Vector2i(f23x, f3y ));
    CouleursWindow.setPosition(sf::Vector2i( f4x-8, f4y ));


    vector<int> results(_mfccs.size(), -1);

    std::ifstream file(_output.c_str());

    unsigned _size;
    file >> _size;

    if (_size > results.size())
        results.resize(_size, -1);

    for (unsigned i(0) ; i < _size ; i++)
    {
        file >> results[i];
    }

    file.close();

    while (GraphWindow.isOpen() && CouleursWindow.isOpen())
    {
        sf::Event event;

        /// PREMIERE FENETRE    -   Graph
        while (GraphWindow.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                GraphWindow.close();

            double lastp = g.selected;
            g.update(event);
            if (lastp != g.selected)
                moved = true;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        {
            if (moved)
            {
                unsigned first = g.selected * 16000;

				if (first >= phrase.data.size())
				{
					for (unsigned i(0) ; i < Nlength ; i++)
						samples[i] = 0;
				}
				else
				{
                    unsigned c = min(Nlength, phrase.data.size() - first);

					for (unsigned i(0) ; i < c ; i++)
						samples[i] = phrase.data[first + i];
					for (unsigned i(c) ; i < Nlength ; i++)
						samples[i] = 0;
				}

                soundFile.stop();

                buf.loadFromSamples(samples, Nlength, 1, 16000);
                soundFile.setBuffer(buf);
                soundFile.setLoop(true);

                soundFile.play();

                moved = false;
            }

            soundFile.setLoop(true);

            if (soundFile.getStatus() == sf::SoundSource::Status::Stopped)
                soundFile.play();
        }
        else if (soundFile.getStatus() == sf::SoundSource::Status::Playing)
            soundFile.setLoop(false);


        GraphWindow.clear(sf::Color::White);

        g.draw();

        GraphWindow.display();

        m = round(100.0*g.selected);

        if (m >= _mfccs.size())
            m = _mfccs.size()-1;

        /// DEUXIEME FENETRE   -   Resultat
        text.setCharacterSize(50*ratioX);

        while (ImageWindow.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                ImageWindow.close();
        }

        // Clear screen
        ImageWindow.clear(sf::Color::White);

        text.setPosition(5, 5);
        if (results[m] != -1)
            text.setString(toString(m) + ":  " + som.labels[results[m]]);
        else
            text.setString(toString(m) + ":  _");

        ImageWindow.draw(text);
        ImageWindow.display();

        /// QUATRIEME FENETRE   -   Couleurs
        rect.setScale(iconSize, iconSize);
        text.setCharacterSize(15*ratioX);

        while (CouleursWindow.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                CouleursWindow.close();

            if (event.type == sf::Event::MouseButtonPressed)
            {
                selected = sf::Mouse::getPosition(CouleursWindow).x / iconSize;

                // Clear screen
                CouleursWindow.clear();

                // Display palette
                for (unsigned i(0) ; i < som.couleurs.size() ; i++)
                {
                    rect.setFillColor( som.couleurs[i] );
                    rect.setPosition(i*iconSize, 0);

                    text.setString( som.labels[i] + '\n' + toString(i) );
                    text.setPosition(i*iconSize+3, 10);

                    if (i == selected)
                        rect.setFillColor( sf::Color::Black );

                    CouleursWindow.draw(rect);
                    CouleursWindow.draw(text);
                }

                rect.setScale(iconSize-6, iconSize-6);
                rect.setFillColor( som.couleurs[selected] );
                rect.setPosition(selected*iconSize+3, 3);

                text.setString( som.labels[selected] );
                text.setPosition(selected*iconSize+5, 10);

                CouleursWindow.draw(rect);
                CouleursWindow.draw(text);

                // Update the window
                CouleursWindow.display();
            }
        }




        if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
        {
            results[m] = selected;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z) && results[m] != -1)
        {
            selected = results[m];
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
        {
            results[m] = -1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))
        {
            std::ofstream file2(_output.c_str(), ios::out | ios::trunc);

            file2 << results.size();

            for (unsigned i(0) ; i < results.size() ; i++)
            {
                file2 << endl << results[i];
            }
        }
    }

    GraphWindow.close();
    CouleursWindow.close();

    // Restore la console
    ShowWindow(GetConsoleWindow(), SW_RESTORE);
    SetFocus(GetConsoleWindow());

    std::ofstream file2(_output.c_str(), ios::out | ios::trunc);

    file2 << results.size();

    for (unsigned i(0) ; i < results.size() ; i++)
    {
        file2 << endl << results[i];
    }
}

void createFANNData(const SOM& _som, string _file, string _tr)
{
    unsigned number = 0;
    vector<int> results;

    std::ifstream input(_tr);

    if (!input)
    {
        cout << "File not found: " << _tr << endl;
        return;
    }

    unsigned _size;
    input >> _size; results.resize(_size);

    for (unsigned i(0) ; i < _size ; i++)
    {
        input >> results[i];
        if (results[i] != -1)
            number ++;
    }

    input.close();


    std::ofstream file(_file.c_str(), ios::out | ios::trunc);

    file << number << " " << 39 <<  " " << _som.couleurs.size();

    for (unsigned i(0) ; i < results.size() ; i++)
    {
        if (results[i] == -1)
            continue;

        // print MFCC
        Vector mfcc = _som.db.db[i];

        file << '\n' << mfcc[0];
        for (unsigned k(1) ; k < 39 ; k++)
            file << " " << mfcc[k];

        // print phoneme
        file << '\n';
        for (int k(0) ; k < (int)_som.couleurs.size() ; k++)
        {
            if (k == results[i])
                file << 1;
            else
                file << 0;

            if (k != (int)_som.couleurs.size()-1)
                file << " ";
        }
    }

}
