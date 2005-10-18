#include "QmitkColorTransferFunctionCanvas.h"
#include <qpainter.h>
#include <qcolordialog.h>
#include <mitkRenderingManager.h>

QmitkColorTransferFunctionCanvas::QmitkColorTransferFunctionCanvas(QWidget * parent, const char * name, WFlags f) : QmitkTransferFunctionCanvas(parent,name,f), m_ColorTransferFunction(0) { 

}
void QmitkColorTransferFunctionCanvas::paintEvent( QPaintEvent* e ) {

  QPainter painter( this );
  painter.save();
  painter.setPen(Qt::green);
  painter.drawRect(0,0,width(),height());
  if (m_ColorTransferFunction) {
    for (int x=0; x< width(); x++)
    {
      vtkFloatingPointType xVal = m_Min + ((float)x)/width() * (m_Max - m_Min);    
      QColor col(
          (int)(m_ColorTransferFunction->GetRedValue(xVal) * 255), 
          (int)(m_ColorTransferFunction->GetGreenValue(xVal) * 255),
          (int)(m_ColorTransferFunction->GetBlueValue(xVal) * 255)
          );
      painter.setPen(col);
      painter.drawLine(x,0,x,height());
    }
    // now paint the handles
    painter.setBrush(Qt::black);
    painter.setPen(Qt::black);
    for (int i=0 ; i<this->GetFunctionSize(); i++) {
      int handleHeight = (i == m_GrabbedHandle) ? (int)(height() / 1.5) : height() / 2;
      int handleWidth= (i == m_GrabbedHandle) ? 6 : 4;
      std::pair<int,int> point = this->FunctionToCanvas(std::make_pair(GetFunctionX(i),0));
      int y = height() / 2;
      painter.drawRoundRect(point.first-handleWidth/2,y-handleHeight/2,handleWidth,handleHeight,50,50);
    }
  }
  painter.restore();
}
int QmitkColorTransferFunctionCanvas::GetNearHandle(int x,int y,unsigned int maxSquaredDistance)
{
  for (int i=0; i< this->GetFunctionSize(); i++)
  {
    std::pair<int,int> point = this->FunctionToCanvas(std::make_pair(GetFunctionX(i),(vtkFloatingPointType)0.0));
    if ((point.first-x)*(point.first-x) < maxSquaredDistance)
    {
      return i;
    }
  }
  return -1;
}

void QmitkColorTransferFunctionCanvas::DoubleClickOnHandle(int handle) {
  vtkFloatingPointType xVal = GetFunctionX(handle);    
  QColor col(
      (int)(m_ColorTransferFunction->GetRedValue(xVal) * 255), 
      (int)(m_ColorTransferFunction->GetGreenValue(xVal) * 255),
      (int)(m_ColorTransferFunction->GetBlueValue(xVal) * 255)
      );
  QColor result = QColorDialog::getColor(col);
  if (result.isValid()) {
    m_ColorTransferFunction->AddRGBPoint(xVal,result.red() / 255.0, result.green() / 255.0, result.blue() / 255.0);
    this->update();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkColorTransferFunctionCanvas::MoveFunctionPoint(int index, std::pair<vtkFloatingPointType,vtkFloatingPointType> pos) {
  vtkFloatingPointType color[3];
  m_ColorTransferFunction->GetColor(GetFunctionX(index),color);
  RemoveFunctionPoint(GetFunctionX(index));
  m_ColorTransferFunction->AddRGBPoint(pos.first,color[0],color[1],color[2]);
}
