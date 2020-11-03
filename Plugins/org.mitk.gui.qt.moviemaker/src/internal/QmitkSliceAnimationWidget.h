/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSliceAnimationWidget_h
#define QmitkSliceAnimationWidget_h

#include "QmitkAnimationWidget.h"

class QmitkSliceAnimationItem;

namespace Ui
{
  class QmitkSliceAnimationWidget;
}

class QmitkSliceAnimationWidget : public QmitkAnimationWidget
{
  Q_OBJECT

public:
  explicit QmitkSliceAnimationWidget(QWidget* parent = nullptr);
  ~QmitkSliceAnimationWidget() override;

  void SetAnimationItem(QmitkAnimationItem* sliceAnimationItem) override;

private slots:
  void OnRenderWindowChanged(int renderWindow);
  void OnFromChanged(double from);
  void OnToChanged(double to);
  void OnReverseChanged(bool reverse);

private:
  Ui::QmitkSliceAnimationWidget* m_Ui;
  QmitkSliceAnimationItem* m_AnimationItem;
};

#endif
