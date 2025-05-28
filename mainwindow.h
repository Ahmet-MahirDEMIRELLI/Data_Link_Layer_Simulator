#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QGraphicsView>
#include <vector>
#include <bitset>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSelectFileClicked();
    void onProcessDataClicked();
    void onTransmissionButtonClicked();
    void simulateNextFrame(int index);

private:
    // UI elements used in the original view
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QLabel *welcomeLabel;
    QLabel *instructionLabel;
    QPushButton *selectFileButton;
    QLabel *fileLabel;
    QPushButton *processButton;
    QLabel *polynomialLabel;
    QTableWidget *crcTable;
    QLabel *checksumLabel;
    QPushButton *transmissionButton;
    double transmissionSpeedMultiplier;  // global multiplier; 1.0 is normal speed, lower values speed up simulation.
    QPushButton *finishTransmissionButton;  // store the checksum string

    // New simulation UI elements
    QGraphicsView *simulationView;
    QGraphicsScene *simulationScene;
    QTableWidget *simulationResultTable;  // Table below simulation with outcomes

    // Processed data storage
    std::string currentFilePath;
    std::vector<std::bitset<100>> processedFrames;

    // File processing and CRC functions.
    std::string dosyayiIkiliFormataCevir(const std::string &dosyaAdi);
    std::vector<std::bitset<100>> cercevele(const std::string &bitAkisi);
    std::bitset<16> computeCRC(const std::bitset<100>& frame);
    std::string calculateChecksum(const std::vector<std::bitset<16>>& crcList);
};

#endif // MAINWINDOW_H
