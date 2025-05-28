#include "mainwindow.h"
#include <QFileDialog>
#include <QTableWidgetItem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <QFont>
#include <QHeaderView>
#include <QSpacerItem>
#include <QSizePolicy>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <bitset>
#include <QRandomGenerator>
#include <QDebug>
#include <QElapsedTimer>
#include <QThread>
#include <QTimer>
#include <QGraphicsRectItem>
#include <QPen>
#include <QBrush>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QEasingCurve>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    simulationView(nullptr),
    simulationScene(nullptr),
    simulationResultTable(nullptr),
    transmissionSpeedMultiplier(1.0)
{
    // Create the central widget and set background.
    centralWidget = new QWidget(this);
    centralWidget->setStyleSheet("background-color: white;");
    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Set global font.
    QFont globalFont("Segoe UI", 12);
    setFont(globalFont);

    // --- Welcome Message ---
    welcomeLabel = new QLabel("Welcome to the Data Link Layer Simulator!", this);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet("color: #2c3e50; font-size: 16pt; font-weight: bold;");
    mainLayout->addWidget(welcomeLabel);

    // --- Instruction Label ---
    instructionLabel = new QLabel("Please select a \".dat\" file from your computer:", this);
    instructionLabel->setAlignment(Qt::AlignCenter);
    instructionLabel->setStyleSheet("color: #34495e; font-size: 11pt;");
    mainLayout->addWidget(instructionLabel);

    // "Select Data File" button.
    selectFileButton = new QPushButton("Select Data File", this);
    selectFileButton->setFixedHeight(50);
    selectFileButton->setMinimumWidth(250);
    selectFileButton->setStyleSheet(
        "QPushButton { background-color: #3a8dde; color: white; font-size: 14pt; font-weight: bold; "
        "border: none; border-radius: 20px; padding: 10px 20px; }"
        "QPushButton:hover { background-color: #559ee0; }"
        "QPushButton:pressed { background-color: #2d7ac1; }"
        );
    mainLayout->addWidget(selectFileButton, 0, Qt::AlignHCenter);

    // File label.
    fileLabel = new QLabel("No file selected.", this);
    fileLabel->setStyleSheet("color: #444444;");
    fileLabel->setFont(QFont("Segoe UI", 11, QFont::Bold));
    fileLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(fileLabel);

    QSpacerItem* verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    mainLayout->addItem(verticalSpacer);

    // "Process Data" button.
    processButton = new QPushButton("Process Data", this);
    processButton->setFixedHeight(50);
    processButton->setMinimumWidth(250);
    processButton->setEnabled(false);
    processButton->setStyleSheet(
        "QPushButton { background-color: #28a745; color: white; font-size: 14pt; font-weight: bold; "
        "border: none; border-radius: 20px; padding: 10px 20px; }"
        "QPushButton:hover { background-color: #34c057; }"
        "QPushButton:pressed { background-color: #1e7e34; }"
        );
    mainLayout->addWidget(processButton, 0, Qt::AlignHCenter);

    // Polynomial label.
    polynomialLabel = new QLabel("CRC Polynomial: x^16 + x^12 + x^5 + 1", this);
    polynomialLabel->setAlignment(Qt::AlignCenter);
    polynomialLabel->setStyleSheet("color: #2c3e50; font-size: 12pt; font-weight: bold;");
    polynomialLabel->setVisible(false);
    mainLayout->addWidget(polynomialLabel);

    // CRC table.
    crcTable = new QTableWidget(this);
    crcTable->setColumnCount(2);
    crcTable->setHorizontalHeaderLabels(QStringList() << "Frame" << "CRC");
    crcTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    crcTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    crcTable->verticalHeader()->setDefaultSectionSize(40);
    crcTable->setStyleSheet(
        "QTableWidget { color: black; background-color: #f9f9f9; gridline-color: #dddddd; "
        "font-family: 'Segoe UI'; font-size: 10pt; font-weight: bold; }"
        "QHeaderView::section { background-color: #0055b3; color: white; font-size: 14pt; "
        "font-weight: bold; padding: 8px; border: none; }"
        );
    crcTable->setAlternatingRowColors(true);
    crcTable->setVisible(false);
    mainLayout->addWidget(crcTable);

    // Checksum label.
    checksumLabel = new QLabel(this);
    checksumLabel->setStyleSheet("color: #333333; font-size: 14pt; font-weight: bold;");
    checksumLabel->setAlignment(Qt::AlignCenter);
    checksumLabel->setVisible(false);
    mainLayout->addWidget(checksumLabel);

    // "Show Transmission Simulation" button.
    transmissionButton = new QPushButton("Show Transmission Simulation", this);
    transmissionButton->setFixedHeight(50);
    transmissionButton->setMinimumWidth(250);
    transmissionButton->setStyleSheet(
        "QPushButton { background-color: #e67e22; color: white; font-size: 14pt; font-weight: bold; "
        "border: none; border-radius: 20px; padding: 10px 20px; }"
        "QPushButton:hover { background-color: #eb984e; }"
        "QPushButton:pressed { background-color: #d35400; }"
        );
    transmissionButton->setVisible(false);
    mainLayout->addWidget(transmissionButton, 0, Qt::AlignHCenter);

    setCentralWidget(centralWidget);
    setWindowTitle("Data Link Layer Simulator");
    setMinimumSize(1000, 600);

    // Connect signals.
    connect(selectFileButton, &QPushButton::clicked, this, &MainWindow::onSelectFileClicked);
    connect(processButton, &QPushButton::clicked, this, &MainWindow::onProcessDataClicked);
    connect(transmissionButton, &QPushButton::clicked, this, &MainWindow::onTransmissionButtonClicked);
}

