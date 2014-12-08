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
  explicit QmitkTimeSliceAnimationWidget(QWidget* parent = NULL);
  ~QmitkTimeSliceAnimationWidget();

  void SetAnimationItem(QmitkAnimationItem* sliceAnimationItem);

private slots:
  void OnFromChanged(double from);
  void OnToChanged(double to);
  void OnReverseChanged(bool reverse);

private:
  Ui::QmitkTimeSliceAnimationWidget* m_Ui;
  QmitkTimeSliceAnimationItem* m_AnimationItem;
};

#endif
