/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkConvertToMultiLabelSegmentationWidget_h
#define QmitkConvertToMultiLabelSegmentationWidget_h

#include <MitkSegmentationUIExports.h>
#include <QmitkAbstractNodeSelectionWidget.h>
#include <QmitkNodeSelectionDialog.h>

#include <QWidget>

namespace Ui
{
  class QmitkConvertToMultiLabelSegmentationWidgetControls;
}

namespace mitk
{
  class DataNode;
  class DataStorage;
  class Surface;
  class Image;
  class MultiLabelSegmentation;
}

/*!
  \brief QmitkConvertToMultiLabelSegmentationWidget

  Widget that offers the GUI and logic to convert different inputs (images, surfaces and contour models)
  into a multi label segmentation (by generating a new or adding the converted inputs to an existing
  segmentation).
*/
class MITKSEGMENTATIONUI_EXPORT QmitkConvertToMultiLabelSegmentationWidget : public QWidget
{
  Q_OBJECT

public:

  /** @brief Default constructor, including creation of GUI elements and signals/slots connections. */
  explicit QmitkConvertToMultiLabelSegmentationWidget(mitk::DataStorage* dataStorage, QWidget* parent = nullptr);

  /** @brief Default destructor. */
  ~QmitkConvertToMultiLabelSegmentationWidget() override;

private slots:

  /** @brief This slot is called if the selection in the workbench is changed. */
  void OnInputSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList /*nodes*/);

  void OnOutputSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList /*nodes*/);

  void OnRefSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList /*nodes*/);


  /** @brief This slot is called if user activates the button to convert a surface into a binary image. */
  void OnConvertPressed();

private:
  void ConvertNodes(const QmitkNodeSelectionDialog::NodeList& nodes);

  bool m_InternalEvent = false;
  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;

  /** @brief Enable buttons if data selection is valid. */
  void ConfigureWidgets();

  Ui::QmitkConvertToMultiLabelSegmentationWidgetControls* m_Controls;
};

#endif
