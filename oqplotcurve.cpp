#include "oqplotcurve.h"
#include "qdebug.h"

OqPlotCurve::OqPlotCurve(const QString &title) : QwtPlotCurve(title) {}

OqPlotCurve::OqPlotCurve( const QwtText &title ) : QwtPlotCurve(title) {}

// Reimplement boundingRect to neglect Inf and NaN values:
QRectF OqPlotCurve::boundingRect() const
{
    QRectF origRect = dataRect();
    //qDebug() << "Original rect top left: " << origRect.topLeft();
    //qDebug() << "Original rect bottom right: " << origRect.bottomRight();

    QPointF topLeft = origRect.topLeft();
    QPointF bottomRight = origRect.bottomRight();

    bool isRealTopLeftX = false;
    bool isRealTopLeftY = false;
    bool isRealBottomRightX = false;
    bool isRealBottomRightY = false;

    if(!qIsNaN(origRect.topLeft().x()) && !qIsInf(origRect.topLeft().x())) {
        topLeft.setX(origRect.topLeft().x());
        isRealTopLeftX = true;
    }
    if(!qIsNaN(origRect.topLeft().y()) && !qIsInf(origRect.topLeft().y())) {
        topLeft.setY(origRect.topLeft().y());
        isRealTopLeftY = true;
    }
    if(!qIsNaN(origRect.bottomRight().x()) && !qIsInf(origRect.bottomRight().x())) {
        bottomRight.setX(origRect.bottomRight().x());
        isRealBottomRightX = true;
    }
    if(!qIsNaN(origRect.bottomRight().y()) && !qIsInf(origRect.bottomRight().y())) {
        bottomRight.setY(origRect.bottomRight().y());
        isRealBottomRightY = true;
    }
    if(isRealTopLeftX && isRealTopLeftY && isRealBottomRightX && isRealBottomRightY)
        return origRect;

    // If we are here, one or more of the corners is Inf or NaN
    const QwtSeriesData<QPointF> * series = data();

    for (int i= 0; i < series->size(); i++) {
        const QPointF sample = series->sample(i);
        if(!qIsNaN(sample.x()) && !qIsInf(sample.x())) {
            // First replace the NaN values with any real values:
            if(qIsNaN(topLeft.x()))
                topLeft.setX(sample.x());
            if(qIsNaN(bottomRight.x()))
                bottomRight.setX(sample.x());
            // Once we have real values, we can check for larger or smaller values:
            if(sample.x() > bottomRight.x())
                bottomRight.setX(sample.x());
            if(sample.x() < topLeft.x())
                topLeft.setX(sample.x());
        }
        if(!qIsNaN(sample.y()) && !qIsInf(sample.y())) {
            if(qIsNaN(topLeft.y()))
                topLeft.setY(sample.y());
            if(qIsNaN(bottomRight.y()))
                bottomRight.setY(sample.y());
            if(sample.y() > bottomRight.y())
                bottomRight.setY(sample.y());
            if(sample.y() < topLeft.y())
                topLeft.setY(sample.y());
        }
    }
    return QRectF(topLeft,bottomRight);
}

// Reimplement drawCurve to skip NaN values.
// Copied from here: https://sourceforge.net/p/qwt/feature-requests/47/
void OqPlotCurve::drawCurve(QPainter *p, int style, const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                           const QRectF &canvasRect, int from, int to) const
{
    const QwtSeriesData<QPointF> * series = data();
    int from_ = from;
    int to_   = from;
    bool lineStarted = false;
    for (int i = from; i <= to; ++i) {
        const QPointF sample = series->sample(i);
        if (qIsNaN(sample.x()) || qIsNaN(sample.y())) {
            if (lineStarted) {
                lineStarted = false;
                if (from_ != to_) {
                    QwtPlotCurve::drawCurve(p, style, xMap, yMap, canvasRect, from_, to_);
                }
            }
        } else {
            if ( ! lineStarted) {
                lineStarted = true;
                from_ = i;
            }
            to_ = i;
        }
    }
    if (lineStarted) {
        if (from_ != to_) {
            QwtPlotCurve::drawCurve(p, style, xMap, yMap, canvasRect, from_, to_);
        }
    }
}
