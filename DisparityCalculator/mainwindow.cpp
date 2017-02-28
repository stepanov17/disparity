#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "disparitycalculator.h"

#include <QPixmap>
#include <QDebug>
#include <QFileDialog>
#include <QSignalMapper>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    valStart = new QIntValidator(this);
    valStart->setBottom(0);
    ui->lnDStart->setValidator(valStart);
    valEnd   = new QIntValidator(this);
    valEnd->setBottom(0);
    ui->lnDEnd->setValidator(valEnd);

    QSignalMapper* mapper = new QSignalMapper(this);
    QObject::connect(ui->actionLoad1, SIGNAL(triggered()), mapper, SLOT(map()));
    QObject::connect(ui->actionLoad2, SIGNAL(triggered()), mapper, SLOT(map()));
    mapper->setMapping(ui->actionLoad1, 1);
    mapper->setMapping(ui->actionLoad2, 2);
    QObject::connect(mapper, SIGNAL(mapped(int)), this, SLOT(onLoadImage(int)));

    QObject::connect(ui->btnRun, SIGNAL(clicked()), this, SLOT(onRun()));

    QObject::connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(onSave()));
}

MainWindow::~MainWindow() { delete ui; }

QPixmap MainWindow::toGrayScale(const QPixmap &pixmap) {

    QImage img = pixmap.toImage();
    if (img.isGrayscale()) {
        return QPixmap(pixmap);
    }
    QImage imgGray = img.convertToFormat(QImage::Format_Grayscale8, Qt::MonoOnly);
    Q_ASSERT(imgGray.isGrayscale());
    return QPixmap::fromImage(imgGray);
}

void MainWindow::onLoadImage(int nImg) {

    Q_ASSERT(nImg == 1 || nImg == 2);

    QString caption = "Load image " + QString::number(nImg);
    QString file = QFileDialog::getOpenFileName(
        this, tr(caption.toStdString().c_str()), "", tr("PNG files (*.png)"));

    if (file.isNull() || file.isEmpty()) { return; }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QPixmap img(file);

    if (img.width() < 1 || img.height() < 1) {

        QMessageBox err;
        err.critical(this, "error", "invalid image dimensions");
        err.show();
        return;
    }

    if (nImg == 1) {
        ui->lblImage1->setPixmap(toGrayScale(img));
    } else {
        ui->lblImage2->setPixmap(toGrayScale(img));
    }

    QApplication::restoreOverrideCursor();

    loaded[nImg - 1] = true;
    ui->btnRun->setEnabled(loaded[0] && loaded[1]);

    ui->lblDisp->clear();
    ui->actionSave->setEnabled(false);
}

void MainWindow::onRun() {

    if (!(loaded[0] && loaded[1])) {
        Q_ASSERT(false);
        return;
    }

    int dStart = ui->lnDStart->text().toInt();
    int dEnd = ui->lnDEnd->text().toInt();

    if (dStart > dEnd) {

        QMessageBox err;
        err.critical(this, "error", "invalid disparity range (max < min)");
        err.show();

        return;
    }

    const QPixmap *p1 = ui->lblImage1->pixmap();
    const QPixmap *p2 = ui->lblImage2->pixmap();
    if (!(p1 && p2)) {
        Q_ASSERT(false);
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QImage disp = DisparityCalculator::calculate(
        p1->toImage(), p2->toImage(), dStart, dEnd);
    ui->lblDisp->setPixmap(QPixmap::fromImage(disp));

    QApplication::restoreOverrideCursor();

    ui->actionSave->setEnabled(true);
}

void MainWindow::onSave() {

    const QPixmap *img = ui->lblDisp->pixmap();
    if (!img) { // just in case
        QMessageBox err;
        err.critical(this, "error", "nothing to save");
        err.show();
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Save file"), "", tr("PNG files (*.png)"));
    if (fileName.isNull() || fileName.isEmpty()) { return; }
    if (!fileName.endsWith(".png")) { fileName += ".png"; }

    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    img->save(&file, "png");
    file.close();
}
