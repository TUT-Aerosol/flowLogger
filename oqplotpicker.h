#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <QPolygon>
#include <QPointF>

class OQPlotPicker : public QwtPlotPicker {
Q_OBJECT

public:
    OQPlotPicker(int xAxis, int yAxis, QWidget *);

    static const int Right = -1;
    static const int Left = 1;

Q_SIGNALS:
    // New signal:
    void selected(const QRectF &rect, int direction);


    /*!
      A signal emitted in case of QwtPickerMachine::PointSelection.
      \param pos Selected point
    */
    void selected( const QPointF &pos );

    /*!
      A signal emitted in case of QwtPickerMachine::RectSelection.
      \param rect Selected rectangle
    */
    void selected( const QRectF &rect );

    /*!
      A signal emitting the selected points,
      at the end of a selection.

      \param pa Selected points
    */
    void selected( const QVector<QPointF> &pa );

    /*!
      A signal emitted when a point has been appended to the selection

      \param pos Position of the appended point.
      \sa append(). moved()
    */
    void appended( const QPointF &pos );

    /*!
      A signal emitted whenever the last appended point of the
      selection has been moved.

      \param pos Position of the moved last point of the selection.
      \sa move(), appended()
    */
    void moved( const QPointF &pos );

protected:
    virtual bool end(bool ok = true); // Override the end function to emit the direction of selected rectangle
};
