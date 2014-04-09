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

#ifndef QmitkSurfaceToImageWidget_h
#define QmitkSurfaceToImageWidget_h

#include "../QmitkSegmentationUtilityWidget.h"
#include <ui_QmitkSurfaceToImageWidgetControls.h>

namespace mitk {
  class Surface;
  class Image;
}

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
  explicit QmitkSurfaceToImageWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent = NULL);

  /** @brief Defaul destructor. */
  ~QmitkSurfaceToImageWidget();

private slots:

  /** @brief This slot is called if the selection in the workbench is changed. */
  void OnSelectionChanged(unsigned int index, const mitk::DataNode* selection);

  /** @brief This slot is called if user activates the button to convert a surface into a binary image. */
  void OnSurface2ImagePressed();

private:

  /** @brief Enable buttons if data selction is valid. */
  void EnableButtons(bool enable = true);

  /** @brief Convert a surface into an binary image. */
  itk::SmartPointer<mitk::Image> ConvertSurfaceToImage( itk::SmartPointer<mitk::Image> image, itk::SmartPointer<mitk::Surface> surface );

  Ui::QmitkSurfaceToImageWidgetControls m_Controls;
};

#endif
