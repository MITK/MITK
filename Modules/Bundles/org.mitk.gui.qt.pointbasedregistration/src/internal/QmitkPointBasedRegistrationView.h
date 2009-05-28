/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#if !defined(QMITK_POINTBASEDREGISTRATION_H__INCLUDED)
#define QMITK_POINTBASEDREGISTRATION_H__INCLUDED

#include "QmitkFunctionality.h"

//#include "mitkTestingConfig.h" // IMPORTANT: this defines or undefines BUILD_TESTING !

#include <mitkPointSetInteractor.h>
#include <mitkGlobalInteraction.h>
#include <mitkAffineInteractor.h>
#include <mitkPointSet.h>
#include <vtkCellArray.h>
#include <vtkLandmarkTransform.h>
//#include "QmitkMessageBoxHelper.h"
#include "ui_QmitkPointBasedRegistrationViewControls.h"

#include "../PointbasedregistrationDll.h"

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

class POINTBASEDREGISTRATION_EXPORT QmitkPointBasedRegistrationView : public QObject, public QmitkFunctionality
{  
  Q_OBJECT

public:  

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

signals:

  /*!  
  \brief Signal that informs about that the fixed image should be reinitialized in the multi-widget.
  */
  void reinitFixed(const mitk::Geometry3D *);

  /*!  
  \brief Signal that informs about that the moving image should be reinitialized in the multi-widget.
  */
  void reinitMoving(const mitk::Geometry3D *);

  protected slots:  

    /*!
    \brief Called whenever the data storage has changed. 
    */
    virtual void DataStorageChanged();

    /*!  
    \brief Returns the predication for the nodes shown in the DataStorageComboBoxes.
    */
    mitk::NodePredicateBase::Pointer GetMovingImagePredicate();

    /*!  
    \brief Sets the fixed Image according to TreeNodeSelector widget
    */
    void FixedSelected(int = 0);

    /*!  
    \brief Sets the moving Image according to TreeNodeSelector widget
    */
    void MovingSelected(int = 0);

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
    \brief Saves the registration result.
    */
    void SaveModel();

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
    \brief Sets whether all other data tree nodes except the two selected should be invisible or not.

    True = all nodes invisible, false = all nodes visible.
    */
    void setInvisible(bool invisible);

    /*!  
    \brief Reinitializes the fixed image in the multi-widget.
    */
    void reinitFixedClicked();

    /*!  
    \brief Reinitializes the moving image in the multi-widget.
    */
    void reinitMovingClicked();

    /*!  
    \brief Reinitializes all visible images in the multi-widget.
    */
    void globalReinitClicked();

    /*!  
    \brief Checks whether the registration can be performed.
    */
    bool checkCalculateEnabled();

    /*!  
    \brief Performs the registration.
    */
    void calculate();

protected:

  /*!  
  \brief List that holds all invisible data tree nodes. 
  */
  typedef std::set<mitk::DataTreeNode*> invisibleNodesList;

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
  mitk::DataTreeNode::Pointer m_MovingPointSetNode;
  mitk::DataTreeNode::Pointer m_FixedPointSetNode;
  mitk::DataTreeNode* m_MovingNode;
  mitk::DataTreeNode* m_FixedNode;
  std::list<mitk::Geometry3D::Pointer> m_UndoGeometryList;
  std::list<mitk::Geometry3D::Pointer> m_UndoPointsGeometryList;
  std::list<mitk::Geometry3D::Pointer> m_RedoGeometryList;
  std::list<mitk::Geometry3D::Pointer> m_RedoPointsGeometryList;
  bool m_SetInvisible;
  bool m_ShowRedGreen;
  float m_Opacity;
  float m_OriginalOpacity;
  int m_OldMovingLayer;
  int m_NewMovingLayer;
  bool m_OldMovingLayerSet;
  bool m_NewMovingLayerSet;
  mitk::Color m_FixedColor;
  mitk::Color m_MovingColor;
  invisibleNodesList m_InvisibleNodesList;
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
