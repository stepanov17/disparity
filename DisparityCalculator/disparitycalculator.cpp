#include "disparitycalculator.h"

#include <QPixmap>
#include <QColor>
#include <QDebug>

QImage DisparityCalculator::calculate(const QImage &img1,
                                      const QImage &img2,
                                      int dStart,
                                      int dEnd) {

    Q_ASSERT(dStart <= dEnd);

    const int
        w1 = img1.width(), h1 = img1.height(),
        w2 = img2.width(), h2 = img2.height();

    Q_ASSERT((h1 > 0) && (w1 > 0) && (h2 > 0) && (w2 > 0));

    std::vector<int> r1(w1, 0.), r2(w2, 0.);
    intMatr_t m1(h1, r1), m2(h2, r2);

    for (int i = 0; i < h1; ++i) {
        for (int j = 0; j < w1; ++j) {
            QColor c = img1.pixel(j, i);
            Q_ASSERT(c.red() == c.green() && c.red() == c.blue());
            m1[i][j] = c.red();
        }
    }

    for (int i = 0; i < h2; ++i) {
        for (int j = 0; j < w2; ++j) {
            QColor c = img2.pixel(j, i);
            Q_ASSERT(c.red() == c.green() && c.red() == c.blue());
            m2[i][j] = c.red();
        }
    }

    // === calculate ===

    QImage res(w1, h1, QImage::Format_ARGB32);

    for (int i = 0; i < h1; ++i) {
        for (int j = 0; j < w1; ++j) {

            int d = getDisparity(m1, m2, i, j, dStart, dEnd);

            QColor c = Qt::blue;
            if (d != UNDEF) {
                double scale =
                    255. * (static_cast<double>(d) - dStart) / (dEnd - dStart);
                int g = static_cast<int>(scale);
                c = QColor(g, g, g);
            }

            res.setPixel(j, i, c.rgba());
        }
    }

    return res;
}

int DisparityCalculator::getDisparity(const intMatr_t &m1,
                                      const intMatr_t &m2,
                                      int i0,
                                      int j0,
                                      int dStart,
                                      int dEnd) {

    int h1 = m1.size(), h2 = m2.size();

    // row checks
    if (i0 < M) { return UNDEF; }
    if (i0 > h1 - M - 1) { return UNDEF; }
    if (i0 > h2 - M - 1) { return UNDEF; }

    // TODO: add column checks to avoid excessive calculations

    int nR = std::max(dStart, dEnd) - std::min(dStart, dEnd) + 1;
    std::vector<double> R(nR, 0.);

    for (int d = dStart; d <= dEnd; ++d) {
        auto res = r(m1, m2, i0, j0, d); // m1 -> m2
        if (res.second) { return UNDEF; }
        else { R[d - dStart] = res.first; }
    }
    int d1 = dStart + argMin(R);

    for (int d = -dEnd; d <= -dStart; ++d) {
        auto res = r(m2, m1, i0, j0 + d1, d); // m2 -> m1
        if (res.second) { return UNDEF; }
        else { R[d + dEnd] = res.first; }
    }
    int d2 = -dEnd + argMin(R);

    if (std::abs(d1 + d2) > 1) { return UNDEF; }
    return d1;
}

std::pair<double, bool> DisparityCalculator::r(const intMatr_t &m1,
                                               const intMatr_t &m2,
                                               int i0,
                                               int j0,
                                               int d) {
    int h1 = m1.size(), h2 = m2.size();
    int w1 = m1[0].size(), w2 = m2[0].size();

    double SSD = 0., mean = 0.;
    for (int i = i0 - M; i <= i0 + M; ++i) {

        // range check
        if (i < 0 || i >= h1 || i >= h2) { return std::make_pair(0., true); }

        for (int j = j0 - M; j <= j0 + M; ++j) {

            int k = j + d;
            // range check
            if (j < 0 || k < 0 || j >= w1 || k >= w2) {
                return std::make_pair(0., true);
            }
            int v1 = m1[i][j];
            int v2 = m2[i][k];
            if (v1 == 0 || v2 == 0) { return std::make_pair(0., true); }
            double dv = (double)(v1 - v2);
            mean += dv;
            SSD += (dv * dv);
        }
    }
    mean /= N;
    return std::make_pair(SSD / N - mean * mean, false);
}
