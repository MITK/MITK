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
#include <mitkBaseRenderer.h>

QmitkOrbitAnimationItem::QmitkOrbitAnimationItem(int orbit, bool reverse, double duration, double delay, bool startWithPrevious)
  : QmitkAnimationItem("Orbit", duration, delay, startWithPrevious)
{
  this->SetOrbit(orbit);
  this->SetReverse(reverse);
}

QmitkOrbitAnimationItem::~QmitkOrbitAnimationItem()
{
}

int QmitkOrbitAnimationItem::GetOrbit() const
{
  return this->data(OrbitRole).toInt();
}

void QmitkOrbitAnimationItem::SetOrbit(int angle)
{
  this->setData(angle, OrbitRole);
}

bool QmitkOrbitAnimationItem::GetReverse() const
{
  return this->data(ReverseRole).toBool();
}

void QmitkOrbitAnimationItem::SetReverse(bool reverse)
{
  this->setData(reverse, ReverseRole);
}

void QmitkOrbitAnimationItem::Animate(double s)
{
  vtkRenderWindow* renderWindow = mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4");

  if (renderWindow == nullptr)
    return;

  mitk::Stepper* stepper = mitk::BaseRenderer::GetInstance(renderWindow)->GetCameraRotationController()->GetSlice();

  if (stepper == nullptr)
    return;

  int newPos = this->GetReverse()
    ? 180 - this->GetOrbit() * s
    : 180 + this->GetOrbit() * s;

  while (newPos < 0)
    newPos += 360;

  while (newPos > 360)
    newPos -= 360;

  stepper->SetPos(static_cast<unsigned int>(newPos));
}
