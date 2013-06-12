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

class QmitkImageMaskingWidget : public QmitkSegmentationUtilityWidget
{
  Q_OBJECT

public:
  explicit QmitkImageMaskingWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent = NULL);
  ~QmitkImageMaskingWidget();

private slots:

  void OnSelectionChanged(unsigned int index, const mitk::DataNode* selection);
  void OnImageMaskingToggled(bool);
  void OnSurfaceMaskingToggled(bool);
  void OnMaskImagePressed();

private:

  void SelectionControll( unsigned int index, const mitk::DataNode* selection);

  /// \brief Enable buttons if data selction is valid
  void EnableButtons(bool enable = true);

  /// \brief Mask an image with a mask. Note, input and mask images must be of the same size.
  mitk::Image::Pointer MaskImage(mitk::Image::Pointer referenceImage, mitk::Image::Pointer maskImage );

  /// \brief Convert a surface to an binary image.
  mitk::Image::Pointer ConvertSurfaceToImage( mitk::Image::Pointer image, mitk::Surface::Pointer surface );

  void AddToDataStorage(mitk::DataStorage::Pointer dataStorage, mitk::Image::Pointer segmentation,
                        const std::string& name, mitk::DataNode::Pointer parent = NULL);

  Ui::QmitkImageMaskingWidgetControls m_Controls;
};

#endif
