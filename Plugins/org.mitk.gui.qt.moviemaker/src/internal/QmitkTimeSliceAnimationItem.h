/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkTimeSliceAnimationItem_h
#define QmitkTimeSliceAnimationItem_h

#include "QmitkAnimationItem.h"

class QmitkTimeSliceAnimationItem : public QmitkAnimationItem
{
public:
  explicit QmitkTimeSliceAnimationItem(int from = 0, int to = 0, bool reverse = false, double duration = 2.0, double delay = 0.0, bool startWithPrevious = false);
  ~QmitkTimeSliceAnimationItem() override;

  int GetFrom() const;
  void SetFrom(int from);

  int GetTo() const;
  void SetTo(int to);

  bool GetReverse() const;
  void SetReverse(bool reverse);

  void Animate(double s) override;
};

#endif
