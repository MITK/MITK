#include "qcolordialog.h"
#include <sstream>
#include <iostream>
#include <qpainter.h>
#include "QmitkTransferFunctionCanvas.h"
#include <assert.h>
#include "mitkRenderWindow.h"
#include <cmath>

QmitkTransferFunctionCanvas::QmitkTransferFunctionCanvas(QWidget * parent, const char * name, WFlags f) : QWidget(parent, name, f), m_GrabbedHandle(NULL), m_GrabbedElement(NULL) {
  m_TransferFunction = mitk::TransferFunction::New();
}

void QmitkTransferFunctionCanvas::PaintElementFunction(const mitk::TransferFunction::ElementSetType &elements, QPainter &p,int penWidth) {
  
  std::vector<mitk::TransferFunction::RGBO> values(width());
  
  mitk::TransferFunction::FillValues(values, elements);
    
  QPointArray heights(width());
  
  float maxValue = 0.0f;
  for (unsigned int i=0;i<values.size();i++) {
    maxValue = std::max(maxValue,values[i].m_Opacity);
  }
  // do not scale if not neccessary
  // maxValue  = std::min(1.0f,maxValue);
  p.save();
  for (unsigned int i=0;i<values.size();i++) {
    heights.setPoint(i, i,(int)(height() - (values[i].m_Opacity  /* / maxValue */ ) * height()));
    
    if (i>0) {
      //std::cout << red << " " << green << " " << blue << std::endl;
      p.setPen(QPen(QColor(values[i].m_Red,values[i].m_Green,values[i].m_Blue),penWidth));
      p.drawLine(heights.point(i-1),heights.point(i));
    }  

  }
  p.restore();
  // p.drawPolyline(heights);  
}


void QmitkTransferFunctionCanvas::paintEvent(QPaintEvent* ev) {
  QPainter painter( this );
  
  PaintHistogram(painter);
  painter.save();
  painter.setPen(Qt::green);
  painter.drawRect(0,0,width(),height());
  // make sure the sum-function is somehow visible
  // painter.translate(0,-1.0);
  //  painter.setPen(QPen(Qt::green,3));
  PaintElementFunction(m_TransferFunction->GetElements(), painter, 3);
  painter.restore();
  for (mitk::TransferFunction::ElementSetType::iterator elemIt = m_TransferFunction->GetElements().begin(); elemIt != m_TransferFunction->GetElements().end(); elemIt++ ) {
    mitk::TransferFunction::ElementSetType element;
    element.insert(*elemIt);
    PaintElementFunction(element,painter);
    PaintElement(painter,*elemIt);
  }

  QWidget::paintEvent(ev);
} 
void QmitkTransferFunctionCanvas::mouseDoubleClickEvent(QMouseEvent* mouseEvent) {
  mitk::TransferFunction::Element* elem = GetNearHandle(mouseEvent->x(),mouseEvent->y()).first;

  if (mouseEvent->button() & Qt::LeftButton && elem) {
    QColor result = QColorDialog::getColor(QColor(elem->m_Red,elem->m_Green,elem->m_Blue));
    if (result.isValid()) {
      elem->m_Red = result.red();
      elem->m_Green = result.green();
      elem->m_Blue = result.blue();
      this->update();
      m_TransferFunction->UpdateVtkFunctions();
      mitk::RenderWindow::UpdateAllInstances();
  }
  }
}

std::pair<mitk::TransferFunction::Element*,mitk::TransferFunction::Handle*> QmitkTransferFunctionCanvas::GetNearHandle(int x,int y,unsigned int maxSquaredDistance) {
  mitk::TransferFunction::Element* nearElement = NULL;
  mitk::TransferFunction::Handle* nearHandle = NULL;
  for (mitk::TransferFunction::ElementSetType::iterator elemIt = m_TransferFunction->GetElements().begin(); !nearHandle && elemIt != m_TransferFunction->GetElements().end(); elemIt++ ) {
    for (mitk::TransferFunction::Element::HandleSetType::iterator it = (*elemIt)->m_Handles.begin(); !nearHandle && it != (*elemIt)->m_Handles.end(); it++) {
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
    m_TransferFunction->GetElements().insert(new mitk::TransferFunction::SimpleElement((float)(mouseEvent->x()) / width(), 1 - (float)(mouseEvent->y()) / height()));
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
      m_TransferFunction->GetElements().erase(m_GrabbedElement);
      m_GrabbedElement = NULL;
      m_GrabbedHandle = NULL;

    }
  }
  update();
  
  m_TransferFunction->UpdateVtkFunctions();
  mitk::RenderWindow::UpdateAllInstances();

}

