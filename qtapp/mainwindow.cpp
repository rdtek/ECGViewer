#include <QGridLayout>
#include <QLabel>
#include <QTimer>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QDate>
#include <QFileDialog>
#include "GLEcgCanvas.h"
#include "MainWindow.h"
#include <QDebug>

MainWindow::MainWindow()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QAction *fileOpenAct = new QAction(QString("Open"),this);
    connect(fileOpenAct, &QAction::triggered, this, &MainWindow::openFile);
    fileMenu->addAction(fileOpenAct);

    setWindowTitle(tr("2D Painting on OpenGL Widget"));
    m_ecg = new GLEcgCanvas(this);
    this->setCentralWidget(m_ecg);
}

void MainWindow::openFile(){

    QString fileName = QFileDialog::getOpenFileName(this,
             tr("Open ECG Data"), "/home", tr("TXT Files (*.txt)"));
    qDebug() << "Filename:" << fileName;

    QFile inputFile(fileName);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        int indexBuffer = 0;
        long signalBuffer[10000];
        int maxBufferLength = 10000;
        QTextStream inputStream(&inputFile);

        while (!inputStream.atEnd()){
            QString line = inputStream.readLine();

            bool convertOk = false;
            signalBuffer[indexBuffer] = line.toLong(&convertOk, 10);
            indexBuffer++;
            if(indexBuffer >= maxBufferLength) break;
        }
        inputFile.close();

        for(int i = 0; i < indexBuffer; i++){
            qDebug() << "Line: " << signalBuffer[i];
        }

        m_ecg->setSignalData(signalBuffer);
    }
}
