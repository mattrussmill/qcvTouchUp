#ifndef HISTOGRAMOBJECT_H
#define HISTOGRAMOBJECT_H

#include <QObject>
#include <QSharedData>
#include <QSharedDataPointer>
class QImage;
class HistogramData;

class HistogramObject : public QObject
{
    Q_OBJECT

public:
    HistogramObject();
    HistogramObject(const QImage &image);
    HistogramObject(const HistogramObject &hobj);
    ~HistogramObject();
    HistogramObject clone();
    uint32_t operator[](int index);
    int getIndex(int channel, int pixelIntensity);
    int getNumChannels();

public slots:
    bool update(const QImage &image);

signals:
    void updated();

private:
    QSharedDataPointer<HistogramData> data;
};

#endif // HISTOGRAMOBJECT_H
