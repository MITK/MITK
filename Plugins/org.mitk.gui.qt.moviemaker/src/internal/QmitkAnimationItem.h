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

#ifndef QmitkAnimationItem_h
#define QmitkAnimationItem_h

#include <QStandardItem>

class QmitkAnimationItem : public QStandardItem
{
public:
  enum AnimationItemDataRole
  {
    WidgetKeyRole = Qt::UserRole + 2,
    DurationRole,
    DelayRole,
    StartWithPreviousRole,
    RenderWindowRole,
    FromRole,
    ToRole,
    ReverseRole,
    StartAngleRole,
    OrbitRole
  };

  explicit QmitkAnimationItem(const QString& widgetKey, double duration = 2.0, double delay = 0.0, bool startWithPrevious = false);
  virtual ~QmitkAnimationItem();

  QString GetWidgetKey() const;
  void SetWidgetKey(const QString& widgetKey);

  double GetDuration() const;
  void SetDuration(double duration);

  double GetDelay() const;
  void SetDelay(double delay);

  bool GetStartWithPrevious() const;
  void SetStartWithPrevious(bool startWithPrevious);

  virtual void Animate(double s) = 0;
};

#endif
