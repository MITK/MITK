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

#ifndef QMITKTRANSFERFUNCTIONCANVAS_H_INCLUDED
#define QMITKTRANSFERFUNCTIONCANVAS_H_INCLUDED

#include "MitkQtWidgetsExtExports.h"

#include <mitkSimpleHistogram.h>
#include <mitkRenderingManager.h>

#include <QWidget>
#include <QLineEdit>


class MitkQtWidgetsExt_EXPORT QmitkTransferFunctionCanvas : public QWidget
{

  Q_OBJECT

public:

  QmitkTransferFunctionCanvas( QWidget * parent=0, Qt::WindowFlags f = 0 );

  mitk::SimpleHistogram* GetHistogram()
  {
    return m_Histogram;
  }

  void SetHistogram(mitk::SimpleHistogram *histogram)
  {
    m_Histogram = histogram;
  }

  double GetMin()
  {
    return m_Min;
  }

  void SetMin(double min)
  {
    this->m_Min = min;
    SetLower(min);
  }

  double GetMax()
  {
    return m_Max;
  }

  void SetMax(double max)
  {
    this->m_Max = max;
    SetUpper(max);
  }

  double GetLower()
  {
    return m_Lower;
  }

  void SetLower(double lower)
  {
    this->m_Lower = lower;
  }

  double GetUpper()
  {
    return m_Upper;
  }

  void SetUpper(double upper)
  {
    this->m_Upper = upper;
  }

  void mousePressEvent( QMouseEvent* mouseEvent );
  virtual void paintEvent( QPaintEvent* e );
  virtual void DoubleClickOnHandle(int handle) = 0;

  void mouseMoveEvent( QMouseEvent* mouseEvent );
  void mouseReleaseEvent( QMouseEvent* mouseEvent );
  void mouseDoubleClickEvent( QMouseEvent* mouseEvent );
  void PaintHistogram(QPainter &p);

  virtual int GetNearHandle(int x,int y,unsigned int maxSquaredDistance = 32) = 0;
  virtual int AddFunctionPoint(double x,double val) = 0;
  virtual void RemoveFunctionPoint(double x) = 0;
  virtual void MoveFunctionPoint(int index, std::pair<double,double> pos) = 0;
  virtual double GetFunctionX(int index) = 0;
  virtual double GetFunctionY(int index) = 0;
  virtual int GetFunctionSize() = 0;
  int m_GrabbedHandle;

  double m_Lower, m_Upper, m_Min, m_Max;

  std::pair<int,int> FunctionToCanvas(std::pair<double,double>);
  std::pair<double,double> CanvasToFunction(std::pair<int,int>);


  mitk::SimpleHistogram *m_Histogram;

  void keyPressEvent ( QKeyEvent * e );

  void SetImmediateUpdate(bool state);

  std::pair<double,double> ValidateCoord( std::pair<double,double> x )
  {
    double max = m_Histogram->GetMax();
    double min = m_Histogram->GetMin();
    if( x.first < min ) x.first = min;
    if( x.first >  max ) x.first =  max;
    if( x.second < 0 ) x.second = 0;
    if( x.second > 1 ) x.second = 1;
    return x;
  }

  void SetX(float x)
  {
    if (m_GrabbedHandle != -1)
    {
      this->MoveFunctionPoint(m_GrabbedHandle, ValidateCoord(std::make_pair(x,GetFunctionY(m_GrabbedHandle))));
      update();
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }

  void SetY(float y)
  {
    if (m_GrabbedHandle != -1)
    {
      this->MoveFunctionPoint(m_GrabbedHandle, ValidateCoord(std::make_pair(GetFunctionX(m_GrabbedHandle),y)));
      update();
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }

  void SetQLineEdits(QLineEdit* xEdit, QLineEdit* yEdit)
  {
    m_XEdit = xEdit;
    m_YEdit = yEdit;
    m_LineEditAvailable = true;
  }

protected:
  bool m_ImmediateUpdate;
  float m_Range;

  bool m_LineEditAvailable;
  QLineEdit* m_XEdit;
  QLineEdit* m_YEdit;
};

#endif