MainWindow::~MainWindow()
{
    // Automatically cleaned by Qt's parent-child system.
}

void MainWindow::onSelectFileClicked() {
    QString fileName = QFileDialog::getOpenFileName(this, "Select Data File", "", "Data Files (*.dat);;All Files (*)");
    if (fileName.isEmpty()){
        fileLabel->setText("No file selected.");
        processButton->setEnabled(false);
    } else {
        fileLabel->setText("Selected File: " + fileName);
        currentFilePath = fileName.toStdString();
        processButton->setEnabled(true);
    }
}

std::string MainWindow::dosyayiIkiliFormataCevir(const std::string &dosyaAdi) {
    std::ifstream dosya(dosyaAdi, std::ios::binary);
    if (!dosya) {
        std::cerr << "Dosya acilamadi!" << std::endl;
        return "";
    }
    std::string bitAkisi;
    char karakter;
    while (dosya.get(karakter))
        bitAkisi += std::bitset<8>(karakter).to_string();
    dosya.close();
    return bitAkisi;
}

std::vector<std::bitset<100>> MainWindow::cercevele(const std::string &bitAkisi) {
    std::vector<std::bitset<100>> frames;
    for (size_t i = 0; i < bitAkisi.size(); i += 100) {
        std::string frameStr = bitAkisi.substr(i, 100);
        while (frameStr.size() < 100)
            frameStr += '0';
        frames.emplace_back(frameStr);
    }
    return frames;
}

// 16-bit CRC hesaplama fonksiyonu
// input: 100 bitlik frame
// output: 16 bitlik CRC
std::bitset<16> MainWindow::computeCRC(const std::bitset<100> &frame) {
    std::bitset<116> data;
    std::bitset<17> divisor(0b10001000000100001); // CRC-16 polinomu: x^16 + x^12 + x^5 + 1
    int i, j;
    // data'nın 0 olduğunu garantile
    for(i = 0; i < 116; i++){
        data[i] = 0;
    }
    // 100 bitlik veriyi 116 bitlik alanın anlamlı kısmına kopyala
    for(i = 0; i < 100; i++){
        data[115 - i] = frame[99 - i];
    }
    // Bölme işlemi
    for(i = 115; i >= 16; i--){
        if(data[i]){ // Bit 1 ise işlem yap
            for(j = 16; j >= 0; j--){
                data[i + j - 16] = data[i + j - 16] ^ divisor[j]; // XOR işlemi
            }
        }
    }
    // En anlamsız 16 bit CRC sonucu olur
    std::bitset<16> crc;
    for(i = 0; i < 16; i++){
        crc[i] = data[i];
    }

    return crc;
}

