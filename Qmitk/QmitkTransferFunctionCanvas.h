#include <set>
#include <qwidget.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <math.h>
#include "mitkTransferFunction.h"


class QmitkTransferFunctionCanvas : public QWidget {

  Q_OBJECT

  public:

    QmitkTransferFunctionCanvas( QWidget * parent=0, const char * name=0, WFlags f = false );	

    // members of QmitkTransferFunctionCanvas follow
    void mousePressEvent( QMouseEvent* mouseEvent );

    virtual void paintEvent( QPaintEvent* e );

    void mouseMoveEvent( QMouseEvent* mouseEvent );
    void mouseReleaseEvent( QMouseEvent* mouseEvent );
    void mouseDoubleClickEvent( QMouseEvent* mouseEvent );
    virtual void PaintElement(QPainter &p,mitk::TransferFunction::Element* element);
    QPoint HandleToPoint(const mitk::TransferFunction::Handle &handle) const {
      int ix = (int)(handle.m_Pos.first * width());
      int iy = height() - (int)( handle.m_Pos.second * height());
      return QPoint(ix,iy);
    }
    void SetTransferFunction(mitk::TransferFunction* transferFunction) {
      this->m_TransferFunction = transferFunction;
      update();
    } 
    mitk::TransferFunction::Pointer m_TransferFunction; 
    void PaintElementFunction(const mitk::TransferFunction::ElementSetType &elements, QPainter &p,int penWidth = 0);

    std::pair<mitk::TransferFunction::Element*,mitk::TransferFunction::Handle*> GetNearHandle(int x,int y,unsigned
    int maxSquaredDistance = 6);
    float SquaredDistance(int x, int y, mitk::TransferFunction::Handle* handle);
    mitk::TransferFunction::Handle* m_GrabbedHandle;
    mitk::TransferFunction::Element* m_GrabbedElement;
    std::pair<int,int> m_MoveStart; 
};

