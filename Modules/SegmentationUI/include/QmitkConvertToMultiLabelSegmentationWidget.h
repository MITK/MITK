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
#include <memory>

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

/**
 * \brief Widget for converting images, surfaces, and contour models into multi-label segmentations.
 *
 * Provides a GUI for selecting input data nodes (images, surfaces, contour models) and either
 * generating a new multi-label segmentation or adding the converted inputs to an existing one.
 * The user can also specify a reference image for geometry alignment.
 *
 * \sa mitk::MultiLabelSegmentation
 */
class MITKSEGMENTATIONUI_EXPORT QmitkConvertToMultiLabelSegmentationWidget : public QWidget
{
  Q_OBJECT

public:

  /**
   * \brief Constructs the widget with GUI elements and signal/slot connections.
   * \param[in] dataStorage Pointer to the data storage for node selection.
   * \param[in] parent Optional parent widget.
   */
  explicit QmitkConvertToMultiLabelSegmentationWidget(mitk::DataStorage* dataStorage, QWidget* parent = nullptr);

  /** \brief Destructor. */
  ~QmitkConvertToMultiLabelSegmentationWidget() override;

private slots:

  /** \brief Called when the input node selection in the workbench changes. */
  void OnInputSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList /*nodes*/);

  void OnOutputSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList /*nodes*/);

  void OnRefSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList /*nodes*/);


  /** \brief Called when the user clicks the convert button. */
  void OnConvertPressed();

private:
  void ConvertNodes(const QmitkNodeSelectionDialog::NodeList& nodes);

  bool m_InternalEvent = false;
  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;

  /** \brief Enables or disables buttons based on the current data selection validity. */
  void ConfigureWidgets();

  std::unique_ptr<Ui::QmitkConvertToMultiLabelSegmentationWidgetControls> m_Controls;
};

#endif
