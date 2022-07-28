#ifndef OQPANNER_H
#define OQPANNER_H

#include <qwt_plot_panner.h>

// Copied from: https://stackoverflow.com/questions/14747959/qwt-zoomer-plus-panner-with-continuous-replot

class OQPanner : public QwtPlotPanner {
public:
    explicit OQPanner(QWidget* parent, Qt::MouseButton mouseBtn);
    virtual bool eventFilter(QObject *object, QEvent *event);
private:
    bool isPressed;
    Qt::MouseButton originalButton = Qt::MouseButton::LeftButton;
};

#endif // CUSTOMPANNER_H