// Tüm CRC kodlarını birleştirip checksum hesaplayan fonksiyon
// input: 16 bitlik CRC listesi
// output: checksum değeri
std::string MainWindow::calculateChecksum(const std::vector<std::bitset<16>> &crcList) {
    uint8_t checksum = 0, complement = 0;
    for(const auto &crc : crcList){
        complement = static_cast<uint8_t>(~crc.to_ulong() & 0xFF); // CRC'nin anlamsız 8 bitinin 1'e göre tümleyeni
        checksum += complement;
        complement = static_cast<uint8_t>(~(crc.to_ulong() >> 8) & 0xFF); // CRC'nin anlamlı 8 bitinin 1'e göre tümleyeni
        checksum += complement;
    }

    checksum = ~checksum + 1; // Tüm checksum'un 2'ye göre tümleyenini
    std::stringstream ss;
    ss << std::hex << static_cast<int>(checksum); // Hexadecimal format
    return ss.str();
}

void MainWindow::onProcessDataClicked() {
    std::string bitAkisi = dosyayiIkiliFormataCevir(currentFilePath);
    if (bitAkisi.empty())
        return;
    auto frames = cercevele(bitAkisi);
    processedFrames = frames;
    std::vector<std::bitset<16>> crcList;
    for (size_t i = 0; i < frames.size(); i++)
        crcList.push_back(computeCRC(frames[i]));
    crcTable->setRowCount(static_cast<int>(frames.size()));
    for (size_t i = 0; i < frames.size(); i++) {
        QTableWidgetItem *frameItem = new QTableWidgetItem(QString::fromStdString(frames[i].to_string()));
        QTableWidgetItem *crcItem = new QTableWidgetItem(QString::fromStdString(crcList[i].to_string()));
        crcTable->setItem(static_cast<int>(i), 0, frameItem);
        crcTable->setItem(static_cast<int>(i), 1, crcItem);
    }
    crcTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    crcTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    crcTable->setVisible(true);
    std::string checksum = calculateChecksum(crcList);
    checksumLabel->setText("Checksum: " + QString::fromStdString(checksum));
    checksumLabel->setVisible(true);
    polynomialLabel->setVisible(true);
    transmissionButton->setVisible(true);
    welcomeLabel->hide();
    instructionLabel->hide();
    selectFileButton->hide();
    fileLabel->hide();
    processButton->hide();
    QLayoutItem *child;
    while ((child = mainLayout->takeAt(0)) != nullptr) { }
    mainLayout->addWidget(polynomialLabel);
    mainLayout->addWidget(checksumLabel);
    mainLayout->addWidget(crcTable);
    mainLayout->addWidget(transmissionButton, 0, Qt::AlignHCenter);
    mainLayout->setStretchFactor(crcTable, 10);
}

