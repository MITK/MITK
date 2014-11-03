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

#ifndef QmitkOrbitAnimationWidget_h
#define QmitkOrbitAnimationWidget_h

#include <QWidget>

namespace Ui
{
  class QmitkOrbitAnimationWidget;
}

class QmitkOrbitAnimationWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkOrbitAnimationWidget(QWidget* parent = NULL);
  ~QmitkOrbitAnimationWidget();

private:
  Ui::QmitkOrbitAnimationWidget* m_Ui;
};

#endif
