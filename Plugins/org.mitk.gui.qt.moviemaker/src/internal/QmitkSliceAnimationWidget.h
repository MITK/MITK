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
  explicit QmitkSliceAnimationWidget(QWidget* parent = NULL);
  ~QmitkSliceAnimationWidget();

  void SetAnimationItem(QmitkAnimationItem* sliceAnimationItem);

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
