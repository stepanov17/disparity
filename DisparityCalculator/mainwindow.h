#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QIntValidator>

class QPixmap;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void onLoadImage(int nImg);
    void onRun();
    void onSave();

private:
    // convert a pixmap to grayscale
    static QPixmap toGrayScale(const QPixmap &pixmap);

    bool loaded[2] = {false, false};

    Ui::MainWindow *ui;

    QIntValidator *valStart;
    QIntValidator *valEnd;
};

#endif // MAINWINDOW_H
