/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
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

#include <QPainter>
#include <QLineEdit>

QmitkPiecewiseFunctionCanvas::QmitkPiecewiseFunctionCanvas(QWidget * parent,
    Qt::WindowFlags f) :
  QmitkTransferFunctionCanvas(parent, f), m_PiecewiseFunction(0)
{
  // used for drawing a border
  setContentsMargins(1,1,1,1);
}

void QmitkPiecewiseFunctionCanvas::paintEvent(QPaintEvent*)
{
  QPainter painter(this);

  if (m_IsGradientOpacityFunction)
  {
    PaintHistogramGO(painter);
  }
  else
  {
    PaintHistogram(painter);
  }
  painter.save();
  painter.setPen(Qt::green);

  QRect contentsRect = this->contentsRect();
  painter.drawRect(0, 0, contentsRect.width()+1, contentsRect.height()+1);
  if (m_PiecewiseFunction)
  {
    vtkFloatingPointType* dp = m_PiecewiseFunction->GetDataPointer();
    for (int i = 0; i < m_PiecewiseFunction->GetSize(); i++)
    {
      //converts values (X/Y)of the TF-point to pixel locations on the canvas-point
      std::pair<int,int> point = this->FunctionToCanvas(std::make_pair(
          dp[i * 2], dp[i * 2 + 1]));
      if (i + 1 < m_PiecewiseFunction->GetSize())
      {
        std::pair<int,int> nextPoint = this->FunctionToCanvas(std::make_pair(
            dp[(i + 1) * 2], dp[(i + 1) * 2 + 1]));
        painter.setPen(Qt::black);
        painter.drawLine(point.first, point.second, nextPoint.first,
            nextPoint.second);
      }

      if (i == m_GrabbedHandle)
      {
        painter.setBrush(QBrush(Qt::red));

        if (m_LineEditAvailable)
        {
          //inserts X/Y values of the grabbed function point into QLineEdit
          m_XEdit->setText(QString::number(GetFunctionX(m_GrabbedHandle)));
          m_YEdit->setText(QString::number(GetFunctionY(m_GrabbedHandle)));
        }
      }
      else
      {
        painter.setBrush(QBrush(Qt::green));
      }
      painter.drawEllipse(point.first - 4, point.second - 4, 8, 8);
    }
    painter.setBrush(Qt::NoBrush);
  }
  else
  {
    painter.setPen(Qt::red);
    painter.drawRect(1, 1, contentsRect.width()-1, contentsRect.height()-1);
  }
  painter.restore();
}

int QmitkPiecewiseFunctionCanvas::GetNearHandle(int x, int y,
    unsigned int maxSquaredDistance)
{
  vtkFloatingPointType* dp = m_PiecewiseFunction->GetDataPointer();
  for (int i = 0; i < m_PiecewiseFunction->GetSize(); i++)
  {
    std::pair<int,int> point = this->FunctionToCanvas(std::make_pair(dp[i * 2],
        dp[i * 2 + 1]));
    if ((unsigned int) ((point.first - x) * (point.first - x) + (point.second
        - y) * (point.second - y)) <= maxSquaredDistance)
    {
      return i;

    }
  }
  return -1;
}

void QmitkPiecewiseFunctionCanvas::MoveFunctionPoint(int index,
    std::pair<vtkFloatingPointType,vtkFloatingPointType> pos)
{
  RemoveFunctionPoint(GetFunctionX(index));
  AddFunctionPoint(pos.first, pos.second);
  //std::cout<<" AddFunctionPoint x: "<<pos.first<<" AddFunctionPoint y: "<<pos.second<<std::endl;
}