void MainWindow::onTransmissionButtonClicked() {
    // Hide controls from previous stages.
    polynomialLabel->hide();
    checksumLabel->hide();
    crcTable->hide();
    transmissionButton->hide();

    // Create a container widget for the simulation elements.
    QWidget *simulationContainer = new QWidget(this);
    QVBoxLayout *simLayout = new QVBoxLayout(simulationContainer);
    simLayout->setContentsMargins(0, 0, 0, 0);
    simLayout->setSpacing(15);
    simLayout->setAlignment(Qt::AlignCenter);

    // Create the QGraphicsScene and QGraphicsView.
    simulationScene = new QGraphicsScene(this);
    simulationView = new QGraphicsView(simulationScene, simulationContainer);
    simulationView->setMinimumHeight(200);
    simulationView->setStyleSheet("background-color: #ffffff; color: black;");
    simulationView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    simulationView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    simulationView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    simulationScene->setSceneRect(0, 0, 800, 200);

    // Add the simulation view to the container.
    simLayout->addWidget(simulationView);

    // Draw the transmission line.
    simulationScene->addLine(0, 150, 800, 150, QPen(Qt::black, 2));
    simulationScene->addLine(0, 190, 800, 190, QPen(Qt::black, 2));

    // Load and place sender and receiver images.
    QPixmap senderPixmap(":/sender.jpg");
    QPixmap receiverPixmap(":/receiver.jpg");
    senderPixmap = senderPixmap.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    receiverPixmap = receiverPixmap.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QGraphicsPixmapItem *senderLogo = simulationScene->addPixmap(senderPixmap);
    senderLogo->setPos(10, 35);
    QGraphicsPixmapItem *receiverLogo = simulationScene->addPixmap(receiverPixmap);
    receiverLogo->setPos(710, 35);

    // Add text labels below sender and receiver.
    QGraphicsTextItem *senderLabel = simulationScene->addText("Sender");
    senderLabel->setFont(QFont("Segoe UI", 12, QFont::Bold));
    senderLabel->setDefaultTextColor(Qt::green);
    senderLabel->setPos(25, 120);

    QGraphicsTextItem *receiverLabel = simulationScene->addText("Receiver");
    receiverLabel->setFont(QFont("Segoe UI", 12, QFont::Bold));
    receiverLabel->setDefaultTextColor(Qt::red);
    receiverLabel->setPos(715, 120);

    // Create the simulation result table.
    simulationResultTable = new QTableWidget(this);
    simulationResultTable->setColumnCount(3);
    simulationResultTable->setHorizontalHeaderLabels(QStringList() << "Frame" << "Outcome" << "Explanation");
    simulationResultTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    simulationResultTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    simulationResultTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    simulationResultTable->setMinimumHeight(350);
    simulationResultTable->setStyleSheet(
        "QTableWidget { background-color: #ffffff; color: black; gridline-color: #000000; }"
        "QHeaderView::section { background-color: #0055b3; color: white; font-weight: bold; padding: 4px; }"
        );
    simulationResultTable->setRowCount(0);

    // Add the simulation result table to the container.
    simLayout->addWidget(simulationResultTable);

    // Speed up Transmission Button
    finishTransmissionButton = new QPushButton("Speed up Transmission", this);
    finishTransmissionButton->setFixedHeight(50);
    finishTransmissionButton->setMinimumWidth(250);
    finishTransmissionButton->setStyleSheet(
        "QPushButton { background-color: #dc3545; color: white; font-size: 14pt; font-weight: bold; "
        "border: none; border-radius: 20px; padding: 10px 20px; }"
        "QPushButton:hover { background-color: #e4606d; }"
        "QPushButton:pressed { background-color: #c82333; }"
        );
    simLayout->addWidget(finishTransmissionButton, 0, Qt::AlignHCenter);

    // Connect the finish button to set the simulation speed multiplier to a very small value,
    // effectively fast-forwarding the simulation.
    connect(finishTransmissionButton, &QPushButton::clicked, this, [this]() {
        transmissionSpeedMultiplier = 0.01;
    });

    // Add the simulation container to the main layout.
    mainLayout->addWidget(simulationContainer, 0, Qt::AlignCenter);

    // Start the simulation.
    simulateNextFrame(1);
}




