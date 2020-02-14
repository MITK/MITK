/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkTimeSliceAnimationWidget_h
#define QmitkTimeSliceAnimationWidget_h

#include "QmitkAnimationWidget.h"

class QmitkTimeSliceAnimationItem;

namespace Ui
{
  class QmitkTimeSliceAnimationWidget;
}

class QmitkTimeSliceAnimationWidget : public QmitkAnimationWidget
{
  Q_OBJECT

public:
  explicit QmitkTimeSliceAnimationWidget(QWidget* parent = nullptr);
  ~QmitkTimeSliceAnimationWidget() override;

  void SetAnimationItem(QmitkAnimationItem* sliceAnimationItem) override;

private slots:
  void OnFromChanged(double from);
  void OnToChanged(double to);
  void OnReverseChanged(bool reverse);

private:
  Ui::QmitkTimeSliceAnimationWidget* m_Ui;
  QmitkTimeSliceAnimationItem* m_AnimationItem;
};

#endif
