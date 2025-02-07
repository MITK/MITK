/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkBooleanOperationsWidget_h
#define QmitkBooleanOperationsWidget_h

#include <mitkLabelSetImage.h>

#include <QmitkAbstractNodeSelectionWidget.h>

#include <QWidget>

#include <MitkSegmentationUIExports.h>

namespace Ui
{
  class QmitkBooleanOperationsWidgetControls;
}

namespace mitk
{
  class DataNode;
  class DataStorage;
}

class MITKSEGMENTATIONUI_EXPORT QmitkBooleanOperationsWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkBooleanOperationsWidget(mitk::DataStorage* dataStorage, QWidget* parent = nullptr);
  ~QmitkBooleanOperationsWidget() override;

private slots:
  void OnSegSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList nodes);
  void OnLabelSelectionChanged(mitk::MultiLabelSegmentation::LabelValueVectorType labels);
  void OnDifferenceButtonClicked();
  void OnIntersectionButtonClicked();
  void OnUnionButtonClicked();

private:
  void ConfigureWidgets();
  void SaveResultLabelMask(const mitk::Image* resultMask, const std::string& labelName) const;


  Ui::QmitkBooleanOperationsWidgetControls* m_Controls;
};

#endif
