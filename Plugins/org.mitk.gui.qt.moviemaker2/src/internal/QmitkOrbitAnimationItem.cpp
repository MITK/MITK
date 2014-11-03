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

#include "QmitkOrbitAnimationItem.h"

QmitkOrbitAnimationItem::QmitkOrbitAnimationItem(double angle, bool reverse, double duration, double delay, bool startWithPrevious)
  : QmitkAnimationItem("Orbit", duration, delay, startWithPrevious)
{
  this->SetAngle(angle);
  this->SetReverse(reverse);
}

QmitkOrbitAnimationItem::~QmitkOrbitAnimationItem()
{
}

int QmitkOrbitAnimationItem::GetAngle() const
{
  return this->data(AngleRole).toInt();
}

void QmitkOrbitAnimationItem::SetAngle(int angle)
{
  this->setData(angle, AngleRole);
}

bool QmitkOrbitAnimationItem::GetReverse() const
{
  return this->data(ReverseRole).toBool();
}

void QmitkOrbitAnimationItem::SetReverse(bool reverse)
{
  this->setData(reverse, ReverseRole);
}
