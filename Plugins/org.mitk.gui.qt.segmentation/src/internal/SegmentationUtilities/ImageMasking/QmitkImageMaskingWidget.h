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

#ifndef QmitkImageMaskingWidget_h
#define QmitkImageMaskingWidget_h

#include "../QmitkSegmentationUtilityWidget.h"
#include <ui_QmitkImageMaskingWidgetControls.h>

#include <mitkSurface.h>

namespace mitk {
  class Image;
}

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
  explicit QmitkImageMaskingWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent = NULL);

  /** @brief Defaul destructor. */
  ~QmitkImageMaskingWidget();

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
  itk::SmartPointer<mitk::Image> MaskImage(itk::SmartPointer<mitk::Image> referenceImage, itk::SmartPointer<mitk::Image> maskImage );

  /** @brief Convert a surface into an binary image. */
  itk::SmartPointer<mitk::Image> ConvertSurfaceToImage( itk::SmartPointer<mitk::Image> image, mitk::Surface::Pointer surface );

  /** @brief Adds a new data object to the DataStorage.*/
  void AddToDataStorage(mitk::DataStorage::Pointer dataStorage, itk::SmartPointer<mitk::Image> segmentation,
                        const std::string& name, mitk::DataNode::Pointer parent = NULL);

  Ui::QmitkImageMaskingWidgetControls m_Controls;
};

#endif
