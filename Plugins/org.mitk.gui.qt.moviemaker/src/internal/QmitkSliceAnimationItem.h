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

#ifndef QmitkSliceAnimationItem_h
#define QmitkSliceAnimationItem_h

#include "QmitkAnimationItem.h"

class QmitkSliceAnimationItem : public QmitkAnimationItem
{
public:
  explicit QmitkSliceAnimationItem(int renderWindow = 0, int from = 0, int to = 0, bool reverse = false, double duration = 2.0, double delay = 0.0, bool startWithPrevious = false);
  virtual ~QmitkSliceAnimationItem();

  int GetRenderWindow() const;
  void SetRenderWindow(int renderWindow);

  int GetFrom() const;
  void SetFrom(int from);

  int GetTo() const;
  void SetTo(int to);

  bool GetReverse() const;
  void SetReverse(bool reverse);

  void Animate(double s);
};

#endif
