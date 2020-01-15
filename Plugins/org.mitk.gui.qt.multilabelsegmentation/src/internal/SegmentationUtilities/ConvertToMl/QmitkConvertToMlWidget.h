/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkConvertToMlWidget_h
#define QmitkConvertToMlWidget_h

#include "../QmitkSegmentationUtilityWidget.h"
#include <ui_QmitkConvertToMlWidgetControls.h>

class QmitkConvertToMlWidget : public QmitkSegmentationUtilityWidget
{
  Q_OBJECT

public:
  explicit QmitkConvertToMlWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent = nullptr);
  ~QmitkConvertToMlWidget() override;

private slots:
  void SelectionChanged();
  void Convert();

private:
  void EnableButtons(bool enable = true);

  Ui::QmitkConvertToMlWidgetControls m_Controls;
};

#endif
