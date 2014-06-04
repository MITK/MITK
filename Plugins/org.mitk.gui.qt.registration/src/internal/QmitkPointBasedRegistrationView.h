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

#if !defined(QMITK_POINTBASEDREGISTRATION_H__INCLUDED)
#define QMITK_POINTBASEDREGISTRATION_H__INCLUDED

#include "QmitkFunctionality.h"

#include "berryISelectionListener.h"
#include "berryIStructuredSelection.h"

//#include "mitkTestingConfig.h" // IMPORTANT: this defines or undefines BUILD_TESTING !

#include <mitkPointSet.h>
#include <vtkCellArray.h>
#include <vtkLandmarkTransform.h>
//#include "QmitkMessageBoxHelper.h"
#include "ui_QmitkPointBasedRegistrationViewControls.h"

#include <org_mitk_gui_qt_registration_Export.h>

/*!
\brief The PointBasedRegistration functionality is used to perform point based registration.

This functionality allows you to register 2D as well as 3D images in a rigid and deformable manner via corresponding
PointSets. Register means to align two images, so that they become as similar as possible.
Therefore you have to set corresponding points in both images, which will be matched. The movement, which has to be
performed on the points to align them will be performed on the moving image as well. The result is shown in the multi-widget.

For more informations see: \ref QmitkPointBasedRegistrationUserManual

\sa QmitkFunctionality
\ingroup Functionalities
\ingroup PointBasedRegistration
*/

class REGISTRATION_EXPORT QmitkPointBasedRegistrationView : public QmitkFunctionality
{

  friend struct SelListenerPointBasedRegistration;

  Q_OBJECT

public:

  static const std::string VIEW_ID;

  /*!
  \brief Default constructor
  */
  QmitkPointBasedRegistrationView(QObject *parent=0, const char *name=0);

  /*!
  \brief Default destructor
  */
  virtual ~QmitkPointBasedRegistrationView();

  /*!
  \brief method for creating the applications main widget
  */
  virtual void CreateQtPartControl(QWidget *parent);

  /*!
  \brief Sets the StdMultiWidget and connects it to the functionality.
  */
  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);

  /*!
  \brief Removes the StdMultiWidget and disconnects it from the functionality.
  */
  virtual void StdMultiWidgetNotAvailable();

  /*!
  \brief Method for creating the connections of main and control widget
  */
  virtual void CreateConnections();

  virtual void Activated();
  virtual void Deactivated();
  virtual void Visible();
  virtual void Hidden();

  //
  //     #ifdef BUILD_TESTING
  //         / **
  //           \brief Testing entry point
  //         * /
  //         virtual int TestYourself();
  //
  //         / **
  //            \brief Helper method for testing
  //         * /
  //         bool TestAllTools();
  //
  //
  //   protected slots:
  //     /**
  //        \brief Helper method for testing
  //     */
  //     void RegistrationErrorDialogFound( QWidget* widget );
  //
  //     /**
  //     \brief Helper method for testing
  //     */
  //     void ClearPointSetDialogFound( QWidget* widget );
  //
  //   private:
  //     bool m_MessageBox;
  //
  //
  //   public:
  // #else
  //     // slot function is needed, because moc ignores our #ifdefs
  //     void RegistrationErrorDialogFound( QWidget* widget ) {}
  //     // slot function is needed, because moc ignores our #ifdefs
  //     void ClearPointSetDialogFound(QWidget* widget){}
  // #endif

    void DataNodeHasBeenRemoved(const mitk::DataNode* node);

  protected slots:

    /*!
    \brief Sets the fixed Image according to TreeNodeSelector widget
    */
    void FixedSelected(mitk::DataNode::Pointer fixedImage);

    /*!
    \brief Sets the moving Image according to TreeNodeSelector widget
    */
    void MovingSelected(mitk::DataNode::Pointer movingImage);

    /*!
    \brief Calculates registration with vtkLandmarkTransform
    */
    void calculateLandmarkbased();

    /*!
    \brief Calculates registration with itkLandmarkWarping
    */
    void calculateLandmarkWarping();

    /*!
    \brief Calculates registration with ICP and vtkLandmarkTransform
    */
    void calculateLandmarkbasedWithICP();

    /*!
    \brief lets the fixed image become invisible and the moving image visible
    */
    void HideMovingImage(bool hide);

    /*!
    \brief lets the moving image become invisible and the fixed image visible
    */
    void HideFixedImage(bool hide);

    /*!
    \brief Checks if registration is possible
    */
    bool CheckCalculate();

    /*!
    \brief Performs an undo for the last transform.
    */
    void UndoTransformation();

    /*!
    \brief Performs a redo for the last undo transform.
    */
    void RedoTransformation();

    /*!
    \brief Stores whether the image will be shown in grayvalues or in red for fixed image and green for moving image

    @param show if true, then images will be shown in red and green
    */
    void showRedGreen(bool show);

    /*!
    \brief Sets the selected opacity for moving image

    @param opacity the selected opacity
    */
    void OpacityUpdate(float opacity);

    /*!
    \brief Sets the selected opacity for moving image

    @param opacity the selected opacity
    */
    void OpacityUpdate(int opacity);

    /*!
    \brief Updates the moving landmarks
    */
    void updateMovingLandmarksList();

    /*!
    \brief Updates the fixed landmarks
    */
    void updateFixedLandmarksList();

    /*!
    \brief Sets the images to gray values or fixed image to red and moving image to green

    @param redGreen if true, then images will be shown in red and green
    */
    void setImageColor(bool redGreen);

    /*!
    \brief Clears the undo and redo transformation lists.
    */
    void clearTransformationLists();

    /*!
    \brief Calculates the landmark error for the selected transformation.
    */
    void checkLandmarkError();

    /*!
    \brief Changes the transformation type and calls checkLandmarkError().
    */
    void transformationChanged(int transform);

    /*!
    \brief Checks whether the registration can be performed.
    */
    bool checkCalculateEnabled();

    /*!
    \brief Performs the registration.
    */
    void calculate();

    void SetImagesVisible(berry::ISelection::ConstPointer /*selection*/);

    void SwitchImages();

