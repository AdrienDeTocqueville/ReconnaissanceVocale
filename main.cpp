#include "Reconnaissance.h"
#include "MFCCComputer.h"
#include "Vector.h"
#include "Graph.h"
#include "HTTP.h"
#include "SOM.h"

#define _WIN32_WINNT 0x0502
#define WINVER 0x0500
#include <windows.h>

string fileName = "revo";

void traiterArguments(int argc, char* argv[], SOM& som);

void entrainerSOM(SOM& som);
void classerSOM(SOM& som, const vector<Vector>& _mfccs, string _file);

int main(int argc, char* argv[])
{
    SetConsoleTitle("Reconnaissance vocale");

    SOM som; som.loadFromFile("revo.som");

    MFCCComputer computer(13, 2);

	unsigned input = 0;

    if (argc > 1)
        traiterArguments(argc, argv, som);

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

		cout << "10. Reconnaissance vocale" << endl;
		cout << "11. Reconnaissance de fichier" << endl << endl;

		cout << "12. Tester wordfind" << endl << endl;



		cout << endl << "0. Quitter" << endl << endl;

		cout << "Dimensions DB: " << som.db.size() << " x " << som.db.inputSize() << endl;
		cout << "Dimensions SOM: " << som.w << " x " << som.h << " x " << som.inputSize << endl;

		cout << "Choix: ";
        cin >> input;
		system("cls");

		switch (input)
		{
            case 0:
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

			case 9: /// Tester DB
            {
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

                 break;
			}

            case 10:
            {
                reconnaissanceVocale(som, computer);

                Database db; db.loadFromFile("ReVo.db");

                vector<Node> somOutput;
                vector<vector<Node>> partitions;

			    for (unsigned i(0) ; i < db.size() ; i++)
                {
                    Node bmu = som.getBMU(db[i]);
                    somOutput.push_back(bmu);
                }

                part(som, partitions, somOutput);
                for (unsigned i(0) ; i < partitions.size() ; i++)
                    transcrirePartition(som, partitions[i]);

                classerSOM(som, db.db, "Database/ReVo.wav");

                break;
            }

            case 11:
            {
                if (!som.db.size())
                {
                    cout << "Empty db" << endl;

                    som.setDatabase("doubleFinal.db");
                }
                vector<Node> somOutput;
                vector<vector<Node>> partitions;

			    for (unsigned i(0) ; i < som.db.size() ; i++)
                {
                    Node bmu = som.getBMU(som.db[i]);
                    somOutput.push_back(bmu);
                }

                cout << "Part: " << somOutput.size() << endl;
                part(som, partitions, somOutput);
                cout << "Division: " << partitions.size() << endl << endl;

                for (unsigned i(0) ; i < partitions.size() ; i++)
                    transcrirePartition(som, partitions[i], true);

                cout << endl;

                system("pause");
                break;
            }

            case 12:
            {
                tester_wordfinds();
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
    Graph g(GraphWindow, 0.00256, 80);
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
    unsigned f24h = som.h*space + iconSize + 2*31;

    unsigned f1x = (1920*ratioX-f12w)*0.33, f1y = iconSize+31+ (1080*ratioY-40-f14h)*0.33*2.0;
    unsigned f4x = (1920*ratioX-f4w)*0.5, f4y = (1080*ratioY-40-f24h)*0.25;
    unsigned f23x = 2.0*f1x + 1280*ratioX, f2y = f4y+iconSize+31 + (1080-f24h-f4y-40)*0.5;

    GraphWindow.setPosition(sf::Vector2i(f1x, f1y));
      SOMWindow.setPosition(sf::Vector2i(f23x, f2y ));
    ImageWindow.setPosition(sf::Vector2i(f23x, f2y ));
    CouleursWindow.setPosition(sf::Vector2i( f4x-8, f4y ));

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
                    unsigned px = mp.x/space, py = mp.y/space;

                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num0))
                    {
                        som.probas[px][py] = Vector(33, 0);
                        som.probas[px][py][selected] = 1.0;
                    }

                    sf::Keyboard::Key touches[] = {sf::Keyboard::Num1, sf::Keyboard::Num2, sf::Keyboard::Num3};
                    for (unsigned i(0) ; i < 3 ; i++)
                    {
                        if (sf::Keyboard::isKeyPressed(touches[i]) && i < som.ordreProbas[px][py].size())
                        {
                            double prev = som.probas[px][py][selected];
                            som.probas[px][py][selected] = som.probas[px][py][som.ordreProbas[px][py][i]];
                            som.probas[px][py][som.ordreProbas[px][py][i]] = prev;
                        }
                    }
                }
            }

            else if (event.type == sf::Event::KeyReleased)
            {
                if (event.key.code == sf::Keyboard::R)
                    som.sortProbas();
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
                text.setPosition(i*space+1, j*space);

                /// Affichage du plus probable
//                unsigned index = som.ordreProbas[i][j][0];
//
//                rect.setFillColor(som.couleurs[index]);
//                text.setString(som.labels[index]);
//
//
//                ImageWindow.draw(rect);
//                ImageWindow.draw(text);

                /// Affichage des 3 plus probables
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