void QmitkTransferFunctionCanvas::mouseMoveEvent( QMouseEvent* mouseEvent ) {
  if (m_GrabbedHandle && m_GrabbedElement) {
    std::pair<int,int> moveDelta = std::make_pair(mouseEvent->x() - m_MoveStart.first,mouseEvent->y() - m_MoveStart.second);
    // float dx = (float)moveDelta.first / width();
    float dy = (float)moveDelta.second / height();

    if (/* TriElement* triElement =*/ dynamic_cast<mitk::TransferFunction::TriElement*>(m_GrabbedElement)) {
      if (mouseEvent->state() & Qt::LeftButton) {
        //       einfach das mitk::TransferFunction::Handle verschieben:
        m_GrabbedElement->Set(m_GrabbedHandle,(float)(mouseEvent->x()) / width(), 1 - (float)(mouseEvent->y()) / height());
      }
    }
    else if (mitk::TransferFunction::SimpleElement* simpleElement = dynamic_cast<mitk::TransferFunction::SimpleElement*>(m_GrabbedElement)) {
      if (mouseEvent->state() & Qt::LeftButton) {
        simpleElement->SetX((float)mouseEvent->x() / width()); 
        simpleElement->m_Window += dy;
      } else if(mouseEvent->state() & Qt::MidButton) {
        simpleElement->SetY(1.0 - (float)mouseEvent->y() / height());
      }   
    }

    m_MoveStart = std::make_pair(mouseEvent->x(),mouseEvent->y());
    update();
    m_TransferFunction->UpdateVtkFunctions();
    mitk::RenderWindow::UpdateAllInstances();
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


float QmitkTransferFunctionCanvas::SquaredDistance(int x, int y, mitk::TransferFunction::Handle* handle) {
  float hx = handle->m_Pos.first * width();
  float hy = height() - handle->m_Pos.second * height();
  float distance2 = (hx-x)*(hx-x) + (hy-y)*(hy-y);
  return distance2; 
}
void QmitkTransferFunctionCanvas::PaintElement(QPainter &p,mitk::TransferFunction::Element* element) {
  p.save();
  if (mitk::TransferFunction::TriElement* elem = dynamic_cast<mitk::TransferFunction::TriElement*>(element)) {
    p.setPen(Qt::green);
    p.drawLine(HandleToPoint(*elem->GetLeftHandle()), HandleToPoint(*elem->GetTopHandle())); 
    p.drawLine(HandleToPoint(*elem->GetTopHandle()), HandleToPoint(*elem->GetRightHandle())); 
    p.drawLine(HandleToPoint(*elem->GetLeftHandle()), HandleToPoint(*elem->GetRightHandle())); 
  }
  for (mitk::TransferFunction::Element::HandleSetType::iterator it = element->m_Handles.begin(); it != element->m_Handles.end(); it++) {

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
};





void QmitkTransferFunctionCanvas::mouseReleaseEvent( QMouseEvent*  ) {
  m_GrabbedHandle = NULL;
  update();
};

void QmitkTransferFunctionCanvas::PaintHistogram(QPainter &p) {
  if (m_TransferFunction->GetHistogram().IsNotNull()) {
  p.save();
  p.setPen(Qt::blue);
  float scaleFactor = (float)(m_TransferFunction->GetHistogram()->GetSize()[0]) / width();
  float maxFreqLog = std::log(mitk::HistogramGenerator::CalculateMaximumFrequency(m_TransferFunction->GetHistogram()));
  for (unsigned int x = 0; x<width(); x++) {
    int tfIndex = x * scaleFactor;
    float freq = m_TransferFunction->GetHistogram()->GetFrequency(tfIndex);
    
    if (freq>0) {
      int y = (1 - std::log(freq) / maxFreqLog) * height();
      p.drawLine(x,height(),x,y);
    }
  }
  p.restore();
  }
}






