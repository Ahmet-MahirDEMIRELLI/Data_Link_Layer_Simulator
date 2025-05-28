#ifndef RESULTDIALOG_H
#define RESULTDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QTableWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <vector>
#include <bitset>

class SimulationWidget; // Forward declaration for our custom simulation widget

class ResultDialog : public QDialog {
    Q_OBJECT
public:
    // Pass the processed frames, CRC codes, and checksum string.
    ResultDialog(const std::vector<std::bitset<100>>& frames,
                 const std::vector<std::bitset<16>>& crcList,
                 const QString &checksum,
                 QWidget *parent = nullptr);

private:
    QTabWidget *tabWidget;
    QWidget *dataTab;
    QWidget *simTab;
    QTableWidget *crcTable;
    QLabel *checksumLabel;
    SimulationWidget *simulationWidget;

    void setupDataTab(const std::vector<std::bitset<100>>& frames,
                      const std::vector<std::bitset<16>>& crcList,
                      const QString &checksum);
    void setupSimulationTab(const std::vector<std::bitset<100>>& frames);
};

#endif // RESULTDIALOG_H