void MainWindow::simulateNextFrame(int index) {
    if (index >= static_cast<int>(processedFrames.size()+1)) {
        // All frames have been processed.
        QGraphicsTextItem *textItem = simulationScene->addText("Sending Checksum Frame...");
        textItem->setDefaultTextColor(Qt::darkRed);
        textItem->setPos(300, 20);

        QGraphicsRectItem *checksumRect = simulationScene->addRect(0, 160, 80, 20, QPen(Qt::blue), QBrush(Qt::cyan));
        QGraphicsTextItem *checksumText = simulationScene->addText(QString("Checksum").arg(index));
        QGraphicsRectItem *headerRect = simulationScene->addRect(0, 160, 20, 20, QPen(Qt::yellow), QBrush(Qt::yellow));
        QGraphicsTextItem *headerText = simulationScene->addText(QString("Header").arg(index));
        checksumText->setDefaultTextColor(Qt::black);
        checksumText->setPos(5, 155);
        headerText->setDefaultTextColor(Qt::black);
        headerText->setPos(85, 155);


        int checksumSteps = 50;
        int baseChecksumDuration = 2000; // milliseconds
        int checksumInterval = baseChecksumDuration / checksumSteps;
        int checksumStartX = 0;
        int checksumEndX = 650;
        int headerStartX = 80;
        int headerEndX = 730;
        int checksumInitialStep = 0;

        // Simulate 5% chance of a checksum error.
        int checksumRand = QRandomGenerator::global()->bounded(100);
        if (checksumRand < 5) {
            // Animate the chceksum from sender to middle.
            checksumEndX = 270;
            int headerEndX = 350;
            QTimer *checksumTimer = new QTimer(this);
            int checksumCurrentStep = checksumInitialStep;
            connect(checksumTimer, &QTimer::timeout, this, [=]() mutable {
                if (checksumCurrentStep >= checksumSteps) {
                    checksumTimer->stop();
                    simulationScene->removeItem(textItem);
                    QGraphicsTextItem *errorText = simulationScene->addText("Checksum frame error occurred.\nStop-and-Wait Simulation completed with checksum error.");
                    errorText->setDefaultTextColor(Qt::red);
                    errorText->setPos(300, 20);
                }
                else {
                    qreal newChecksumX = checksumStartX + (checksumEndX - checksumStartX) * checksumCurrentStep / static_cast<double>(checksumSteps);
                    qreal newHeaderX = headerStartX + (headerEndX - headerStartX) * checksumCurrentStep / static_cast<double>(checksumSteps);
                    checksumRect->setPos(newChecksumX, 0);
                    headerRect->setPos(newHeaderX,0);
                    checksumText->setPos(newChecksumX + 5, 155);
                    headerText->setPos(newHeaderX + 5, 155);
                    checksumCurrentStep++;
                }
            });
            checksumTimer->start(checksumInterval);
        } else {
            // Animate the chceksum from sender to receiver.
            QTimer *checksumTimer = new QTimer(this);
            int checksumCurrentStep = checksumInitialStep;
            connect(checksumTimer, &QTimer::timeout, this, [=]() mutable {
                if (checksumCurrentStep >= checksumSteps) {
                    checksumTimer->stop();
                    simulationScene->removeItem(textItem);
                    QGraphicsTextItem *successText = simulationScene->addText("Checksum frame sent successfully.\nStop-and-Wait Simulation completed.");
                    successText->setDefaultTextColor(Qt::green);
                    successText->setPos(300, 20);
                }
                else {
                    qreal newChecksumX = checksumStartX + (checksumEndX - checksumStartX) * checksumCurrentStep / static_cast<double>(checksumSteps);
                    qreal newHeaderX = headerStartX + (headerEndX - headerStartX) * checksumCurrentStep / static_cast<double>(checksumSteps);
                    checksumRect->setPos(newChecksumX, 0);
                    headerRect->setPos(newHeaderX,0);
                    checksumText->setPos(newChecksumX + 5, 155);
                    headerText->setPos(newHeaderX + 5, 155);
                    checksumCurrentStep++;
                }
            });
            checksumTimer->start(checksumInterval);
        }
        return;
    }

    // Create a rectangle representing the frame at the sender.
    QGraphicsRectItem *frameRect = simulationScene->addRect(0, 160, 80, 20, QPen(Qt::blue), QBrush(Qt::cyan));
    QGraphicsTextItem *frameText = simulationScene->addText(QString("Frame %1").arg(index));
    frameText->setDefaultTextColor(Qt::black);
    frameText->setPos(5, 155);

    // Animate the frame from sender to receiver.
    int frameSteps = 50;
    int baseFrameDuration = 2000; // milliseconds
    int frameInterval = static_cast<int>((baseFrameDuration / static_cast<double>(frameSteps)) * transmissionSpeedMultiplier);

    int frameStartX = 0;
    int frameEndX = 730;
    int frameInitialStep = 0;

    // Determine the outcome for this frame.
    int outcomeRand = QRandomGenerator::global()->bounded(100);
    QString outcomeText;
    QString explanation;
    if (outcomeRand < 10) {  // 10% chance of frame loss.
        outcomeText = "Frame LOST";
        explanation = "Severe interference caused the frame to be lost. The acknowledgement was not received (timeout)";
    } else if (outcomeRand < 30) {  // Next 20%: frame corrupted.
        outcomeText = "Frame CORRUPTED";
        explanation = "Noise in the channel introduced bit errors. NACK received";
    } else if (outcomeRand < 45) {  // Next 15%: ACK lost.
        outcomeText = "ACK LOST";
        explanation = "The acknowledgement was not received (timeout).";
    } else {  // Remaining 55%: ACK received.
        outcomeText = "ACK RECEIVED";
        explanation = "Frame delivered successfully.";
    }

    if (outcomeText == "Frame LOST") {
        frameEndX = 350;
        frameInterval = static_cast<int>((1000.0 / frameSteps) * transmissionSpeedMultiplier);
    }
    QTimer *frameTimer = new QTimer(this);
    int frameCurrentStep = frameInitialStep;
    connect(frameTimer, &QTimer::timeout, this, [=]() mutable {
        if (frameCurrentStep >= frameSteps) {
            frameTimer->stop();
            if (outcomeText == "Frame CORRUPTED") {
                frameRect->setBrush(QBrush(Qt::red));
                int shortDelay = static_cast<int>(500 * transmissionSpeedMultiplier);
                QTimer::singleShot(shortDelay, [this, frameRect, frameText, index, outcomeText, explanation]() mutable {
                    simulationScene->removeItem(frameRect);
                    simulationScene->removeItem(frameText);
                    delete frameRect;
                    delete frameText;

                    QGraphicsRectItem *nackRect = simulationScene->addRect(0, 160, 60, 20, QPen(Qt::green), QBrush(Qt::yellow));
                    QGraphicsTextItem *nackText = simulationScene->addText("NACK");
                    nackText->setDefaultTextColor(Qt::black);
                    nackText->setPos(705, 155);
                    int nackSteps = 50;
                    int nackInterval = static_cast<int>((2000.0 / nackSteps) * transmissionSpeedMultiplier);
                    int nackCurrentStep = 0;
                    int nackStartX = 700;
                    int nackEndX = 0;
                    QTimer *nackTimer = new QTimer(this);
                    connect(nackTimer, &QTimer::timeout, this, [=]() mutable {
                        if (nackCurrentStep >= nackSteps) {
                            nackTimer->stop();
                            simulationScene->removeItem(nackRect);
                            simulationScene->removeItem(nackText);
                            delete nackRect;
                            delete nackText;
                            int row = simulationResultTable->rowCount();
                            simulationResultTable->insertRow(row);
                            simulationResultTable->setItem(row, 0, new QTableWidgetItem(QString::number(index)));
                            simulationResultTable->setItem(row, 1, new QTableWidgetItem(outcomeText));
                            simulationResultTable->setItem(row, 2, new QTableWidgetItem(explanation));
                            int finishDelay = static_cast<int>(500 * transmissionSpeedMultiplier);
                            QTimer::singleShot(finishDelay, [this, index]() {
                                simulateNextFrame(index); // NACK alındıysa aynı frame'i tekrar gönder
                            });
                        } else {
                            qreal nackNewX = nackStartX - ((nackStartX - nackEndX) * nackCurrentStep / static_cast<double>(nackSteps));
                            nackRect->setPos(nackNewX, 0);
                            nackText->setPos(nackNewX + 10, 155);
                            nackCurrentStep++;
                        }
                    });
                    nackTimer->start(nackInterval);
                });
            } else {
                simulationScene->removeItem(frameRect);
                simulationScene->removeItem(frameText);
                delete frameRect;
                delete frameText;
                if (outcomeText == "ACK RECEIVED") {
                    // Animate ACK from receiver to sender.
                    QGraphicsRectItem *ackRect = simulationScene->addRect(0, 160, 80, 20, QPen(Qt::green), QBrush(Qt::yellow));
                    QGraphicsTextItem *ackText = simulationScene->addText("ACK");
                    ackText->setDefaultTextColor(Qt::black);
                    ackText->setPos(705, 155);

                    int ackSteps = 50;
                    int ackInterval = static_cast<int>((2000.0 / ackSteps) * transmissionSpeedMultiplier);
                    int ackCurrentStep = 0;
                    int ackStartX = 730;
                    int ackEndX = 0;
                    QTimer *ackTimer = new QTimer(this);
                    connect(ackTimer, &QTimer::timeout, this, [=]() mutable {
                        if (ackCurrentStep >= ackSteps) {
                            ackTimer->stop();
                            simulationScene->removeItem(ackRect);
                            simulationScene->removeItem(ackText);
                            delete ackRect;
                            delete ackText;
                            int row = simulationResultTable->rowCount();
                            simulationResultTable->insertRow(row);
                            simulationResultTable->setItem(row, 0, new QTableWidgetItem(QString::number(index)));
                            simulationResultTable->setItem(row, 1, new QTableWidgetItem(outcomeText));
                            simulationResultTable->setItem(row, 2, new QTableWidgetItem(explanation));
                            int ackFinishDelay = static_cast<int>(500 * transmissionSpeedMultiplier);
                            QTimer::singleShot(ackFinishDelay, [this, index]() {
                                simulateNextFrame(index + 1);
                            });
                        } else {
                            qreal ackNewX = ackStartX - ((ackStartX - ackEndX) * ackCurrentStep / static_cast<double>(ackSteps));
                            ackRect->setPos(ackNewX, 0);
                            ackText->setPos(ackNewX + 10, 155);
                            ackCurrentStep++;
                        }
                    });
                    ackTimer->start(ackInterval);
                } else if (outcomeText == "ACK LOST") {
                    QGraphicsRectItem *ackRect = simulationScene->addRect(0, 160, 60, 20, QPen(Qt::green), QBrush(Qt::yellow));
                    QGraphicsTextItem *ackText = simulationScene->addText("ACK");
                    ackText->setDefaultTextColor(Qt::black);
                    ackText->setPos(705, 155);
                    int ackSteps = 50;
                    int ackInterval = static_cast<int>((1000.0 / ackSteps) * transmissionSpeedMultiplier);
                    int ackCurrentStep = 0;
                    int ackStartX = 700;
                    int ackEndX = 350;
                    QTimer *ackTimer = new QTimer(this);
                    connect(ackTimer, &QTimer::timeout, this, [=]() mutable {
                        if (ackCurrentStep >= ackSteps) {
                            ackTimer->stop();
                            simulationScene->removeItem(ackRect);
                            simulationScene->removeItem(ackText);
                            delete ackRect;
                            delete ackText;
                            int row = simulationResultTable->rowCount();
                            simulationResultTable->insertRow(row);
                            simulationResultTable->setItem(row, 0, new QTableWidgetItem(QString::number(index)));
                            simulationResultTable->setItem(row, 1, new QTableWidgetItem(outcomeText));
                            simulationResultTable->setItem(row, 2, new QTableWidgetItem(explanation));
                            int ackFinishDelay = static_cast<int>(500 * transmissionSpeedMultiplier);
                            QTimer::singleShot(ackFinishDelay, [this, index]() {
                                simulateNextFrame(index);  // ACK Lost ise aynı frame'i tekrar göndericez
                            });
                        } else {
                            qreal ackNewX = ackStartX - ((ackStartX - ackEndX) * ackCurrentStep / static_cast<double>(ackSteps));
                            ackRect->setPos(ackNewX, 0);
                            ackText->setPos(ackNewX + 10, 155);
                            ackCurrentStep++;
                        }
                    });
                    ackTimer->start(ackInterval);
                } else {
                    int row = simulationResultTable->rowCount();
                    simulationResultTable->insertRow(row);
                    simulationResultTable->setItem(row, 0, new QTableWidgetItem(QString::number(index)));
                    simulationResultTable->setItem(row, 1, new QTableWidgetItem(outcomeText));
                    simulationResultTable->setItem(row, 2, new QTableWidgetItem(explanation));
                    int finishDelay = static_cast<int>(500 * transmissionSpeedMultiplier);
                    QTimer::singleShot(finishDelay, [this, index]() {
                        simulateNextFrame(index); // Frame Lost ise aynı frame'i tekrar göndericez
                    });
                }
            }
        } else {
            qreal newX = frameStartX + (frameEndX - frameStartX) * frameCurrentStep / static_cast<double>(frameSteps);
            frameRect->setPos(newX, 0);
            frameText->setPos(newX + 5, 155);
            frameCurrentStep++;
        }
    });
    frameTimer->start(frameInterval);
}
