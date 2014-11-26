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

QmitkOrbitAnimationItem::QmitkOrbitAnimationItem(int startAngle, int orbit, bool reverse, double duration, double delay, bool startWithPrevious)
  : QmitkAnimationItem("Orbit", duration, delay, startWithPrevious)
{
  this->SetStartAngle(startAngle);
  this->SetOrbit(orbit);
  this->SetReverse(reverse);
}

QmitkOrbitAnimationItem::~QmitkOrbitAnimationItem()
{
}

int QmitkOrbitAnimationItem::GetStartAngle() const
{
  return this->data(StartAngleRole).toInt();
}

void QmitkOrbitAnimationItem::SetStartAngle(int angle)
{
  this->setData(angle, StartAngleRole);
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

  if (renderWindow == NULL)
    return;

  mitk::Stepper* stepper = mitk::BaseRenderer::GetInstance(renderWindow)->GetCameraRotationController()->GetSlice();

  if (stepper == NULL)
    return;

  int newPos = this->GetReverse()
    ? this->GetStartAngle() - this->GetOrbit() * s
    : this->GetStartAngle() + this->GetOrbit() * s;

  while (newPos < 0)
    newPos += 360;

  while (newPos > 360)
    newPos -= 360;

  stepper->SetPos(static_cast<unsigned int>(newPos));
}
