/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkImageMaskingWidget_h
#define QmitkImageMaskingWidget_h

#include "../QmitkSegmentationUtilityWidget.h"
#include <ui_QmitkImageMaskingWidgetControls.h>

#include <mitkSurface.h>
#include <mitkImage.h>

/*!
  \brief QmitkImageMaskingWidget

  Tool masks an image with a binary image or a surface. The Method requires
  an image and a binary image mask or a surface. The input image and the binary
  image mask must be of the same size. Masking with a surface creates first a
  binary image of the surface and then use this for the masking of the input image.
*/
class QmitkImageMaskingWidget : public QmitkSegmentationUtilityWidget
{
  Q_OBJECT

public:

  /** @brief Default constructor, including creation of GUI elements and signals/slots connections. */
  explicit QmitkImageMaskingWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent = nullptr);

  /** @brief Defaul destructor. */
  ~QmitkImageMaskingWidget() override;

private slots:

  /** @brief This slot is called if the selection in the workbench is changed. */
  void OnSelectionChanged(unsigned int index, const mitk::DataNode* selection);

  /** @brief This slot is called if user activates the radio button for masking an image with a binary image mask. */
  void OnImageMaskingToggled(bool);

  /** @brief This slot is called if user activates the radio button for masking an image with a surface. */
  void OnSurfaceMaskingToggled(bool);

  /** @brief This slot is called if user activates the button to mask an image. */
  void OnMaskImagePressed();

private:

  /** @brief Check if selections is valid. */
  void SelectionControl( unsigned int index, const mitk::DataNode* selection);

  /** @brief Enable buttons if data selction is valid. */
  void EnableButtons(bool enable = true);

  /** @brief Mask an image with a given binary mask. Note that the input image and the mask image must be of the same size. */
  mitk::Image::Pointer MaskImage(mitk::Image::Pointer referenceImage, mitk::Image::Pointer maskImage );

  /** @brief Convert a surface into an binary image. */
  mitk::Image::Pointer ConvertSurfaceToImage( mitk::Image::Pointer image, mitk::Surface::Pointer surface );

  /** @brief Adds a new data object to the DataStorage.*/
  void AddToDataStorage(mitk::DataStorage::Pointer dataStorage, mitk::Image::Pointer segmentation,
                        const std::string& name, mitk::DataNode::Pointer parent = nullptr);

  Ui::QmitkImageMaskingWidgetControls m_Controls;
};

#endif
