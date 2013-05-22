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

#ifndef QmitkImageMaskingWidget_h
#define QmitkImageMaskingWidget_h

#include <QWidget>
#include <ui_QmitkImageMaskingWidgetControls.h>

class QmitkImageMaskingWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkImageMaskingWidget(QWidget* parent = NULL);
  ~QmitkImageMaskingWidget();

private:
  Ui::QmitkImageMaskingWidgetControls m_Controls;
};

#endif
