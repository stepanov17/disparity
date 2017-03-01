#ifndef DISPARITYCALCULATOR_H
#define DISPARITYCALCULATOR_H

class QImage;

#include <vector>
#include <limits>
#include <algorithm>

typedef std::vector<std::vector<int> > intMatr_t;

class DisparityCalculator {

private:

    static const int M = 3; // half of the window size (use 7x7 window)
    static const int N = (2 * M + 1) * (2 * M + 1); // number of window points

    static const int TOLERANCE = 1; // tolerance to cut off undefined values

    /**
     * undefined values for disparity:
     */

    /**
     * corresponds to a case when abs(d1 - d2) > 1
     */
    static const int UNDEF = std::numeric_limits<int>::min();

    /**
     * row index for one of the windows goes outside of the corresponding image
     */
    static const int UNDEF_I = UNDEF + 1;

    /**
     * column index for one of the windows goes outside of the corresponding image
     */
    static const int UNDEF_J = UNDEF + 2;

    /**
     * zero intensity pixel found for one of the windows
     */
    static const int UNDEF_0 = UNDEF + 3;

    /**
     * arg min for the vector of doubles
     */
    static int argMin(const std::vector<double> &v) {
        return std::min_element(v.begin(), v.end()) - v.begin();
    }

    /**
     * response function at point (i0, j0)
     * @param m1 1st intensity matrix
     * @param m2 2nd intensity matrix
     * @param i0 row index
     * @param j0 column index
     * @return the response value
     */
    static std::pair<double, bool> r(const intMatr_t &m1,
                                     const intMatr_t &m2,
                                     int i0,
                                     int j0,
                                     int d);

    /**
     * disparity at point (i0, j0) for [dStart, dEnd] disparity range
     * @param m1 1st intensity matrix
     * @param m2 2nd intensity matrix
     * @param i0 row index
     * @param j0 column index
     * @param dStart disparity range start value
     * @param dEnd disparity range end value
     * @return the disparity value
     */
    static int getDisparity(const intMatr_t &m1,
                            const intMatr_t &m2,
                            int i0,
                            int j0,
                            int dStart,
                            int dEnd);

public:

    DisparityCalculator() = delete;

    /**
     * calculate a disparity map for a pair of grayscaled images
     * for a given disparity range
     *
     * @param img1 the 1st image
     * @param img2 the 2nd image
     * @param dStart the start value for disparity range
     * @param dEnd the end value for disparity range
     * @return the disparity map image
     */
    static QImage calculate(const QImage &img1,
                            const QImage &img2,
                            int dStart,
                            int dEnd);
};

#endif // DISPARITYCALCULATOR_H
