/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkPiecewiseFunctionCanvas.h>

QmitkPiecewiseFunctionCanvas::QmitkPiecewiseFunctionCanvas(QWidget *parent, Qt::WindowFlags f)
  : QmitkTransferFunctionCanvas(parent, f), m_PiecewiseFunction(nullptr)
{
  // used for drawing a border
  setContentsMargins(1, 1, 1, 1);
}

int QmitkPiecewiseFunctionCanvas::GetNearHandle(int x, int y, unsigned int maxSquaredDistance)
{
  double *dp = m_PiecewiseFunction->GetDataPointer();
  for (int i = 0; i < m_PiecewiseFunction->GetSize(); i++)
  {
    std::pair<int, int> point = this->FunctionToCanvas(std::make_pair(dp[i * 2], dp[i * 2 + 1]));
    if ((unsigned int)((point.first - x) * (point.first - x) + (point.second - y) * (point.second - y)) <=
        maxSquaredDistance)
    {
      return i;
    }
  }
  return -1;
}

void QmitkPiecewiseFunctionCanvas::MoveFunctionPoint(int index, std::pair<double, double> pos)
{
  RemoveFunctionPoint(GetFunctionX(index));
  m_GrabbedHandle = AddFunctionPoint(pos.first, pos.second);
}
