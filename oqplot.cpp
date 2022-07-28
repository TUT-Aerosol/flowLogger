#include "oqplot.h"
#include <QDebug>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_draw.h>
#include <qwt_date_scale_draw.h>
#include <qwt_date_scale_engine.h>
#include <qwt_legend.h>
#include <qwt_plot_dict.h>
#include <qwt_plot_item.h>
#include "oqplotpicker.h"
#include "oqplotcurve.h"

OQPlot::OQPlot(QwtPlot *UIPlot, OQPlotType Type, QString Title, QString xLabel, QString yLabel){

    CurveCount = 0;
    Plot = UIPlot;

    PlotType = Type;

    Plot->setCanvasBackground(QColor(Qt::white));
    Plot->setFrameStyle(QFrame::Box | QFrame::Sunken);

    Legend = new QwtLegend;
    Plot->setFrameStyle(QFrame::Box | QFrame::Sunken);
    Plot->insertLegend(Legend, QwtPlot::RightLegend);

    Plot->setTitle(Title);
    Plot->setAxisTitle(QwtPlot::xBottom, xLabel);
    Plot->setAxisTitle(QwtPlot::yLeft, yLabel);

    rectZoomPicker = new OQPlotPicker(Plot->xBottom,Plot->yLeft,Plot->canvas());
    rectZoomPicker->setStateMachine(new QwtPickerDragRectMachine);
    rectZoomPicker->setTrackerMode(QwtPicker::AlwaysOff);
    rectZoomPicker->setRubberBand(QwtPicker::RectRubberBand);
    rectZoomPicker->setRubberBandPen(QColor(Qt::black));

    livePanner = new OQPanner(Plot->canvas(),Qt::MouseButton::RightButton);

    connect(rectZoomPicker,SIGNAL(selected(const QRectF &, int)),this,SLOT(zoomToRect(const QRectF &, int)));
    connect(livePanner,SIGNAL(panned(int,int)),this,SLOT(pannedSlot(int,int)));
}


OQPlot::~OQPlot(){
    for (int i = 0; i < CurveCount; i++) delete Curves[i].Curve;
    delete Legend;
    delete rectZoomPicker;
    delete livePanner;
}


bool OQPlot::AddCurve(QString Title, QString Unit, QColor Color, int yAxis, int precision){

    if (CurveCount >= 10) return false;

    //Curves[CurveCount].Curve = new QwtPlotCurve(Title);
    Curves[CurveCount].Curve = new OqPlotCurve(Title);
    Curves[CurveCount].Title = Title;
    Curves[CurveCount].Unit = Unit;
    Curves[CurveCount].Curve->setPen(Color, 2, Qt::SolidLine);
    Curves[CurveCount].LegendValuePrecision = precision;
    Curves[CurveCount].Curve->attach(Plot);
    Curves[CurveCount].Curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    Curves[CurveCount].Curve->setYAxis(yAxis);

    if(yAxis == QwtPlot::yRight) {
        Plot->enableAxis(QwtPlot::yRight);
    }

    Plot->replot();
    CurveCount++;

    return true;
}

bool OQPlot::ReplaceCurve(quint8 curveId, QString Title, QString Unit, QColor Color, int yAxis){

    if (curveId > CurveCount) return false;

    Curves[curveId].Curve = new OqPlotCurve(Title);
    Curves[curveId].Title = Title;
    Curves[curveId].Unit = Unit;
    Curves[curveId].Curve->setPen(Color, 2, Qt::SolidLine);
    Curves[curveId].LegendValuePrecision = 0;
    Curves[curveId].Curve->attach(Plot);
    Curves[curveId].Curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    Curves[curveId].Curve->setYAxis(yAxis);

    if(yAxis == QwtPlot::yRight) {
        Plot->enableAxis(QwtPlot::yRight);
    }

    Plot->replot();

    return true;
}

bool OQPlot::HideCurve(qint8 curveId) {
    if(curveId > CurveCount) return false;

    Curves[curveId].Curve->detach();
    Plot->replot();
    return true;
}

bool OQPlot::ShowCurve(qint8 curveId) {
    if(curveId > CurveCount) return false;

    Curves[curveId].Curve->attach(Plot);
    Plot->replot();
    return true;
}

void OQPlot::RemoveCurves() {
    if(CurveCount == 0)
        return;

    for(int i=CurveCount; i > 0; i--) {
        Curves[CurveCount].Curve->detach();
        CurveCount --;
    }
}

double OQPlot::getAxisMax(int axisId) {
    return this->Plot->axisScaleDiv(axisId).upperBound();
}

double OQPlot::getAxisMin(int axisId) {
    return this->Plot->axisScaleDiv(axisId).lowerBound();
}

