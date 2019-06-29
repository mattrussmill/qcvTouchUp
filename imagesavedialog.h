#ifndef IMAGESAVEDIALOG_H
#define IMAGESAVEDIALOG_H

#include <QFileDialog>

class ImageSaveDialog : public QFileDialog
{
    Q_OBJECT

public:
    explicit ImageSaveDialog(QImage &image, QWidget *parent = nullptr);
    ImageSaveDialog(QImage &image, QWidget *parent = nullptr, const QString &caption = QString(),
                    const QString &directory = QString());
    ~ImageSaveDialog();

private:
    void appendImageOptionsWidget(QWidget *widget);
};

#endif // IMAGESAVEDIALOG_H
