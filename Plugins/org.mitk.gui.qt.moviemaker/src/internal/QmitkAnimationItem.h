/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
    OrbitRole
  };

  explicit QmitkAnimationItem(const QString& widgetKey, double duration = 2.0, double delay = 0.0, bool startWithPrevious = false);
  ~QmitkAnimationItem() override;

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
