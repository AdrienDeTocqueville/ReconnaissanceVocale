#include "Graph.h"

Graph::Graph(sf::RenderWindow& _window, double _xScale, double _yScale):
    window(_window),
    w(window.getSize().x), h(window.getSize().y),
    xS(_xScale), yS(_yScale),

    x(sf::Vector2f(w, 1)), y(sf::Vector2f(1, h)),
    cursor(sf::Vector2f(1, h)), selected(0.0),

    leftClic(false), rightClic(false)
{
    setOrigin(sf::Vector2i(0, h/2));

    x.setFillColor(sf::Color::Black);
    y.setFillColor(sf::Color::Black);

    cursor.setFillColor(sf::Color::Black);
}

Graph::~Graph()
{ }

void Graph::addSignal(Signal* func)
{
    signals.push_back(func);
}

void Graph::draw(bool _drawSelection)
{
    if (leftClic)
    {
        sf::Vector2i np = sf::Mouse::getPosition(window);
        setOrigin(o + np - cp);
        cp = np;
    }


    window.draw(x);
    window.draw(y);



    double winLength = 0.025;

    double timeStart = -o.x * xS, timeEnd = (w - o.x) * xS;
    for (Signal* func: signals)
    {
        unsigned iMin = min(timeStart * func->sampleRate, func->data.size() -1.0), iMax = min(timeEnd * func->sampleRate, func->data.size() -1.0);

        if (iMax-iMin > w)
        {
            sf::VertexArray ligne(sf::LinesStrip, w);

            for (unsigned i(0) ; i < w ; i++)
            {
                double delta = (timeEnd - timeStart) / w;

                unsigned index = iMin + (i*delta) * func->sampleRate;
                if (index >= func->data.size())
                {
                    ligne.resize(i);
                    break;
                }

                double time = timeStart + i*delta;

                ligne[i].position.x = i;
                ligne[i].position.y = o.y - func->data[index]/yS;

                if (_drawSelection && time >= selected && time < selected + winLength)
                    ligne[i].color = sf::Color::Green;
                else
                    ligne[i].color = func->color;
            }

            window.draw(ligne);
        }
        else
        {
            sf::VertexArray ligne(sf::LinesStrip, iMax - iMin);


            for (unsigned i(iMin) ; i < iMax ; i++)
            {
                double time = i/func->sampleRate;

                ligne[i-iMin].position.x = o.x + time/xS;
                ligne[i-iMin].position.y = o.y - func->data[i]/yS;

                if (_drawSelection && time >= selected && time < selected + winLength)
                    ligne[i-iMin].color = sf::Color::Green;
                else
                    ligne[i-iMin].color = func->color;
            }

            window.draw(ligne);
        }
    }

    cursor.setPosition(selected/xS + o.x -1, 0);
    if (_drawSelection)
        window.draw(cursor);
}

void Graph::setOrigin(sf::Vector2i _o)
{
    o = _o;
    o.x = min(o.x, 0);

    x.setPosition(0, o.y);
    y.setPosition(o.x, 0);
}

void Graph::setScale(double _x, double _y)
{
    xS = _x;
    yS = _y;
}

void Graph::update(const sf::Event& event)
{
    if (event.type == sf::Event::Resized)
    {
        w = event.size.width;
        h = event.size.height;

        window.setView( sf::View(sf::FloatRect(0.0, 0.0, w, h)) );


        x.setSize(sf::Vector2f(w, 1));
        y.setSize(sf::Vector2f(1, h));

        cursor.setSize(sf::Vector2f(1, h));

        o.x = w*0.5 - selected / xS;
        setOrigin(o);
    }

    if (event.type == sf::Event::MouseButtonPressed)
    {
        if (event.mouseButton.button == sf::Mouse::Left)
        {
            leftClic = true;
            cp = sf::Mouse::getPosition(window);
        }
        if (event.mouseButton.button == sf::Mouse::Right)
            rightClic = true;
    }
    else if (event.type == sf::Event::MouseButtonReleased)
    {
        if (event.mouseButton.button == sf::Mouse::Left)
            leftClic = false;

        if (event.mouseButton.button == sf::Mouse::Right)
            rightClic = false;
    }

    if (rightClic && sf::Mouse::isButtonPressed(sf::Mouse::Right))
    {
        sf::Vector2i mp = sf::Mouse::getPosition(window);

        selected = max((mp.x - o.x) * xS, 0.0);

        unsigned temp = (unsigned)(selected * 100);
        selected = temp * 0.01;
    }

    if (event.type == sf::Event::MouseWheelScrolled && event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel)
    {
        double mult = pow(2, -event.mouseWheelScroll.delta);

        xS  *= mult;
        yS  *= mult;

        o.x = w*0.5 - selected / xS;
        setOrigin(o);
    }

    if (event.type == sf::Event::KeyReleased)
    {
        if (event.key.code == sf::Keyboard::C)
            std::cout << xS << "   " << yS << std::endl;

        if (event.key.code == sf::Keyboard::Right)
            xS *= 0.5;
        else if (event.key.code == sf::Keyboard::Left)
            xS *= 2;
        else if (event.key.code == sf::Keyboard::Up)
            yS *= 0.5;
        else if (event.key.code == sf::Keyboard::Down)
            yS *= 2;
        else
            return;

        o.x = w*0.5 - selected / xS;
        setOrigin(o);
    }
}
