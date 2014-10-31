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
    DurationRole = Qt::UserRole + 2,
    DelayRole,
    StartWithPreviousRole
  };

  explicit QmitkAnimationItem(double duration, double delay = 0.0, bool startWithPrevious = false);
  ~QmitkAnimationItem();

  double GetDuration() const;
  void SetDuration(double duration);

  double GetDelay() const;
  void SetDelay(double delay);

  bool GetStartWithPrevious() const;
  void SetStartWithPrevious(bool startWithPrevious);
};

#endif
