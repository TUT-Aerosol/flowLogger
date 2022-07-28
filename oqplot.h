#ifndef OQPLOT_H
#define OQPLOT_H
#include <QObject>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>
#include <qwt_legend.h>
#include <qdatetime.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include "oqpanner.h"
#include "oqplotcurve.h"
#include <QPen>

enum OQPlotType {PT_NORMAL, PT_DATETIME};

class OQPlot : public QObject
{
Q_OBJECT

public:
    OQPlot(QwtPlot *UIPlot, OQPlotType Type = PT_NORMAL, QString Title = "", QString xLabel = "", QString yLabel = "");
    ~OQPlot();

    bool AddCurve(QString Title = "", QString Unit = "", QColor Color = Qt::black, int yAxis = QwtPlot::yLeft, int precision = 0);
    bool ReplaceCurve(quint8 curveId, QString Title = "", QString Unit = "", QColor Color = Qt::black, int yAxis = QwtPlot::yLeft);

    bool AddDataSet(quint8 Curve, double *XData, double *YData, quint64 Count);

    bool AddPoint(quint8 Curve, QDateTime Time, double Value);

    void setLegendFormat(quint8 Curve, quint8 Precision);

    void ClearAllData(void);

    double getAxisMax(int axisId);
    double getAxisMin(int axisId);

    void autoScaleAxis(int axisId);
    void refresh();

    //bool isZoomed = false;
    void setAxisScale(int axisId, double min, double max);
    void setLogScale(int axisId, bool isLog);
    void RemoveFirstPoint(qint8 Curve);
    void setUnit(quint8 Curve, QString unitStr);

    int getCurveLength(qint8 Curve);
    void RemoveCurves();

    bool HideCurve(qint8 curveId);
    bool ShowCurve(qint8 curveId);
public slots:
    void zoomToRect(const QRectF &rect, int direction);
    void pannedSlot(int dx, int dy);
signals:
    void zoomedIn(bool isZoomed);
private:
    QwtPlot *Plot;
    QwtLegend *Legend;
    OQPlotType PlotType;
    quint8 CurveCount;
    QwtPlotPicker *rectZoomPicker;
    OQPanner *livePanner;

    struct OQCurves{
        //QwtPlotCurve *Curve;
        OqPlotCurve *Curve;
        QVector<double> xData;
        QVector<QDateTime> xTime;
        QVector<double> yData;
        QString Title;
        QString Unit;
        quint8 LegendValuePrecision;
    }Curves[10];
};

#endif // PLOT_H