void OQPlot::zoomToRect(const QRectF &rect, int direction)
{

    if(direction == OQPlotPicker::Right && rect.topLeft() != rect.bottomRight()) {
        double yLeftMax = getAxisMax(QwtPlot::yLeft);
        double yLeftMin = getAxisMin(QwtPlot::yLeft);
        Plot->setAxisAutoScale(QwtPlot::xBottom,false);
        Plot->setAxisAutoScale(QwtPlot::yLeft,false);
        Plot->setAxisScale(QwtPlot::xBottom, rect.topLeft().x(), rect.topRight().x());
        // Top and bottom according to Qt coordinate system...
        Plot->setAxisScale(QwtPlot::yLeft, rect.topLeft().y(), rect.bottomLeft().y());
        if(Plot->axisEnabled(QwtPlot::yRight)) {
            double yRightMax = getAxisMax(QwtPlot::yRight);
            double yRightMin = getAxisMin(QwtPlot::yRight);
            Plot->setAxisAutoScale(QwtPlot::yRight,false);
            double correctedYMax = yRightMin + (rect.bottomLeft().y()-yLeftMin)/(yLeftMax-yLeftMin)*(yRightMax-yRightMin);
            double correctedYMin = yRightMin + (rect.topLeft().y()-yLeftMin)/(yLeftMax-yLeftMin)*(yRightMax-yRightMin);
            Plot->setAxisScale(QwtPlot::yRight, correctedYMin, correctedYMax);
        }
        Plot->replot();
        //isZoomed = true;
        emit zoomedIn(true);
    }
    else if(rect.topLeft() != rect.bottomRight()) {
        Plot->setAxisAutoScale(QwtPlot::xBottom,true);
        Plot->setAxisAutoScale(QwtPlot::yLeft,true);
        if(Plot->axisEnabled(QwtPlot::yRight)) {
            Plot->setAxisAutoScale(QwtPlot::yRight,true);
        }
        Plot->replot();
        //isZoomed = false;
        emit zoomedIn(false);
    }
}

void OQPlot::pannedSlot(int dx, int dy) {
    Q_UNUSED(dx);
    Q_UNUSED(dy);

    emit zoomedIn(true);
}

void OQPlot::autoScaleAxis(int axisId) {
    Plot->setAxisAutoScale(axisId,true);
    Plot->replot();
}

void OQPlot::setAxisScale(int axisId, double min, double max) {
    Plot->setAxisAutoScale(axisId,false);
    Plot->setAxisScale(axisId,min,max);
    Plot->replot();
}
void OQPlot::refresh() {
    Plot->replot();
}

bool OQPlot::AddDataSet(quint8 Curve, double *XData, double *YData, quint64 Count){

    if (PlotType == PT_NORMAL) return false;

    Curves[Curve].Curve->setSamples(XData, YData, Count);
    Plot->replot();

    return true;
}

void OQPlot::setLogScale(int axisId, bool isLog) {
    if(isLog)
        Plot->setAxisScaleEngine(axisId, new QwtLogScaleEngine());
    else
        Plot->setAxisScaleEngine(axisId, new QwtLinearScaleEngine());
}

bool OQPlot::AddPoint(quint8 Curve, QDateTime Time, double Value){

    Curves[Curve].xTime.append(Time);
    Curves[Curve].xData.append(QwtDate::toDouble(Time));
    Curves[Curve].yData.append(Value);

    Curves[Curve].Curve->setSamples(Curves[Curve].xData, Curves[Curve].yData);
    QwtDateScaleDraw *ScaleDraw = new QwtDateScaleDraw(Qt::LocalTime);

    if (Curves[Curve].xTime[0].msecsTo(Curves[Curve].xTime[Curves[Curve].xTime.length()-1]) < 5000){
            //ScaleDraw->setDateFormat(QwtDate::Millisecond, QString("hh:mm:ss\ndd.MM.yyyy"));
        ScaleDraw->setDateFormat(QwtDate::Millisecond, QString("hh:mm:ss"));
    }
    else
            ScaleDraw->setDateFormat(QwtDate::Second, QString("hh:mm:ss"));

    Curves[Curve].Curve->setTitle(Curves[Curve].Title + ": " + QString::number(Value, 'f', Curves[Curve].LegendValuePrecision) + " " + Curves[Curve].Unit);

    Plot->setAxisScaleDraw(QwtPlot::xBottom, ScaleDraw);
    Plot->setAxisScaleEngine(QwtPlot::xBottom, new QwtDateScaleEngine());
    Plot->setAxisLabelRotation(QwtPlot::xBottom, -45);
    Plot->setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);
    Plot->replot();

    return true;
}

void OQPlot::RemoveFirstPoint(qint8 Curve) {
    Curves[Curve].xTime.removeFirst();
    Curves[Curve].xData.removeFirst();
    Curves[Curve].yData.removeFirst();
    Curves[Curve].Curve->setSamples(Curves[Curve].xData, Curves[Curve].yData);

    // No need to update and replot, because it will be done when adding points.
}


void OQPlot::setLegendFormat(quint8 Curve, quint8 Precision){

    Curves[Curve].LegendValuePrecision = Precision;

}

void OQPlot::setUnit(quint8 Curve, QString unitStr) {
    Curves[Curve].Unit = unitStr;
}


 void OQPlot::ClearAllData(void){
     for (int i = 0; i < 10; i++){
         Curves[i].xData.clear();
         Curves[i].xTime.clear();
         Curves[i].yData.clear();
     }
 }

 int OQPlot::getCurveLength(qint8 Curve) {
     return Curves[Curve].xData.length();
 }

 qint8 OQPlot::getNumCurves() {
     return CurveCount;
 }
