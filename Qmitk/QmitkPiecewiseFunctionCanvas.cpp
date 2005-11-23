/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkPiecewiseFunctionCanvas.h"
#include <qpainter.h>

QmitkPiecewiseFunctionCanvas::QmitkPiecewiseFunctionCanvas(QWidget * parent, const char * name, WFlags f) : QmitkTransferFunctionCanvas(parent,name,f), m_PiecewiseFunction(0) { 

}
void QmitkPiecewiseFunctionCanvas::paintEvent( QPaintEvent* e ) {

  QPainter painter( this );
  PaintHistogram(painter);
  painter.save();
  painter.setPen(Qt::green);
  painter.drawRect(0,0,width(),height());
  if (m_PiecewiseFunction) {
    vtkFloatingPointType* dp = m_PiecewiseFunction->GetDataPointer();
    for (int i=0; i< m_PiecewiseFunction->GetSize(); i++)
    {
      std::pair<int,int> point = this->FunctionToCanvas(std::make_pair(dp[i*2],dp[i*2+1]));
      if (i+1 < m_PiecewiseFunction->GetSize())
      {
        std::pair<int,int> nextPoint = this->FunctionToCanvas(std::make_pair(dp[(i+1)*2],dp[(i+1)*2+1]));
        painter.setPen( Qt::black );
        painter.drawLine(point.first,point.second,nextPoint.first,nextPoint.second);
      }
      painter.setPen( Qt::black );
      painter.setBrush( (i == m_GrabbedHandle) ? QBrush(Qt::red) : QBrush(Qt::green));
      painter.drawEllipse(point.first-4,point.second-4,8,8);
    }
    painter.setBrush(Qt::NoBrush);
  } else {

    painter.setPen(Qt::red);
    painter.drawRect(1,1,width()-2,height()-2);
  }
  painter.restore();
}
int QmitkPiecewiseFunctionCanvas::GetNearHandle(int x,int y,unsigned int maxSquaredDistance)
{
  vtkFloatingPointType* dp = m_PiecewiseFunction->GetDataPointer();
  for (int i=0; i< m_PiecewiseFunction->GetSize(); i++)
  {
    std::pair<int,int> point = this->FunctionToCanvas(std::make_pair(dp[i*2],dp[i*2+1]));
    if ((point.first-x)*(point.first-x)+(point.second-y)*(point.second-y) < maxSquaredDistance)
    {
      return i;
    }
  }
  return -1;
}

void QmitkPiecewiseFunctionCanvas::MoveFunctionPoint(int index, std::pair<vtkFloatingPointType,vtkFloatingPointType> pos) {
  RemoveFunctionPoint(GetFunctionX(index));
  AddFunctionPoint(pos.first,pos.second);
}
