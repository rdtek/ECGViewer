#include "GLEcgCanvas.h"
#include <QPainter>
#include <QBrush>
#include <QPaintEvent>
#include <QWidget>
#include <QDebug>
#include <tgmath.h>
#include <array>

GLEcgCanvas::GLEcgCanvas(QWidget *parent) : QOpenGLWidget(parent)
{
    setFixedSize(600, 400);
    setAutoFillBackground(false);
}

void GLEcgCanvas::setSignalData(vector<long>& signalData)
{
    m_signalBuffer = signalData; //TODO: load signal data into member vector
    update();
}

void GLEcgCanvas::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(event->rect(), QBrush(Qt::white));
    painter.end();

    drawGridLines(10, QColor(255, 192, 192));
    drawGridLines(50, QColor(240, 128, 128));
    drawSignal(QColor(0, 0, 0));
}

void GLEcgCanvas::drawGridLines(int interval, const QColor &penColor)
{
    int windowWidth = this->width();
    int windowHeight = this->height();

    QPainter painter(this);
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(penColor, 1));

    //Vertical lines
    for(int i = 0; i * interval < windowWidth; i++){
        QVector<QPoint> polyPoints;
        polyPoints.append( QPoint(interval * i, 0) );
        polyPoints.append( QPoint(interval * i, windowHeight) );
        painter.drawPolyline( polyPoints );
    }

    //Horizontal lines
    for(int i = 0; i * interval < windowHeight; i++){
        QVector<QPoint> polyPoints;
        polyPoints.append( QPoint(0, interval * i) );
        polyPoints.append( QPoint(windowWidth, interval * i) );
        painter.drawPolyline( polyPoints );
    }

    painter.end();
}

void GLEcgCanvas::drawSignal(const QColor &penColor)
{
    qDebug() << "Draw signal.";
    //QPainter painter(this);
    //painter.begin(this);
    //painter.setPen(QPen(penColor, 1));

    double yOffset = trackYOffset() / 2;
    double xPos = m_paddingX, yPos = 0;
    int i = 0, iSample = 0;

    if(m_signalBuffer.empty()) return;

    for (i = 0, iSample = 0; iSample < 1000 - 1; i++, iSample++) {

        qDebug() << "loop: ";
        //qDebug() << QString("%1").arg(m_signalBuffer[i]);

        double x1 = xPos;
        double test = m_signalBuffer[iSample] * -1;
        double y1 = yOffset + scaleSignalYToPixels(m_signalBuffer[iSample] * -1);
        double x2 = m_paddingX + scaleSignalXToPixels(i + 1);
        double y2 = yOffset + scaleSignalYToPixels(m_signalBuffer[iSample + 1] * -1);

        QVector<QPoint> linePoints;
        linePoints.append( QPoint(x1, y1) );
        linePoints.append( QPoint(x2, y2) );
        //painter.drawPolyline( linePoints );

        xPos = x2;
        yPos = y2;

        if (i == (pointsPerTrack() - 1)) {
            i = 0;
            xPos = m_paddingX;
            yOffset += trackYOffset();
        }
    }
}

void GLEcgCanvas::setScaleFactor(double scaleFactor) {
    m_scaleFactor = scaleFactor;
}

double GLEcgCanvas::trackYOffset() {
    return 2.5 * ecgBigSquarePx();
}

double GLEcgCanvas::ecgBigSquarePx() {
    return m_scaleFactor * round(m_mmPerBigSquare * m_dotsPerInch / m_mmPerInch);
}

double GLEcgCanvas::ecgSmallSquarePx() {
    return ecgBigSquarePx() / 5;
}

double GLEcgCanvas::trackWidthPx() {
    return this->width() - 2 * m_paddingX;
}

int GLEcgCanvas::pointsPerTrack() {
    return round(trackWidthPx() / ecgBigSquarePx() * m_sampleFrequency);
}

double GLEcgCanvas::scaleSignalXToPixels(int sampleIndex) {
    double xPixelValue = sampleIndex / m_sampleFrequency * ecgBigSquarePx();
    return round(xPixelValue);
}

double GLEcgCanvas::scaleSignalYToPixels(int sample) {
    qDebug() << "s";
    double yPixelValue = sample * m_sampleResolution * ecgBigSquarePx();
     qDebug() << "pv" << yPixelValue;
    return round(yPixelValue);
}
