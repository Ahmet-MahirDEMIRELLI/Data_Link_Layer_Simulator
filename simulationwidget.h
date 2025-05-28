#ifndef SIMULATIONWIDGET_H
#define SIMULATIONWIDGET_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <vector>
#include <bitset>

class SimulationWidget : public QGraphicsView {
    Q_OBJECT
public:
    explicit SimulationWidget(const std::vector<std::bitset<100>>& frames, QWidget *parent = nullptr);

public slots:
    void startSimulation(); // slot to kick off the animation

private:
    QGraphicsScene *scene;
    std::vector<std::bitset<100>> framesData;
    int currentFrame;

    void setupScene();
    void simulateFrameTransmission(); // simulate one frame's transmission
};

#endif // SIMULATIONWIDGET_H
