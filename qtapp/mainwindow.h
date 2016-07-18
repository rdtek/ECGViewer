#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QWidget>
#include <QMainWindow>
#include "GLEcgCanvas.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
private:
    GLEcgCanvas *m_ecg;
private slots:
    void openFile();
};

#endif
