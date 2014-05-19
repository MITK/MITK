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

#ifndef QmitkContourModelToImageWidget_h
#define QmitkContourModelToImageWidget_h

#include "../QmitkSegmentationUtilityWidget.h"
#include <ui_QmitkContourModelToImageWidgetControls.h>

namespace mitk {
  class Image;
  class ContourModelSet;
  class ContourModel;
  class Geometry3D;
  class PlaneGeometry;
}

/*!
  \brief QmitkContourModelToImageWidget

  Tool masks an image with a binary image or a surface. The Method requires
  an image and a binary image mask or a surface. The input image and the binary
  image mask must be of the same size. Masking with a surface creates first a
  binary image of the surface and then use this for the masking of the input image.
*/
class QmitkContourModelToImageWidget : public QmitkSegmentationUtilityWidget
{
  Q_OBJECT

public:

  /** @brief Default constructor, including creation of GUI elements and signals/slots connections. */
  explicit QmitkContourModelToImageWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent = NULL);

  /** @brief Defaul destructor. */
  ~QmitkContourModelToImageWidget();

private slots:

  /** @brief This slot is called if the selection in the workbench is changed. */
  void OnSelectionChanged(unsigned int index, const mitk::DataNode* selection);

//  /** @brief This slot is called if user activates the radio button for masking an image with a binary image mask. */
//  void OnContourModelToImageToggled(bool);

//  /** @brief This slot is called if user activates the radio button for masking an image with a surface. */
//  void OnSurfaceMaskingToggled(bool);

  /** @brief This slot is called if user activates the button to mask an image. */
  void OnProcessPressed();

private:

  /** @brief Check if selections is valid. */
  void SelectionControl( unsigned int index, const mitk::DataNode* selection);

  /** @brief Enable buttons if data selction is valid. */
  void EnableButtons(bool enable = true);

  /** @brief Extracts the slice which correspond to the position of the contour*/
  itk::SmartPointer<mitk::Image> GetSliceForContour(itk::SmartPointer<mitk::ContourModel> contour, unsigned int timestep);

  /** @brief Extracts the slice which correspond to the position of the contour*/
  void WriteBackSlice (itk::SmartPointer<mitk::Image> slice, unsigned int timestep);

  /** @brief Fills a mitk::ContourModel into a given segmentation image */
  itk::SmartPointer<mitk::Image> ContourModelToImage(itk::SmartPointer<mitk::Image> segmenationImage, itk::SmartPointer<mitk::ContourModel> contour );

  /** @brief Fills a whole ContourModelSet into a given segmentation image */
  itk::SmartPointer<mitk::Image> ContourModelSetToImage( itk::SmartPointer<mitk::Image> segmenationImage, itk::SmartPointer<mitk::ContourModelSet> contourSet );

  Ui::QmitkContourModelToImageWidgetControls m_Controls;

  itk::SmartPointer<mitk::Image> m_SegmentationImage;
  mitk::Geometry3D::Pointer m_SegmentationImageGeometry;
  itk::SmartPointer<mitk::PlaneGeometry> m_CurrentPlane;
};

#endif
