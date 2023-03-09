/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkColorTransferFunctionCanvas_h
#define QmitkColorTransferFunctionCanvas_h

#include "MitkQtWidgetsExtExports.h"
#include "QmitkTransferFunctionCanvas.h"

#include <vtkColorTransferFunction.h>

class MITKQTWIDGETSEXT_EXPORT QmitkColorTransferFunctionCanvas : public QmitkTransferFunctionCanvas
{
  Q_OBJECT

public:
  QmitkColorTransferFunctionCanvas(QWidget *parent = nullptr, Qt::WindowFlags f = nullptr);
  void paintEvent(QPaintEvent *e) override;
  int GetNearHandle(int x, int y, unsigned int maxSquaredDistance = 32) override;
  void SetTitle(const QString &title);

  void SetColorTransferFunction(vtkColorTransferFunction *colorTransferFunction)
  {
    this->m_ColorTransferFunction = colorTransferFunction;
    this->SetMin(colorTransferFunction->GetRange()[0]);
    this->SetMax(colorTransferFunction->GetRange()[1]);
    setEnabled(true);
    update();
  }

  int AddFunctionPoint(double x, double) override
  {
    return m_ColorTransferFunction->AddRGBPoint(x,
                                                m_ColorTransferFunction->GetRedValue(x),
                                                m_ColorTransferFunction->GetGreenValue(x),
                                                m_ColorTransferFunction->GetBlueValue(x));
  }

  void RemoveFunctionPoint(double x) override
  {
    int old_size = GetFunctionSize();
    m_ColorTransferFunction->RemovePoint(x);
    if (GetFunctionSize() + 1 != old_size)
    {
      std::cout << "old/new size" << old_size << "/" << GetFunctionSize() << std::endl;
      std::cout << "called with x=" << x << std::endl;
    }
  }

  double GetFunctionX(int index) override { return m_ColorTransferFunction->GetDataPointer()[index * 4]; }
  int GetFunctionSize() override { return m_ColorTransferFunction->GetSize(); }
  void DoubleClickOnHandle(int handle) override;
  void MoveFunctionPoint(int index, std::pair<double, double> pos) override;

  void AddRGB(double x, double r, double g, double b);

  double GetFunctionMax() { return m_ColorTransferFunction->GetRange()[1]; }
  double GetFunctionMin() { return m_ColorTransferFunction->GetRange()[0]; }
  double GetFunctionRange()
  {
    double range;
    if ((m_ColorTransferFunction->GetRange()[0]) == 0)
    {
      range = m_ColorTransferFunction->GetRange()[1];
      return range;
    }
    else
    {
      range = (m_ColorTransferFunction->GetRange()[1]) - (m_ColorTransferFunction->GetRange()[0]);
      return range;
    }
  }

  void RemoveAllFunctionPoints()
  {
    m_ColorTransferFunction->AddRGBSegment(this->GetFunctionMin(), 1, 0, 0, this->GetFunctionMax(), 1, 1, 0);
  }

  double GetFunctionY(int) override { return 0.0; }
protected:
  vtkColorTransferFunction *m_ColorTransferFunction;
  QString m_Title;
};

#endif
