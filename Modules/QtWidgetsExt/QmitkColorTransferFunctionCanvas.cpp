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

#include "QmitkColorTransferFunctionCanvas.h"

#include <QPainter>
#include <QLineEdit>
#include <QColorDialog>

QmitkColorTransferFunctionCanvas::QmitkColorTransferFunctionCanvas(
    QWidget * parent, Qt::WindowFlags f) :
  QmitkTransferFunctionCanvas(parent, f), m_ColorTransferFunction(0)
{
  // used for drawing a border
  setContentsMargins(1,1,1,1);
}

void QmitkColorTransferFunctionCanvas::SetTitle(const QString& title)
{
  m_Title=title;
}


void QmitkColorTransferFunctionCanvas::paintEvent(QPaintEvent*)
{
  QPainter painter(this);

  // Render gray background
  QRect contentsRect = this->contentsRect();
  painter.setPen(Qt::gray);
  painter.drawRect(0, 0, contentsRect.width()+1, contentsRect.height()+1);

  if( ! this->isEnabled() )
    return;

  if (m_ColorTransferFunction)
  {
    for (int x = contentsRect.x(); x < contentsRect.x() + contentsRect.width(); x++)
    {
      double xVal = m_Min + ((float) x) / contentsRect.width() * (m_Max
          - m_Min);
      QColor col((int) (m_ColorTransferFunction->GetRedValue(xVal) * 255),
          (int) (m_ColorTransferFunction->GetGreenValue(xVal) * 255),
          (int) (m_ColorTransferFunction->GetBlueValue(xVal) * 255));
      painter.setPen(col);
      painter.drawLine(x, 1, x, contentsRect.height());
    }
  }

 //paint title
  if (m_Title.size()>0)
  {
    painter.setPen(Qt::black);
    painter.drawText(QPoint(11,21),m_Title);
    painter.setPen(Qt::white);
    painter.drawText(QPoint(10,20),m_Title);
  }

  //paint min and max
  QString qs_min = QString::number( m_Min );
  QString qs_max = QString::number( m_Max );

  QRect qr_min = painter.fontMetrics().boundingRect( qs_min );
  QRect qr_max = painter.fontMetrics().boundingRect( qs_max );

  int y,x;

  y=this->contentsRect().height()-qr_min.height()+5;
  x=10;

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

  if (m_ColorTransferFunction)
  {
    // now paint the handles
    painter.setBrush(Qt::black);
    painter.setPen(Qt::black);
    for (int i = 0; i < this->GetFunctionSize(); i++)
    {
      int handleHeight = (i == m_GrabbedHandle) ? (int) (contentsRect.height() / 1.5)
          : contentsRect.height() / 2;
      int handleWidth = (i == m_GrabbedHandle) ? 6 : 4;
      std::pair<int,int> point = this->FunctionToCanvas(std::make_pair(
          GetFunctionX(i), 0.0f));
      int y = height() / 2;
      painter.drawRoundRect(point.first - handleWidth / 2,
          y - handleHeight / 2, handleWidth, handleHeight, 50, 50);

      if (i == m_GrabbedHandle && m_LineEditAvailable)
      {
        int xCursor = m_XEdit->cursorPosition();
        m_XEdit->setText(QString::number(GetFunctionX(m_GrabbedHandle), 'g', 4));
        m_XEdit->setCursorPosition( xCursor );
      }
    }
  }
}

int QmitkColorTransferFunctionCanvas::GetNearHandle(int x, int,
    unsigned int maxSquaredDistance)
{
  for (int i = 0; i < this->GetFunctionSize(); i++)
  {
    std::pair<int,int> point = this->FunctionToCanvas(std::make_pair(
        GetFunctionX(i), (double) 0.0));
    if ((unsigned int) ((point.first - x) * (point.first - x))
        < maxSquaredDistance)
    {
      return i;
    }
  }
  return -1;
}

void QmitkColorTransferFunctionCanvas::DoubleClickOnHandle(int handle)
{
  double xVal = GetFunctionX(handle);
  QColor col((int) (m_ColorTransferFunction->GetRedValue(xVal) * 255),
      (int) (m_ColorTransferFunction->GetGreenValue(xVal) * 255),
      (int) (m_ColorTransferFunction->GetBlueValue(xVal) * 255));
  QColor result = QColorDialog::getColor(col);
  if (result.isValid())
  {
    m_ColorTransferFunction->AddRGBPoint(xVal, result.red() / 255.0,
        result.green() / 255.0, result.blue() / 255.0);
    this->update();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkColorTransferFunctionCanvas::MoveFunctionPoint(int index,
    std::pair<double,double> pos)
{
  double color[3];
  m_ColorTransferFunction->GetColor(GetFunctionX(index), color);
  RemoveFunctionPoint( GetFunctionX(index));
  m_ColorTransferFunction->AddRGBPoint(pos.first, color[0], color[1], color[2]);
}

void QmitkColorTransferFunctionCanvas::AddRGB(double x, double r, double g,
    double b)
{
  m_ColorTransferFunction->AddRGBPoint(x, r, g, b);
}