protected:

  berry::ISelectionListener::Pointer m_SelListener;
  berry::IStructuredSelection::ConstPointer m_CurrentSelection;

  /*!
  * default main widget containing 4 windows showing 3
  * orthogonal slices of the volume and a 3d render window
  */
  QmitkStdMultiWidget * m_MultiWidget;

  /*!
  * control widget to make all changes for point based registration
  */
  Ui::QmitkPointBasedRegistrationControls m_Controls;
  mitk::PointSet::Pointer m_FixedLandmarks;
  mitk::PointSet::Pointer m_MovingLandmarks;
  mitk::DataNode::Pointer m_MovingPointSetNode;
  mitk::DataNode::Pointer m_FixedPointSetNode;
  mitk::DataNode::Pointer m_MovingNode;
  mitk::DataNode::Pointer m_FixedNode;
  std::list<mitk::BaseGeometry::Pointer> m_UndoGeometryList;
  std::list<mitk::BaseGeometry::Pointer> m_UndoPointsGeometryList;
  std::list<mitk::BaseGeometry::Pointer> m_RedoGeometryList;
  std::list<mitk::BaseGeometry::Pointer> m_RedoPointsGeometryList;
  bool m_ShowRedGreen;
  float m_Opacity;
  float m_OriginalOpacity;
  mitk::Color m_FixedColor;
  mitk::Color m_MovingColor;
  int m_Transformation;
  bool m_HideFixedImage;
  bool m_HideMovingImage;
  std::string m_OldFixedLabel;
  std::string m_OldMovingLabel;
  bool m_Deactivated;
  int m_CurrentFixedLandmarksObserverID;
  int m_CurrentMovingLandmarksObserverID;
  itk::SimpleMemberCommand<QmitkPointBasedRegistrationView>::Pointer m_FixedLandmarksChangedCommand;
  itk::SimpleMemberCommand<QmitkPointBasedRegistrationView>::Pointer m_MovingLandmarksChangedCommand;
};
#endif // !defined(QMITK_POINTBASEDREGISTRATION_H__INCLUDED)
