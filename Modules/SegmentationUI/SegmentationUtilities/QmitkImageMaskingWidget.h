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

namespace Ui
{
  class QmitkImageMaskingWidgetControls;
}

namespace mitk
{
  class Image;
  class DataStorage;
}

/*!
  \brief QmitkImageMaskingWidget

  Tool masks an image with a binary image or a surface. The Method requires
  an image and a binary image mask or a surface. The input image and the binary
  image mask must be of the same size. Masking with a surface creates first a
  binary image of the surface and then use this for the masking of the input image.
*/
class MITKSEGMENTATIONUI_EXPORT QmitkImageMaskingWidget : public QWidget
{
  Q_OBJECT

public:

  /** @brief Default constructor, including creation of GUI elements and signals/slots connections. */
  explicit QmitkImageMaskingWidget(mitk::DataStorage* dataStorage, QWidget* parent = nullptr);

  /** @brief Default destructor. */
  ~QmitkImageMaskingWidget() override;

private:

  /** @brief This slot is called if the image selection changed.*/
  void OnImageSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList /*nodes*/);

  /** @brief This slot is called if the segmentation selection changed.*/
  void OnSegSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList /*nodes*/);

  /** @brief This slot is called if user activates the button to mask an image. */
  void OnMaskImagePressed();

  /** @brief This slot is called if the user toggles the "Custom" radio button. */
  void OnCustomValueButtonToggled(bool checked);

  /** @brief Configure the widgets according to the internal state. */
  void ConfigureWidgets();
  void EnableButtons(bool enable);

  /** @brief Mask an image with a given binary mask. Note that the input image and the mask image must be of the same size. */
  itk::SmartPointer<mitk::Image> MaskImage(itk::SmartPointer<mitk::Image> referenceImage, itk::SmartPointer<mitk::Image> maskImage );

  /** @brief Adds a new data object to the DataStorage.*/
  void AddToDataStorage(mitk::DataStorage::Pointer dataStorage, itk::SmartPointer<mitk::Image> segmentation,
                        const std::string& name, mitk::DataNode::Pointer parent = nullptr);

  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
  Ui::QmitkImageMaskingWidgetControls* m_Controls;
};

#endif
