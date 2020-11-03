/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAnimationWidget_h
#define QmitkAnimationWidget_h

#include <QWidget>

class QmitkAnimationItem;

class QmitkAnimationWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkAnimationWidget(QWidget* parent = nullptr);
  ~QmitkAnimationWidget() override;

  virtual void SetAnimationItem(QmitkAnimationItem* animationItem) = 0;
};

#endif
