#include "disparitycalculator.h"

#include <QPixmap>
#include <QColor>
#include <QDebug>

QImage DisparityCalculator::calculate(const QImage &img1,
                                      const QImage &img2,
                                      int dStart,
                                      int dEnd,
                                      int tolerance) {

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

            int d = getDisparity(m1, m2, i, j, dStart, dEnd, tolerance);

            QColor c = Qt::green;

            switch (d) {
            case UNDEF_I:
                break;
            case UNDEF_J:
                break;
            case UNDEF_0:
                c = Qt::cyan;
                break;
            case UNDEF:
                c = Qt::blue;
                break;
            default:
                double scale =
                    255. * (static_cast<double>(d) - dStart) / (dEnd - dStart);
                int g = static_cast<int>(scale);
                c = QColor(g, g, g);
                break;
            }

            if (UNDEF_I == d || UNDEF_J == d) {
                c = Qt::green;
            } else if (UNDEF_0 == d) {
                c = Qt::cyan;
            } else if (UNDEF == d) {
                c = Qt::blue;
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
                                      int dEnd,
                                      int tolerance) {

    int h1 = m1.size(), w1 = m1[0].size(),
        h2 = m2.size(), w2 = m2[0].size();

    // === row checks ===
    if (i0 < M) { return UNDEF_I; }
    if (i0 > h1 - M - 1) { return UNDEF_I; }
    if (i0 > h2 - M - 1) { return UNDEF_I; }

    // === column checks, 1 -> 2 ===
    // 1st image
    if (j0 < M) { return UNDEF_J; }
    if (j0 > w1 - M - 1) { return UNDEF_J; }

    // 2nd image
    int jStart = j0 + dStart;
    int jEnd   = j0 + dEnd;
    int start = std::min<int>(jStart, jEnd);
    int end   = std::max<int>(jStart, jEnd);

    if (start < M) { return UNDEF_J; }
    if (end > w2 - M - 1) { return UNDEF_J; }

    // === column checks, 2 -> 1 ===
    jStart = j0 - dEnd;
    jEnd   = j0 - dStart;
    start = std::min<int>(jStart, jEnd);
    end   = std::max<int>(jStart, jEnd);

    if (start < M) { return UNDEF_J; }
    if (end > w2 - M - 1) { return UNDEF_J; }


    int nR = std::max(dStart, dEnd) - std::min(dStart, dEnd) + 1;
    std::vector<double> R(nR, 0.);

    for (int d = dStart; d <= dEnd; ++d) {
        auto res = r(m1, m2, i0, j0, d); // m1 -> m2
        if (res.second) { return UNDEF_0; }
        else { R[d - dStart] = res.first; }
    }
    int d1 = dStart + argMin(R);

    for (int d = -dEnd; d <= -dStart; ++d) {
        auto res = r(m2, m1, i0, j0, d); // m2 -> m1
        if (res.second) { return UNDEF_0; }
        else { R[d + dEnd] = res.first; }
    }
    int d2 = -dEnd + argMin(R);

    if (std::abs(d1 + d2) > tolerance) { return UNDEF; }
    return d1;
}

std::pair<double, bool> DisparityCalculator::r(const intMatr_t &m1,
                                               const intMatr_t &m2,
                                               int i0,
                                               int j0,
                                               int d) {
    double SSD = 0., mean = 0.;
    for (int i = i0 - M; i <= i0 + M; ++i) {
        for (int j = j0 - M; j <= j0 + M; ++j) {
            int v1 = m1[i][j];
            int v2 = m2[i][j + d];
            if (v1 == 0 || v2 == 0) { return std::make_pair(0., true); }
            double dv = (double)(v1 - v2);
            mean += dv;
            SSD += (dv * dv);
        }
    }
    mean /= N;
    return std::make_pair(SSD / N - mean * mean, false);
}
