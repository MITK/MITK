/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkBooleanOperationsWidget_h
#define QmitkBooleanOperationsWidget_h

#include <MitkSegmentationUIExports.h>

#include <mitkBooleanOperation.h>
#include <QmitkSegmentationUtilityWidget.h>

namespace Ui
{
  class QmitkBooleanOperationsWidgetControls;
}

namespace mitk
{
  class DataNode;
  class DataStorage;
}

class MITKSEGMENTATIONUI_EXPORT QmitkBooleanOperationsWidget : public QmitkSegmentationUtilityWidget
{
  Q_OBJECT

public:
  explicit QmitkBooleanOperationsWidget(mitk::DataStorage* dataStorage,
                                        mitk::SliceNavigationController* timeNavigationController,
                                        QWidget* parent = nullptr);
  ~QmitkBooleanOperationsWidget() override;

private slots:
  void OnSelectionChanged(unsigned int index, const mitk::DataNode* selection);
  void OnDifferenceButtonClicked();
  void OnIntersectionButtonClicked();
  void OnUnionButtonClicked();

private:
  void EnableButtons(bool enable = true);
  void DoBooleanOperation(mitk::BooleanOperation::Type type);

  Ui::QmitkBooleanOperationsWidgetControls* m_Controls;
};

#endif
