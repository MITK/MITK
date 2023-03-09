/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkImageCropperView_h
#define QmitkImageCropperView_h

#include <QmitkAbstractView.h>

#include <mitkBoundingShapeInteractor.h>

#include "ui_QmitkImageCropperViewControls.h"

class QmitkImageCropperView : public QmitkAbstractView
{

  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkImageCropperView(QObject *parent = nullptr);

  ~QmitkImageCropperView() override;

  void CreateQtPartControl(QWidget *parent) override;

  void SetFocus() override { };

protected Q_SLOTS:

  /*!
  * @brief Updates current selection of the image to crop
  */
  void OnImageSelectionChanged(QList<mitk::DataNode::Pointer> nodes);
  /*!
  * @brief Updates current selection of the bounding object
  */
  void OnBoundingBoxSelectionChanged(QList<mitk::DataNode::Pointer> nodes);
  /*!
  * @brief Creates a new bounding object
  */
  void OnCreateNewBoundingBox();
  /*!
  * @brief Whenever Crop button is pressed, issue a cropping action
  */
  void OnCropping();
  /*!
  * @brief Whenever Mask button is pressed, issue a masking action
  */
  void OnMasking();
  /*!
  * @brief Sets the scalar value for outside pixels in case of masking
  */
  void OnSliderValueChanged(int slidervalue);

private:

  void CreateBoundingShapeInteractor(bool rotationEnabled);

  // initializes a new bounding shape using the selected image geometry.
  mitk::Geometry3D::Pointer InitializeWithImageGeometry(const mitk::BaseGeometry* geometry) const;

  void ProcessImage(bool crop);

  void SetDefaultGUI();

  QString AdaptBoundingObjectName(const QString& name) const;

  QWidget* m_ParentWidget;

  // interactor for moving and scaling the cuboid
  mitk::BoundingShapeInteractor::Pointer m_BoundingShapeInteractor;

  // cropping parameter
  mitk::ScalarType m_CropOutsideValue;

  Ui::QmitkImageCropperViewControls m_Controls;
};

#endif
