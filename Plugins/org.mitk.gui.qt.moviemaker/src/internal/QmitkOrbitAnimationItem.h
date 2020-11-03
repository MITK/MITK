/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkOrbitAnimationItem_h
#define QmitkOrbitAnimationItem_h

#include "QmitkAnimationItem.h"

class QmitkOrbitAnimationItem : public QmitkAnimationItem
{
public:
  explicit QmitkOrbitAnimationItem(int orbit = 360, bool reverse = false, double duration = 2.0, double delay = 0.0, bool startWithPrevious = false);
  ~QmitkOrbitAnimationItem() override;

  int GetOrbit() const;
  void SetOrbit(int angle);

  bool GetReverse() const;
  void SetReverse(bool reverse);

  void Animate(double s) override;
};

#endif
