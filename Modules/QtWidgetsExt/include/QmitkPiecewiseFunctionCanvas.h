/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKPIECEWISEFUNCTIONCANVAS_H_INCLUDED
#define QMITKPIECEWISEFUNCTIONCANVAS_H_INCLUDED

#include "MitkQtWidgetsExtExports.h"
#include "QmitkTransferFunctionCanvas.h"

#include <vtkPiecewiseFunction.h>

class MITKQTWIDGETSEXT_EXPORT QmitkPiecewiseFunctionCanvas : public QmitkTransferFunctionCanvas
{
  Q_OBJECT

public:
  QmitkPiecewiseFunctionCanvas(QWidget *parent = nullptr, Qt::WindowFlags f = nullptr);
  void paintEvent(QPaintEvent *e) override;
  void SetTitle(const QString &title);
  int GetNearHandle(int x, int y, unsigned int maxSquaredDistance = 32) override;

  void SetPiecewiseFunction(vtkPiecewiseFunction *piecewiseFunction)
  {
    this->m_PiecewiseFunction = piecewiseFunction;
    this->SetMin(m_PiecewiseFunction->GetRange()[0]);
    this->SetMax(m_PiecewiseFunction->GetRange()[1]);
    setEnabled(true);

    update();
  }

  int AddFunctionPoint(double x, double val) override { return m_PiecewiseFunction->AddPoint(x, val); }
  void RemoveFunctionPoint(double x) override
  {
    int old_size = GetFunctionSize();
    m_PiecewiseFunction->RemovePoint(x);
    if (GetFunctionSize() + 1 != old_size)
    {
      std::cout << "old/new size" << old_size << "/" << GetFunctionSize() << std::endl;
      std::cout << "called with x=" << x << std::endl;
    }
  }

  double GetFunctionX(int index) override { return m_PiecewiseFunction->GetDataPointer()[index * 2]; }
  double GetFunctionY(int index) override
  {
    return m_PiecewiseFunction->GetValue(m_PiecewiseFunction->GetDataPointer()[index * 2]);
  }

  int GetFunctionSize() override { return m_PiecewiseFunction->GetSize(); }
  void DoubleClickOnHandle(int) override {}
  void MoveFunctionPoint(int index, std::pair<double, double> pos) override;

  double GetFunctionMax() { return m_PiecewiseFunction->GetRange()[1]; }
  double GetFunctionMin() { return m_PiecewiseFunction->GetRange()[0]; }
  double GetFunctionRange()
  {
    double range;
    if ((m_PiecewiseFunction->GetRange()[0]) < 0)
    {
      range = (m_PiecewiseFunction->GetRange()[1]) - (m_PiecewiseFunction->GetRange()[0]);
      return range;
    }
    else
    {
      range = m_PiecewiseFunction->GetRange()[1];
      return range;
    }
  }

  void RemoveAllFunctionPoints()
  {
    m_PiecewiseFunction->AddSegment(this->GetFunctionMin(), 0, this->GetFunctionMax(), 1);
    m_PiecewiseFunction->AddPoint(0.0, 0.0);
  }

  void ResetGO()
  { // Gradient Opacity
    m_PiecewiseFunction->AddSegment(this->GetFunctionMin(), 0, 0, 1);
    m_PiecewiseFunction->AddSegment(0, 1, ((this->GetFunctionRange()) * 0.125), 1);
    m_PiecewiseFunction->AddSegment(((this->GetFunctionRange()) * 0.125), 1, ((this->GetFunctionRange()) * 0.2), 1);
    m_PiecewiseFunction->AddSegment(((this->GetFunctionRange()) * 0.2), 1, ((this->GetFunctionRange()) * 0.25), 1);
  }

protected:
  vtkPiecewiseFunction *m_PiecewiseFunction;
  QString m_Title;
};

#endif
