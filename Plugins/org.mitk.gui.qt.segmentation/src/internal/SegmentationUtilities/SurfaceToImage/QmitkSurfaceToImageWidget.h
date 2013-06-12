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

#include <mitkSurface.h>

class QmitkSurfaceToImageWidget : public QmitkSegmentationUtilityWidget
{
  Q_OBJECT

public:
  explicit QmitkSurfaceToImageWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent = NULL);
  ~QmitkSurfaceToImageWidget();

private slots:

  void OnSelectionChanged(unsigned int index, const mitk::DataNode* selection);
  void OnSurface2ImagePressed();

private:

  /// \brief Enable and disable buttons
  void EnableButtons(bool enable = true);

  /// \brief Convert a surface to an binary image.
  mitk::Image::Pointer ConvertSurfaceToImage( mitk::Image::Pointer image, mitk::Surface::Pointer surface );

  Ui::QmitkSurfaceToImageWidgetControls m_Controls;
};

#endif
