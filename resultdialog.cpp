#include "resultdialog.h"
#include "simulationwidget.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QStringList>

ResultDialog::ResultDialog(const std::vector<std::bitset<100>>& frames,
                           const std::vector<std::bitset<16>>& crcList,
                           const QString &checksum,
                           QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Data Processing Results");
    resize(1000, 600);

    tabWidget = new QTabWidget(this);

    // Create two tabs.
    dataTab = new QWidget(this);
    simTab = new QWidget(this);

    setupDataTab(frames, crcList, checksum);
    setupSimulationTab(frames);

    tabWidget->addTab(dataTab, "Processed Data");
    tabWidget->addTab(simTab, "Transmission Simulation");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(tabWidget);
    setLayout(mainLayout);
}

void ResultDialog::setupDataTab(const std::vector<std::bitset<100>>& frames,
                                const std::vector<std::bitset<16>>& crcList,
                                const QString &checksum)
{
    QVBoxLayout *layout = new QVBoxLayout(dataTab);

    // Create the table for frames and CRCs.
    crcTable = new QTableWidget(dataTab);
    crcTable->setColumnCount(2);
    crcTable->setHorizontalHeaderLabels(QStringList() << "Frame" << "CRC");
    crcTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    crcTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    crcTable->setRowCount(static_cast<int>(frames.size()));

    for (size_t i = 0; i < frames.size(); i++) {
        QTableWidgetItem *frameItem = new QTableWidgetItem(QString::fromStdString(frames[i].to_string()));
        QTableWidgetItem *crcItem = new QTableWidgetItem(QString::fromStdString(crcList[i].to_string()));
        crcTable->setItem(static_cast<int>(i), 0, frameItem);
        crcTable->setItem(static_cast<int>(i), 1, crcItem);
    }

    checksumLabel = new QLabel("Checksum: " + checksum, dataTab);
    checksumLabel->setAlignment(Qt::AlignCenter);
    checksumLabel->setStyleSheet("font-size: 14pt; font-weight: bold;");

    layout->addWidget(checksumLabel);
    layout->addWidget(crcTable);
    dataTab->setLayout(layout);
}

void ResultDialog::setupSimulationTab(const std::vector<std::bitset<100>>& frames)
{
    QVBoxLayout *layout = new QVBoxLayout(simTab);

    // Create the simulation widget.
    simulationWidget = new SimulationWidget(frames, simTab);
    layout->addWidget(simulationWidget);
    simTab->setLayout(layout);
}
