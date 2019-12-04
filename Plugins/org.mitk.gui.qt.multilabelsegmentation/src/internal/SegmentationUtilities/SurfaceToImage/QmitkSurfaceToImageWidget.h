/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSurfaceToImageWidget_h
#define QmitkSurfaceToImageWidget_h

#include "../QmitkSegmentationUtilityWidget.h"
#include <ui_QmitkSurfaceToImageWidgetControls.h>

#include <mitkSurface.h>
#include <mitkImage.h>

/*!
  \brief QmitkSurfaceToImageWidget

  The Tool converts a surface to a binary image. The Method requires
  a surface and an image, which header information defines the output
  image. The resulting binary image has the same dimension, size, and
  Geometry3D as the input image.
*/
class QmitkSurfaceToImageWidget : public QmitkSegmentationUtilityWidget
{
  Q_OBJECT

public:

  /** @brief Default constructor, including creation of GUI elements and signals/slots connections. */
  explicit QmitkSurfaceToImageWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent = nullptr);

  /** @brief Defaul destructor. */
  ~QmitkSurfaceToImageWidget() override;

private slots:

  /** @brief This slot is called if the selection in the workbench is changed. */
  void OnSelectionChanged(unsigned int index, const mitk::DataNode* selection);

  /** @brief This slot is called if user activates the button to convert a surface into a binary image. */
  void OnSurface2ImagePressed();

  void OnMakeOutputBinaryChanged(bool value);

private:

  /** @brief Enable buttons if data selction is valid. */
  void EnableButtons(bool enable = true);

  /** @brief Convert a surface into an binary image. */
  mitk::Image::Pointer ConvertSurfaceToImage( mitk::Image::Pointer image, mitk::Surface::Pointer surface );

  Ui::QmitkSurfaceToImageWidgetControls m_Controls;
};

#endif
