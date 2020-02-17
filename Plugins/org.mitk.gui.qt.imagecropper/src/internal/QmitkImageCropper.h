/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkImageCropper_h
#define QmitkImageCropper_h

#include <QmitkAbstractView.h>

#include <mitkBoundingShapeInteractor.h>

#include "ui_QmitkImageCropperViewControls.h"

class QmitkImageCropper : public QmitkAbstractView
{

  Q_OBJECT

public:

  QmitkImageCropper(QObject *parent = nullptr);

  ~QmitkImageCropper() override;

  static const std::string VIEW_ID;

  void CreateQtPartControl(QWidget *parent) override;

  void SetFocus() override { };

protected Q_SLOTS:
  /*!
  * @brief Creates a new bounding object
  */
  virtual void DoCreateNewBoundingObject();
  /*!
  * @brief Whenever Crop button is pressed, issue a cropping action
  */
  void DoCropping();
  /*!
  * @brief Whenever Mask button is pressed, issue a masking action
  */
  void DoMasking();
  /*!
* @brief Updates current selection of the image to crop
*/
  void OnImageSelectionChanged(QList<mitk::DataNode::Pointer> nodes);
  /*!
  * @brief Updates current selection of the bounding object
  */
  void OnBoundingBoxSelectionChanged(QList<mitk::DataNode::Pointer> nodes);
  /*!
  * @brief Sets the scalar value for outside pixels in case of masking
  */
  void OnSliderValueChanged(int slidervalue);

protected:

  /*!
  * @brief Initializes a new bounding shape using the selected image geometry.
  */
  mitk::Geometry3D::Pointer InitializeWithImageGeometry(mitk::BaseGeometry::Pointer geometry);

  void CreateBoundingShapeInteractor(bool rotationEnabled);

private:

  QWidget* m_ParentWidget;
  /*!
  * @brief Interactor for moving and scaling the cuboid
  */
  mitk::BoundingShapeInteractor::Pointer m_BoundingShapeInteractor;

  void ProcessImage(bool crop);

  /*!
  * @brief Resets GUI to default
  */
  void SetDefaultGUI();

  QString AdaptBoundingObjectName(const QString& name) const;

  // cropping parameter
  mitk::ScalarType m_CropOutsideValue;

  Ui::QmitkImageCropperViewControls m_Controls;
};

#endif // QmitkImageCropper_h
