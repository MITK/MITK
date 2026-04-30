/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkImageMaskingWidget_h
#define QmitkImageMaskingWidget_h

#include <MitkSegmentationUIExports.h>

#include <mitkDataStorage.h>
#include <QmitkAbstractNodeSelectionWidget.h>

#include <QWidget>
#include <memory>

namespace Ui
{
  class QmitkImageMaskingWidgetControls;
}

namespace mitk
{
  class Image;
  class DataStorage;
}

/**
 * \brief Widget for masking an image with a binary image or surface mask.
 *
 * Applies a mask to an image using either a binary image mask or a surface. The input image
 * and binary image mask must be of the same size. When masking with a surface, a binary image
 * is first generated from the surface and then used for masking. The user can select the masking
 * value (minimum of image, custom value, etc.) for pixels outside the mask region.
 *
 * \sa QmitkBooleanOperationsWidget
 */
class MITKSEGMENTATIONUI_EXPORT QmitkImageMaskingWidget : public QWidget
{
  Q_OBJECT

public:

  /**
   * \brief Constructs the widget with GUI elements and signal/slot connections.
   * \param[in] dataStorage Pointer to the data storage for node selection.
   * \param[in] parent Optional parent widget.
   */
  explicit QmitkImageMaskingWidget(mitk::DataStorage* dataStorage, QWidget* parent = nullptr);

  /** \brief Destructor. */
  ~QmitkImageMaskingWidget() override;

private:

  /** \brief Called when the image selection changes. */
  void OnImageSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList /*nodes*/);

  /** \brief Called when the segmentation selection changes. */
  void OnSegSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList /*nodes*/);

  /** \brief Called when the user clicks the mask image button. */
  void OnMaskImagePressed();

  /** \brief Called when the user toggles the "Custom" radio button. */
  void OnCustomValueButtonToggled(bool checked);

  /** \brief Configures the widgets according to the internal state. */
  void ConfigureWidgets();
  void EnableButtons(bool enable);

  /** \brief Masks an image with a given binary mask. The input image and the mask image must be of the same size. */
  itk::SmartPointer<mitk::Image> MaskImage(itk::SmartPointer<mitk::Image> referenceImage, itk::SmartPointer<mitk::Image> maskImage );

  /** \brief Adds a new data object to the DataStorage. */
  void AddToDataStorage(mitk::DataStorage::Pointer dataStorage, itk::SmartPointer<mitk::Image> segmentation,
                        const std::string& name, mitk::DataNode::Pointer parent = nullptr);

  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
  std::unique_ptr<Ui::QmitkImageMaskingWidgetControls> m_Controls;
};

#endif
