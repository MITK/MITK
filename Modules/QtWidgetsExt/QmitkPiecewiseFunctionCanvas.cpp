/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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

void QmitkPiecewiseFunctionCanvas::SetTitle(const QString& title)
{
  m_Title=title;
}

void QmitkPiecewiseFunctionCanvas::paintEvent(QPaintEvent*)
{
  QPainter painter(this);

  PaintHistogram(painter);

  if (m_Title.size()>0)
  {
    painter.setPen(Qt::black);
    painter.drawText(QPoint(11,21),m_Title);
    painter.setPen(Qt::white);
    painter.drawText(QPoint(10,20),m_Title);
  }

  {
    QString qs_min = QString::number( m_Min, 'g', 4 );
    QString qs_max = QString::number( m_Max, 'g', 4 );

    QRect qr_min = painter.fontMetrics().boundingRect( qs_min );
    QRect qr_max = painter.fontMetrics().boundingRect( qs_max );

    int y,x;

    y=this->contentsRect().height()-qr_min.height()+5;
    x=10;

    // Fill the tf presets in the generator widget

    painter.setPen(Qt::black);
    painter.drawText(QPoint(x+1,y+1),qs_min);
    painter.setPen(Qt::white);
    painter.drawText(QPoint(x  ,y  ),qs_min);

    y=this->contentsRect().height()-qr_max.height()+5;
    x=this->contentsRect().width()-qr_max.width()-6;

    painter.setPen(Qt::black);
    painter.drawText(QPoint(x,y+1),qs_max);
    painter.setPen(Qt::white);
    painter.drawText(QPoint(x,y  ),qs_max);
  }

  painter.setPen(Qt::gray);

  QRect contentsRect = this->contentsRect();
  painter.drawRect(0, 0, contentsRect.width()+1, contentsRect.height()+1);
  if (m_PiecewiseFunction && this->isEnabled())
  {
    double* dp = m_PiecewiseFunction->GetDataPointer();

    // Render lines

    painter.setPen(Qt::black);

    for (int i = -1; i < m_PiecewiseFunction->GetSize(); i++)
    {
      std::pair<int,int> left;
      std::pair<int,int> right;

      if(i < 0)
        left = this->FunctionToCanvas(std::make_pair(-32768, dp[0 * 2 + 1]));
      else
        left = this->FunctionToCanvas(std::make_pair(dp[i * 2], dp[i * 2 + 1]));

      if(i+1 >= m_PiecewiseFunction->GetSize())
        right = this->FunctionToCanvas(std::make_pair(32768, dp[(i  ) * 2 + 1]));
      else
        right = this->FunctionToCanvas(std::make_pair(dp[(i+1) * 2], dp[(i+1) * 2 + 1]));

      painter.drawLine(left.first, left.second, right.first, right.second);
    }

    // Render Points

    for (int i = 0; i < m_PiecewiseFunction->GetSize(); i++)
    {
      std::pair<int,int> point = this->FunctionToCanvas(std::make_pair(
          dp[i * 2], dp[i * 2 + 1]));

      if (i == m_GrabbedHandle)
      {
        painter.setBrush(QBrush(Qt::red));
        if (m_LineEditAvailable)
        {
          int xCursor = m_XEdit->cursorPosition();
          int yCursor = m_YEdit->cursorPosition();
          m_XEdit->setText(QString::number(GetFunctionX(m_GrabbedHandle), 'g', 4));
          m_YEdit->setText(QString::number(GetFunctionY(m_GrabbedHandle), 'g', 4));
          m_XEdit->setCursorPosition( xCursor );
          m_YEdit->setCursorPosition( yCursor );
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
}

int QmitkPiecewiseFunctionCanvas::GetNearHandle(int x, int y,
    unsigned int maxSquaredDistance)
{
  double* dp = m_PiecewiseFunction->GetDataPointer();
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
    std::pair<double,double> pos)
{
  RemoveFunctionPoint(GetFunctionX(index));
  m_GrabbedHandle = AddFunctionPoint(pos.first, pos.second);
}
