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
