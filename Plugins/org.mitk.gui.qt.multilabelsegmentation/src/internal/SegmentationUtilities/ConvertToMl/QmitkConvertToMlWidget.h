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

#ifndef QmitkConvertToMlWidget_h
#define QmitkConvertToMlWidget_h

#include "../QmitkSegmentationUtilityWidget.h"
#include <ui_QmitkConvertToMlWidgetControls.h>

class QmitkConvertToMlWidget : public QmitkSegmentationUtilityWidget
{
  Q_OBJECT

public:
  explicit QmitkConvertToMlWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent = nullptr);
  ~QmitkConvertToMlWidget();

private slots:
  void SelectionChanged();
  void Convert();

private:
  void EnableButtons(bool enable = true);

  Ui::QmitkConvertToMlWidgetControls m_Controls;
};

#endif
