/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkImageCropper_h
#define QmitkImageCropper_h

#include <QmitkAbstractView.h>

#ifdef WIN32
#pragma warning( disable : 4250 )
#endif

#include <QProgressDialog>
#include "QmitkRegisterClasses.h"
#include <QList>

#include "itkCommand.h"
#include <itkImage.h>
#include <itksys/SystemTools.hxx>

#include <vtkEventQtSlotConnect.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>

#include <mitkBoundingShapeInteractor.h>
#include <mitkDataStorage.h>
#include <mitkEventConfig.h>
#include <mitkGeometryData.h>
#include <mitkPointSet.h>
#include <mitkWeakPointer.h>

#include "ui_ImageCropperControls.h"

#include "usServiceRegistration.h"

/*!
@brief QmitkImageCropperView
\warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

\sa QmitkFunctionality
\ingroup ${plugin_target}_internal
*/
class QmitkImageCropper : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
private:

  Q_OBJECT

public:
  /*!
  @brief Constructor. Called by SampleApp (or other apps that use functionalities)
  */
  QmitkImageCropper(QObject *parent = 0);

  virtual ~QmitkImageCropper();

  static const std::string VIEW_ID;

  virtual void CreateQtPartControl(QWidget *parent);

  virtual void SetFocus() override;

  /*!
  @brief Creates the Qt connections needed
  */

  QWidget* GetControls();

  /// @brief Called when the user clicks the GUI button
  protected slots:
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

  /*!
  * The parent QWidget
  */
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
