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

#ifndef QmitkAnimationWidget_h
#define QmitkAnimationWidget_h

#include <QWidget>

class QmitkAnimationItem;

class QmitkAnimationWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkAnimationWidget(QWidget* parent = nullptr);
  virtual ~QmitkAnimationWidget();

  virtual void SetAnimationItem(QmitkAnimationItem* animationItem) = 0;
};

#endif
