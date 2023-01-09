/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkContourModelToImageWidget_h
#define QmitkContourModelToImageWidget_h

#include <MitkSegmentationUIExports.h>

#include <QmitkSegmentationUtilityWidget.h>

#include <QScopedPointer>

class QmitkContourModelToImageWidgetPrivate;

namespace mitk
{
  class DataNode;
  class DataStorage;
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
class MITKSEGMENTATIONUI_EXPORT QmitkContourModelToImageWidget : public QmitkSegmentationUtilityWidget
{
  Q_OBJECT

public:

  /** @brief Default constructor, including creation of GUI elements and signals/slots connections. */
  explicit QmitkContourModelToImageWidget(mitk::DataStorage* dataStorage,
                                          mitk::SliceNavigationController* timeNavigationController,
                                          QWidget* parent = nullptr);

  /** @brief Defaul destructor. */
  ~QmitkContourModelToImageWidget() override;

private slots:

  /** @brief This slot is called if the selection in the workbench is changed. */
  void OnSelectionChanged(unsigned int index, const mitk::DataNode* selection);

  /** @brief This slot is called if user activates the button to mask an image. */
  void OnProcessPressed();

  /** @brief This slot is called after processing is finished */
  void OnProcessingFinished();

private:

  QScopedPointer<QmitkContourModelToImageWidgetPrivate> d_ptr;

  Q_DECLARE_PRIVATE(QmitkContourModelToImageWidget)
  Q_DISABLE_COPY(QmitkContourModelToImageWidget)
};

#endif
