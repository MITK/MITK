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

#ifndef QmitkOrbitAnimationItem_h
#define QmitkOrbitAnimationItem_h

#include "QmitkAnimationItem.h"

class QmitkOrbitAnimationItem : public QmitkAnimationItem
{
public:
  QmitkOrbitAnimationItem(double angle, bool reverse, double duration = 2.0, double delay = 0.0, bool startWithPrevious = false);
  virtual ~QmitkOrbitAnimationItem();

  double GetAngle() const;
  void SetAngle(double angle);

  bool GetReverse() const;
  void SetReverse(bool reverse);
};

#endif
