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

  if (stepper == NULL)
    return;

  int newPos = this->GetReverse()
    ? this->GetTo() - static_cast<int>((this->GetTo() - this->GetFrom()) * s)
    : this->GetFrom() + static_cast<int>((this->GetTo() - this->GetFrom()) * s);

  stepper->SetPos(static_cast<unsigned int>(newPos));
}
