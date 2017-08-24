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

#ifndef QmitkBooleanOperationsWidget_h
#define QmitkBooleanOperationsWidget_h

#include "../QmitkSegmentationUtilityWidget.h"
#include <ui_QmitkBooleanOperationsWidgetControls.h>
#include <mitkBooleanOperation.h>

class QmitkBooleanOperationsWidget : public QmitkSegmentationUtilityWidget
{
  Q_OBJECT

public:
  explicit QmitkBooleanOperationsWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent = nullptr);
  ~QmitkBooleanOperationsWidget();

private slots:
  void OnSelectionChanged(unsigned int index, const mitk::DataNode* selection);
  void OnDifferenceButtonClicked();
  void OnIntersectionButtonClicked();
  void OnUnionButtonClicked();

private:
  void EnableButtons(bool enable = true);
  void DoBooleanOperation(mitk::BooleanOperation::Type type);

  Ui::QmitkBooleanOperationsWidgetControls m_Controls;
};

#endif
