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

#ifdef WIN32
#pragma warning( disable : 4250 )
#endif

#include <mitkBoundingShapeInteractor.h>
#include <mitkWeakPointer.h>

#include "ui_ImageCropperControls.h"

/*!
@brief QmitkImageCropperView
\warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

\sa QmitkFunctionality
\ingroup ${plugin_target}_internal
*/
class QmitkImageCropper : public QmitkAbstractView
{

  Q_OBJECT

public:
  QmitkImageCropper(QObject *parent = nullptr);

  ~QmitkImageCropper() override;

  static const std::string VIEW_ID;

  void CreateQtPartControl(QWidget *parent) override;

  void SetFocus() override;

  /*!
  @brief Creates the Qt connections needed
  */

  QWidget* GetControls();
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
  * @brief Dis- or enable the advanced setting section
  */
  void OnAdvancedSettingsButtonToggled();
  /*!
  * @brief Updates current selection of the bounding object
  */
  void OnDataSelectionChanged(const mitk::DataNode* node);
  /*!
  * @brief Sets the scalar value for outside pixels in case of masking
  */
  void OnSliderValueChanged(int slidervalue);

protected:

  /*!
 @brief called by QmitkFunctionality when DataManager's selection has changed
  */
  void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;
  /*!
  @brief Sets the selected bounding object as current bounding object and set up interactor
  */
  void OnComboBoxSelectionChanged(const mitk::DataNode* node);
  /*!
  * @brief Initializes a new bounding shape using the selected image geometry.
  */
  mitk::Geometry3D::Pointer InitializeWithImageGeometry(mitk::BaseGeometry::Pointer geometry);

  void CreateBoundingShapeInteractor(bool rotationEnabled);

private:

  QWidget* m_ParentWidget;
  /*!
  * @brief A pointer to the node of the image to be cropped.
  */
  mitk::WeakPointer<mitk::DataNode> m_ImageNode;
  /*!
  * @brief The cuboid used for cropping.
  */
  mitk::GeometryData::Pointer m_CroppingObject;

  /*!
  * @brief Tree node of the cuboid used for cropping.
  */
  mitk::DataNode::Pointer m_CroppingObjectNode;

  /*!
  * @brief Interactor for moving and scaling the cuboid
  */
  mitk::BoundingShapeInteractor::Pointer m_BoundingShapeInteractor;

  void ProcessImage(bool crop);

  /*!
  * @brief Resets GUI to default
  */
  void setDefaultGUI();

  QString AdaptBoundingObjectName(const QString& name) const;

  // cropping parameter
  mitk::ScalarType m_CropOutsideValue;
  bool m_Advanced;
  bool m_Active;
  bool m_ScrollEnabled;

  Ui::ImageCropperControls m_Controls;
};

#endif // QmitkImageCropper_h
