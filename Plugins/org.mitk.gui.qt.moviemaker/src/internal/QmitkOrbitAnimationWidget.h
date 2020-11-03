/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkOrbitAnimationWidget_h
#define QmitkOrbitAnimationWidget_h

#include "QmitkAnimationWidget.h"

class QmitkOrbitAnimationItem;

namespace Ui
{
  class QmitkOrbitAnimationWidget;
}

class QmitkOrbitAnimationWidget : public QmitkAnimationWidget
{
  Q_OBJECT

public:
  explicit QmitkOrbitAnimationWidget(QWidget* parent = nullptr);
  ~QmitkOrbitAnimationWidget() override;

  void SetAnimationItem(QmitkAnimationItem* orbitAnimationItem) override;

private slots:
  void OnOrbitChanged(int orbit);
  void OnReverseChanged(bool reverse);

private:
  Ui::QmitkOrbitAnimationWidget* m_Ui;
  QmitkOrbitAnimationItem* m_AnimationItem;
};

#endif
