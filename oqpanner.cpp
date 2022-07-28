#include "oqpanner.h"
#include <qevent.h>
#include <QDebug>

OQPanner::OQPanner(QWidget *parent, Qt::MouseButton mouseBtn) : QwtPlotPanner(parent) {
    isPressed = false;
    setMouseButton(mouseBtn);
    originalButton = mouseBtn;
}


bool OQPanner::eventFilter( QObject * object, QEvent * event)
{
    if ( object == NULL || object != parentWidget() )
            return false;

    switch ( event->type() )
    {
        case QEvent::MouseButtonPress:
        {
            QMouseEvent * evr = static_cast<QMouseEvent *>( event );
            if(evr->button() == originalButton) {
                isPressed = true;
            }
            widgetMousePressEvent( static_cast<QMouseEvent *>( event ) );
            break;
        }
        case QEvent::MouseMove:
        {
            if(!isPressed)
                break;
            QMouseEvent * evr = static_cast<QMouseEvent *>( event );
            widgetMouseMoveEvent(evr);
            widgetMouseReleaseEvent(evr);
            setMouseButton(evr->button(), evr->modifiers());
            widgetMousePressEvent(evr);
            break;
        }
        case QEvent::MouseButtonRelease:
        {
            QMouseEvent * evr = static_cast<QMouseEvent *>( event );
            widgetMouseReleaseEvent(evr);
            isPressed = false;
            setMouseButton(originalButton);
        }
        case QEvent::KeyPress:
        {
            //widgetKeyPressEvent( static_cast<QKeyEvent *>( event ) );
            break;
        }
        case QEvent::KeyRelease:
        {
            //widgetKeyReleaseEvent( static_cast<QKeyEvent *>( event ) );
            break;
        }
        case QEvent::Paint:
        {
            if ( isVisible() )
                return true;
            break;
        }
        default:;
    }

    return false;
}

