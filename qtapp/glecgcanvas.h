#ifndef GLECGCANVAS_H
#define GLECGCANVAS_H

#include <QOpenGLWidget>
#include <array>

class GLEcgCanvas : public QOpenGLWidget
{
    Q_OBJECT

public:
    GLEcgCanvas(QWidget *parent);
    void setSignalData(long* signalData);
    long* getSignalData();

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    void drawGridLines(int interval,  const QColor &penColor);
    void drawSignal(const QColor &penColor);
    void setScaleFactor(double scaleFactor);
    double trackYOffset();
    double ecgBigSquarePx();
    double ecgSmallSquarePx();
    double trackWidthPx();
    int pointsPerTrack();
    double scaleSignalXToPixels(int sampleIndex);
    double scaleSignalYToPixels(int sample);

    double m_mmPerBigSquare = 25.0;
    double m_msPerBigSquare = 1000;
    double m_dotsPerInch = 96.0;
    double m_mmPerInch = 25.4;
    double m_scaleFactor;
    std::vector<long> m_signalBuffer;
    int m_maxSignalSamples = 10000;
    int m_sampleFrequency = 500;
    double m_sampleResolution = 0.005;
    int m_paddingX = 50;
};

#endif
