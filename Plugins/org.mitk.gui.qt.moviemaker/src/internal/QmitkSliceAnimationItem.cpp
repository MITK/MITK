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

#include "QmitkSliceAnimationItem.h"
#include <mitkBaseRenderer.h>

QmitkSliceAnimationItem::QmitkSliceAnimationItem(int renderWindow, int from, int to, bool reverse, double duration, double delay, bool startWithPrevious)
  : QmitkAnimationItem("Slice", duration, delay, startWithPrevious)
{
  this->SetRenderWindow(renderWindow);
  this->SetFrom(from);
  this->SetTo(to);
  this->SetReverse(reverse);
}

QmitkSliceAnimationItem::~QmitkSliceAnimationItem()
{
}

int QmitkSliceAnimationItem::GetRenderWindow() const
{
  return this->data(RenderWindowRole).toInt();
}

void QmitkSliceAnimationItem::SetRenderWindow(int renderWindow)
{
  this->setData(renderWindow, RenderWindowRole);
}

int QmitkSliceAnimationItem::GetFrom() const
{
  return this->data(FromRole).toInt();
}

void QmitkSliceAnimationItem::SetFrom(int from)
{
  this->setData(from, FromRole);
}

int QmitkSliceAnimationItem::GetTo() const
{
  return this->data(ToRole).toInt();
}

void QmitkSliceAnimationItem::SetTo(int to)
{
  this->setData(to, ToRole);
}

bool QmitkSliceAnimationItem::GetReverse() const
{
  return this->data(ReverseRole).toBool();
}

void QmitkSliceAnimationItem::SetReverse(bool reverse)
{
  this->setData(reverse, ReverseRole);
}

void QmitkSliceAnimationItem::Animate(double s)
{
  const QString renderWindowName = QString("stdmulti.widget%1").arg(this->GetRenderWindow() + 1);
  vtkRenderWindow* renderWindow = mitk::BaseRenderer::GetRenderWindowByName(renderWindowName.toStdString());

  if (renderWindow == NULL)
    return;

  mitk::Stepper* stepper = mitk::BaseRenderer::GetInstance(renderWindow)->GetSliceNavigationController()->GetSlice();

  if (stepper == NULL)
    return;

  int newPos = this->GetReverse()
    ? this->GetTo() - static_cast<int>((this->GetTo() - this->GetFrom()) * s)
    : this->GetFrom() + static_cast<int>((this->GetTo() - this->GetFrom()) * s);

  stepper->SetPos(static_cast<unsigned int>(newPos));
}
