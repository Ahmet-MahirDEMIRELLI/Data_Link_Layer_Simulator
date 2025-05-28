#include "simulationwidget.h"
#include <QGraphicsRectItem>
#include <QTimer>
#include <QRandomGenerator>
#include <QDebug>
#include <QPen>
#include <QBrush>

SimulationWidget::SimulationWidget(const std::vector<std::bitset<100>>& frames, QWidget *parent)
    : QGraphicsView(parent), framesData(frames), currentFrame(0)
{
    scene = new QGraphicsScene(this);
    setScene(scene);
    setupScene();

    // Start the simulation after a short delay.
    QTimer::singleShot(1000, this, SLOT(startSimulation()));
}

void SimulationWidget::setupScene() {
    // Define the scene rectangle. You can adjust these values as needed.
    scene->setSceneRect(0, 0, 800, 400);
    // Draw a line to simulate the transmission channel.
    scene->addLine(0, 200, 800, 200, QPen(Qt::black, 2));
}

void SimulationWidget::startSimulation() {
    if(currentFrame < static_cast<int>(framesData.size())) {
        simulateFrameTransmission();
    } else {
        qDebug() << "Simulation complete.";
    }
}

void SimulationWidget::simulateFrameTransmission() {
    // Create a rectangle item to represent the current frame.
    QGraphicsRectItem* frameRect = scene->addRect(0, 180, 50, 40, QPen(Qt::black), QBrush(Qt::blue));

    // Animation parameters.
    int animationDuration = QRandomGenerator::global()->bounded(50, 200);
    int startX = 0;
    int endX = 750;
    int steps = 20;
    int currentStep = 0;

    // Animate using a QTimer.
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=, &currentStep]() mutable {
        if (currentStep >= steps) {
            timer->stop();
            scene->removeItem(frameRect);
            delete frameRect;
            currentFrame++; // Move to next frame.
            // Wait a moment before starting the next frame.
            QTimer::singleShot(500, this, SLOT(startSimulation()));
        } else {
            // Calculate new position for the frame.
            qreal dx = startX + (endX - startX) * currentStep / steps;
            frameRect->setPos(dx, 180);
            currentStep++;
        }
    });
    timer->start(animationDuration);
}
