#include <sstream>
#include <iostream>
#include <qpainter.h>
#include "QmitkTransferFunctionCanvas.h"
#include <assert.h>
#include "qcolordialog.h"

QmitkTransferFunctionCanvas::QmitkTransferFunctionCanvas(QWidget * parent, const char * name, WFlags f) : QWidget(parent, name, f), m_GrabbedHandle(NULL), m_GrabbedElement(NULL) {
  std::cout << "end of c'tor" << std::endl;
}

void QmitkTransferFunctionCanvas::PaintElementFunction(const ElementSetType &elements, QPainter &p,int penWidth) {
  QPointArray heights(width());
  std::vector<float> values(width());
  for (unsigned int i=0;i<values.size();i++) {
    float ix = (float)i / values.size();
    values[i]=0.0f ;
    for (ElementSetType::iterator elemIt = elements.begin(); elemIt != elements.end(); elemIt++ ) {
      values[i] = values[i] + (*elemIt)->GetValueAt(ix); 
    }
  }
  float maxValue = 0.0f;
  for (unsigned int i=0;i<values.size();i++) {
    maxValue = std::max(maxValue,values[i]);
  }
  // do not scale if not neccessary
  // maxValue  = std::min(1.0f,maxValue);
  p.save();
  for (unsigned int i=0;i<values.size();i++) {
    heights.setPoint(i, i,(int)(height() - (values[i]  /* / maxValue */ ) * height()));
    float red = 0, green = 0, blue = 0;
    for (ElementSetType::iterator elemIt = elements.begin(); elemIt != elements.end(); elemIt++ ) {
    float ix = (float)i / values.size();
    if (values[i]>0) { 
    red += ((*elemIt) -> m_Red) * (*elemIt)->GetValueAt(ix) / values[i] ;   
       green += ((*elemIt) -> m_Green) * (*elemIt)->GetValueAt(ix) / values[i] ;   
       blue += ((*elemIt) -> m_Blue) * (*elemIt)->GetValueAt(ix) / values[i] ;   
    }
    }
    if (i>0) {
      //std::cout << red << " " << green << " " << blue << std::endl;
      p.setPen(QPen(QColor((int)red,(int)green,(int)blue),penWidth));
      p.drawLine(heights.point(i-1),heights.point(i));
    }  
          
  }
  p.restore();
   // p.drawPolyline(heights);  
}


void QmitkTransferFunctionCanvas::paintEvent(QPaintEvent* ev) {
  QPainter paint( this );
  paint.save();
  paint.setPen(Qt::green);
  paint.drawRect(0,0,width(),height());
  // make sure the sum-function is somehow visible
  // paint.translate(0,-1.0);
//  paint.setPen(QPen(Qt::green,3));
  PaintElementFunction(m_Elements, paint, 3);
  paint.restore();
  for (ElementSetType::iterator elemIt = m_Elements.begin(); elemIt != m_Elements.end(); elemIt++ ) {
    ElementSetType element;
    element.insert(*elemIt);
    PaintElementFunction(element,paint);
    PaintElement(paint,*elemIt);
  }

  QWidget::paintEvent(ev);
} 
void QmitkTransferFunctionCanvas::mouseDoubleClickEvent(QMouseEvent* mouseEvent) {
  Element* elem = GetNearHandle(mouseEvent->x(),mouseEvent->y()).first;
  
  if (mouseEvent->button() & Qt::LeftButton && elem) {
    QColor result = QColorDialog::getColor(QColor(elem->m_Red,elem->m_Green,elem->m_Blue));
    if (result.isValid()) {
    elem->m_Red = result.red();
    elem->m_Green = result.green();
    elem->m_Blue = result.blue();
    this->update();
    }
  }
}

std::pair<QmitkTransferFunctionCanvas::Element*,QmitkTransferFunctionCanvas::Handle*> QmitkTransferFunctionCanvas::GetNearHandle(int x,int y,unsigned int maxSquaredDistance) {
  Element* nearElement = NULL;
  Handle* nearHandle = NULL;
  for (ElementSetType::iterator elemIt = m_Elements.begin(); !nearHandle && elemIt != m_Elements.end(); elemIt++ ) {
      for (Element::HandleSetType::iterator it = (*elemIt)->m_Handles.begin(); !nearHandle && it != (*elemIt)->m_Handles.end(); it++) {
        if (SquaredDistance(x,y,*it) < maxSquaredDistance) {
          nearElement = *elemIt;
          nearHandle = *it;
        }
    }
  }
  return std::make_pair(nearElement,nearHandle); 
}

