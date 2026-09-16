/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkExtractFromMultiLabelSegmentationWidget_h
#define QmitkExtractFromMultiLabelSegmentationWidget_h

#include <MitkSegmentationUIExports.h>
#include <QmitkNodeSelectionDialog.h>
#include <QmitkAbstractNodeSelectionWidget.h>

#include <QWidget>
#include <memory>
#include <vector>

namespace Ui
{
  class QmitkExtractFromMultiLabelSegmentationWidgetControls;
}

namespace mitk
{
  class DataNode;
  class DataStorage;
  class Surface;
  class Image;
  class MultiLabelSegmentation;
}

/**
 * \brief Widget for extracting images from multi-label segmentations.
 *
 * Provides a GUI and logic to extract different image representations (class maps, instance maps,
 * and instance masks) from a multi-label segmentation. The extracted images are stored as child
 * nodes in the data storage.
 *
 * \sa mitk::MultiLabelSegmentation
 * \sa QmitkConvertToMultiLabelSegmentationWidget
 */
class MITKSEGMENTATIONUI_EXPORT QmitkExtractFromMultiLabelSegmentationWidget : public QWidget
{
  Q_OBJECT

public:

  /**
   * \brief Constructs the widget with GUI elements and signal/slot connections.
   * \param[in] dataStorage Pointer to the data storage for node selection and result storage.
   * \param[in] parent Optional parent widget.
   */
  explicit QmitkExtractFromMultiLabelSegmentationWidget(mitk::DataStorage* dataStorage, QWidget* parent = nullptr);

  /** \brief Destructor. */
  ~QmitkExtractFromMultiLabelSegmentationWidget() override;

signals:
  /** \brief Emitted after extraction result node(s) were added to the data storage. */
  void NewResultsReady(const QList<mitk::DataNode::Pointer>& nodes);

private slots:

  /** \brief Called when the segmentation selection in the workbench changes. */
  void OnSegSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList /*nodes*/);

  /** \brief Called when the user clicks the extract button. */
  void OnExtractPressed();

  /** \brief Removes the result node(s) created by the most recent extraction run. */
  void OnRemoveResultPressed();

private:
  bool m_InternalEvent = false;
  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
  std::vector<mitk::WeakPointer<mitk::DataNode>> m_LastResultNodes;

  void ConfigureWidgets();

  mitk::DataNode::Pointer StoreToDataStorage(mitk::Image* image, const std::string& name, mitk::DataNode* parent);

  /** \brief Enables the remove-result button only while a last result still exists in the data storage. */
  void UpdateRemoveResultButton();

  std::unique_ptr<Ui::QmitkExtractFromMultiLabelSegmentationWidgetControls> m_Controls;
};

#endif
