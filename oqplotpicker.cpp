#include "oqplotpicker.h"

OQPlotPicker::OQPlotPicker(int xAxis, int yAxis, QWidget *widget) : QwtPlotPicker(xAxis, yAxis, widget) {
}

bool OQPlotPicker::end( bool ok )
{
    ok = QwtPicker::end( ok );
    if ( !ok )
        return false;

    QwtPlot *plot = OQPlotPicker::plot();
    if ( !plot )
        return false;

    const QPolygon points = selection();
    if ( points.count() == 0 )
        return false;

    QwtPickerMachine::SelectionType selectionType =
        QwtPickerMachine::NoSelection;

    if ( stateMachine() )
        selectionType = stateMachine()->selectionType();

    switch ( selectionType )
    {
        case QwtPickerMachine::PointSelection:
        {
            const QPointF pos = invTransform( points.first() );
            Q_EMIT selected( pos );
            break;
        }
        case QwtPickerMachine::RectSelection:
        {
            if ( points.count() >= 2 )
            {
                const QPoint p1 = points.first();
                const QPoint p2 = points.last();
                int direction = OQPlotPicker::Left;
                if(p1.x() > p2.x())
                    direction = OQPlotPicker::Left;
                else
                    direction = OQPlotPicker::Right;

                const QRect rect = QRect( p1, p2 ).normalized();
                Q_EMIT selected( invTransform( rect ), direction );
            }
            break;
        }
        case QwtPickerMachine::PolygonSelection:
        {
            QVector<QPointF> dpa( points.count() );
            for ( int i = 0; i < points.count(); i++ )
                dpa[i] = invTransform( points[i] );

            Q_EMIT selected( dpa );
        }
        default:
            break;
    }

    return true;
}
