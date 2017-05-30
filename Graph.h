#ifndef GRAPH_H
#define GRAPH_H

#include "Signal.h"

class Graph
{
    public:
        Graph(sf::RenderWindow& _window, double _xScale = 1, double _yScale = 1);
        ~Graph();

        void addSignal(Signal* func);

        void update(const sf::Event& event);

        void draw(bool _drawSelection = true);

        void setOrigin(sf::Vector2i _o);
        void setScale(double _x, double _y);

//    private:
        sf::RenderWindow& window;

        std::vector<Signal*> signals;

        int w, h;
        sf::Vector2i o;
        double xS; // 1 pixel = xS seconde
        double yS; // 1 pixel = yS UA

        sf::RectangleShape x, y, cursor;
        double selected;

        bool leftClic, rightClic;
        sf::Vector2i cp;
};

#endif // GRAPH_H
