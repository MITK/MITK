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

#include <QWidget>

namespace Ui
{
  class QmitkSliceAnimationWidget;
}

class QmitkSliceAnimationWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkSliceAnimationWidget(QWidget* parent = NULL);
  ~QmitkSliceAnimationWidget();

private:
  Ui::QmitkSliceAnimationWidget* m_Ui;
};

#endif
