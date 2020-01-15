/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTimeSliceAnimationItem.h"
#include <mitkBaseRenderer.h>

QmitkTimeSliceAnimationItem::QmitkTimeSliceAnimationItem(int from, int to, bool reverse, double duration, double delay, bool startWithPrevious)
  : QmitkAnimationItem("Time", duration, delay, startWithPrevious)
{
  this->SetFrom(from);
  this->SetTo(to);
  this->SetReverse(reverse);
}

QmitkTimeSliceAnimationItem::~QmitkTimeSliceAnimationItem()
{
}

int QmitkTimeSliceAnimationItem::GetFrom() const
{
  return this->data(FromRole).toInt();
}

void QmitkTimeSliceAnimationItem::SetFrom(int from)
{
  this->setData(from, FromRole);
}

int QmitkTimeSliceAnimationItem::GetTo() const
{
  return this->data(ToRole).toInt();
}

void QmitkTimeSliceAnimationItem::SetTo(int to)
{
  this->setData(to, ToRole);
}

bool QmitkTimeSliceAnimationItem::GetReverse() const
{
  return this->data(ReverseRole).toBool();
}

void QmitkTimeSliceAnimationItem::SetReverse(bool reverse)
{
  this->setData(reverse, ReverseRole);
}

void QmitkTimeSliceAnimationItem::Animate(double s)
{
  mitk::Stepper* stepper = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetTime();

  if (stepper == nullptr)
    return;

  int newPos = this->GetReverse()
    ? this->GetTo() - static_cast<int>((this->GetTo() - this->GetFrom()) * s)
    : this->GetFrom() + static_cast<int>((this->GetTo() - this->GetFrom()) * s);

  stepper->SetPos(static_cast<unsigned int>(newPos));
}
