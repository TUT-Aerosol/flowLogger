#ifndef OQPLOTCURVE_H
#define OQPLOTCURVE_H

#include <qwt_plot_curve.h>
#include <qwt_plot_seriesitem.h>

class OqPlotCurve : public QwtPlotCurve
{
public:
    explicit OqPlotCurve( const QString &title = QString() );
    explicit OqPlotCurve( const QwtText &title );

    virtual QRectF boundingRect() const QWT_OVERRIDE;

protected:
    virtual void drawCurve(QPainter *p, int style, const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                           const QRectF &canvasRect, int from, int to) const override;
};

#endif // OQPLOTCURVE_H
