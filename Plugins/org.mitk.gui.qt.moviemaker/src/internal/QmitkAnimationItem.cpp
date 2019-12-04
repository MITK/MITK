/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkAnimationItem.h"

QmitkAnimationItem::QmitkAnimationItem(const QString& widgetKey, double duration, double delay, bool startWithPrevious)
{
  this->SetWidgetKey(widgetKey);
  this->SetDuration(duration);
  this->SetDelay(delay);
  this->SetStartWithPrevious(startWithPrevious);

  this->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

QmitkAnimationItem::~QmitkAnimationItem()
{
}

QString QmitkAnimationItem::GetWidgetKey() const
{
  return this->data(WidgetKeyRole).toString();
}

void QmitkAnimationItem::SetWidgetKey(const QString& widgetKey)
{
  this->setData(widgetKey, WidgetKeyRole);
}

double QmitkAnimationItem::GetDuration() const
{
  return this->data(DurationRole).toDouble();
}

void QmitkAnimationItem::SetDuration(double duration)
{
  this->setData(duration, DurationRole);
}

double QmitkAnimationItem::GetDelay() const
{
  return this->data(DelayRole).toDouble();
}

void QmitkAnimationItem::SetDelay(double delay)
{
  this->setData(delay, DelayRole);
}

bool QmitkAnimationItem::GetStartWithPrevious() const
{
  return this->data(StartWithPreviousRole).toBool();
}

void QmitkAnimationItem::SetStartWithPrevious(bool startWithPrevious)
{
  this->setData(startWithPrevious, StartWithPreviousRole);
}