void QmitkTransferFunctionCanvas::mousePressEvent( QMouseEvent* mouseEvent ) {
  if (( mouseEvent->state() & Qt::ShiftButton ) &&
      (mouseEvent->button() & Qt::LeftButton)) {
    // mode = add
    m_Elements.insert(new SimpleElement((float)(mouseEvent->x()) / width(), 1 - (float)(mouseEvent->y()) / height()));
  } else if ((mouseEvent->button() & Qt::LeftButton) || 
      (mouseEvent->button() & Qt::MidButton)) {

    m_GrabbedElement =GetNearHandle(mouseEvent->x(),mouseEvent->y()).first; 
    m_GrabbedHandle = GetNearHandle(mouseEvent->x(),mouseEvent->y()).second;  
    if (m_GrabbedHandle) { 
          m_MoveStart = std::make_pair(mouseEvent->x(),mouseEvent->y());
    }
    if ((mouseEvent->button() & Qt::MidButton)  && 
        (mouseEvent->state() & Qt::ShiftButton) && 
        m_GrabbedHandle) {
      // mode = delete
      m_Elements.erase(m_GrabbedElement);
      m_GrabbedElement = NULL;
      m_GrabbedHandle = NULL;

    }
  }
  update();
}

void QmitkTransferFunctionCanvas::mouseMoveEvent( QMouseEvent* mouseEvent ) {
  if (m_GrabbedHandle && m_GrabbedElement) {
    std::pair<int,int> moveDelta = std::make_pair(mouseEvent->x() - m_MoveStart.first,mouseEvent->y() - m_MoveStart.second);
    // float dx = (float)moveDelta.first / width();
    float dy = (float)moveDelta.second / height();

    if (/* TriElement* triElement =*/ dynamic_cast<TriElement*>(m_GrabbedElement)) {
      if (mouseEvent->state() & Qt::LeftButton) {
        //       einfach das Handle verschieben:
        m_GrabbedElement->Set(m_GrabbedHandle,(float)(mouseEvent->x()) / width(), 1 - (float)(mouseEvent->y()) / height());
      }
    }
    else if (SimpleElement* simpleElement = dynamic_cast<SimpleElement*>(m_GrabbedElement)) {
      if (mouseEvent->state() & Qt::LeftButton) {
        simpleElement->SetX((float)mouseEvent->x() / width()); 
        simpleElement->m_Window += dy;
      } else if(mouseEvent->state() & Qt::MidButton) {
        simpleElement->SetY(1.0 - (float)mouseEvent->y() / height());
      }   
    }

    m_MoveStart = std::make_pair(mouseEvent->x(),mouseEvent->y());
    update();

  }
}
  /*  Alter TriElement Code
       *             triElement->m_Handles[i], triElement->m_Handles[i]->GetPos().first + dx, triElement->m_Handles[i]->GetPos().second);
                    }
                    triElement->Set(triElement->GetRightHandle(),triElement->GetRightHandle()->GetPos().first + dy, triElement->GetRightHandle()->GetPos().second);
                    triElement->Set(triElement->GetLeftHandle(),triElement->GetLeftHandle()->GetPos().first - dy, triElement->GetLeftHandle()->GetPos().second);
                    } else if (mouseEvent->state() & Qt::MidButton){
                    triElement->Set(triElement->GetTopHandle(),triElement->GetTopHandle()->GetPos().first, triElement->GetTopHandle()->GetPos().second - dy);
                    triElement->Set(triElement->GetRightHandle(),triElement->GetRightHandle()->GetPos().first + dx, triElement->GetRightHandle()->GetPos().second);
                    triElement->Set(triElement->GetLeftHandle(),triElement->GetLeftHandle()->GetPos().first + dx, triElement->GetLeftHandle()->GetPos().second);
                    */
 

float QmitkTransferFunctionCanvas::SquaredDistance(int x, int y, Handle* handle) {
  float hx = handle->m_Pos.first * width();
  float hy = height() - handle->m_Pos.second * height();
  float distance2 = (hx-x)*(hx-x) + (hy-y)*(hy-y);
  return distance2; 
}

void QmitkTransferFunctionCanvas::mouseReleaseEvent( QMouseEvent*  ) {
  m_GrabbedHandle = NULL;
  update();
};

void QmitkTransferFunctionCanvas::PaintElement(QPainter &p,Element* element) {
    p.save();
  if (TriElement* elem = dynamic_cast<TriElement*>(element)) {
    p.setPen(Qt::green);
    p.drawLine(HandleToPoint(*elem->GetLeftHandle()), HandleToPoint(*elem->GetTopHandle())); 
    p.drawLine(HandleToPoint(*elem->GetTopHandle()), HandleToPoint(*elem->GetRightHandle())); 
    p.drawLine(HandleToPoint(*elem->GetLeftHandle()), HandleToPoint(*elem->GetRightHandle())); 
  }
  for (Element::HandleSetType::iterator it = element->m_Handles.begin(); it != element->m_Handles.end(); it++) {

    int ix = (int)((*it)->m_Pos.first * width());
    int iy = height() - (int)( (*it)->m_Pos.second * height());
    if (*it == m_GrabbedHandle) {
      p.setPen(Qt::red);
    } else {
      p.setPen(Qt::black);
    } 

    p.drawRect(ix-2,iy-2,5,5);
  }
    p.restore();
}
