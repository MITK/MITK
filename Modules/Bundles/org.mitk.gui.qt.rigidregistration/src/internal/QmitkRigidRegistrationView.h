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


#ifndef QMITKRIGIDREGISTRATION_H
#define QMITKRIGIDREGISTRATION_H

#include "QmitkFunctionality.h"
#include "ui_QmitkRigidRegistrationViewControls.h"

#include "../RigidregistrationDll.h"

// Time Slider related
#include <QmitkStepperAdapter.h>

/*!
\brief This functionality allows you to register 2D as well as 3D images in a rigid manner.  

Register means to align two images, so that they become as similar as possible. 
Therefore you can select from different transforms, metrics and optimizers. 
Registration results will directly be applied to the Moving Image.

\sa QmitkFunctionality
\ingroup Functionalities
\ingroup RigidRegistration

\author Daniel Stein
*/

class RIGIDREGISTRATION_EXPORT QmitkRigidRegistrationView : public QObject, public QmitkFunctionality
{  
  Q_OBJECT
  
  public:  

    /*!  
    \brief default constructor  
    */  
    QmitkRigidRegistrationView(QObject *parent=0, const char *name=0);

    /*!  
    \brief default destructor  
    */  
    virtual ~QmitkRigidRegistrationView();

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
    \brief method for creating the connections of main and control widget  
    */  
    virtual void CreateConnections();

    /*!  
    \brief Method which is called when this functionality is selected in MITK  
    */ 
    virtual void Activated();

    /*!  
    \brief Method which is called whenever the functionality is deselected in MITK  
    */
    virtual void Deactivated();

  signals:

    /*!  
    \brief Signal that informs about that the fixed image should be reinitialized in the multi-widget.
    */
    void reinitFixed(const mitk::Geometry3D *);

    /*!  
    \brief Signal that informs about that the moving image should be reinitialized in the multi-widget.
    */
    void reinitMoving(const mitk::Geometry3D *);

    /*!  
    \brief Signal that informs about that the Demons registration should be performed.
    */
    void calculateDemonsRegistration();

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
    * sets the fixed Image according to TreeNodeSelector widget
    */
    void FixedSelected(int = 0);

    /*!
    * sets the moving Image according to TreeNodeSelector widget
    */
    void MovingSelected(int = 0);

    /*!
    * checks if registration is possible
    */
    bool CheckCalculate();

    /*!
    * \brief Saves the moving image, intended to be done after a registration.
    */
    void SaveModel();
    
    /*!
    * \brief Undo the last registration.
    */
    void UndoTransformation();
    
    /*!
    * \brief Redo the last registration
    */
    void RedoTransformation();

    /*!
    * \brief Adds a new Transformation to the undo list and enables the undo button.
    */
    void AddNewTransformationToUndoList();
    
    /*!
    * \brief Translates the moving image in x, y and z direction given by translateVector
    *
    * @param translateVector Contains the translation in x, y and z direction.
    */
    void Translate(int* translateVector);

    /*!
    * \brief Rotates the moving image in x, y and z direction given by rotateVector
    *
    * @param rotateVector Contains the rotation around x, y and z axis.
    */
    void Rotate(int* rotateVector);

    /*!
    * \brief Automatically aligns the image centers.
    */
    void AlignCenters();

    /*!
    * \brief Stores whether the image will be shown in gray values or in red for fixed image and green for moving image
    * @param show if true, then images will be shown in red and green
    */
    void ShowRedGreen(bool show);

    /*!
    * \brief Sets the selected opacity for moving image
    * @param opacity the selected opacity
    */
    void OpacityUpdate(float opacity);

    /*!  
    \brief Sets the selected opacity for moving image

    @param opacity the selected opacity
    */
    void OpacityUpdate(int opacity);

    /*!
    * \brief Sets the images to grayvalues or fixed image to red and moving image to green
    * @param redGreen if true, then images will be shown in red and green
    */
    void SetImageColor(bool redGreen);

    /*!
    * \brief Clears the undo and redo lists and sets the undo and redo buttons to disabled.
    */
    void ClearTransformationLists();

    /*!
    * \brief Only shows the fixed image in the render windows.
    * Only shows the fixed image in the render windows.  All other data tree nodes are invisible.
    */
    void ShowFixedImage();

    /*!
    * \brief Only shows the moving image in the render windows.
    * Only shows the moving image in the render windows. All other data tree nodes are invisible.
    */
    void ShowMovingImage();

    /*!
    * \brief Shows the fixed and the moving image in the render windows.
    * Shows the fixed and the moving image in the render windows. All other data tree nodes are invisible.
    */
    void ShowBothImages();

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

    void SetUndoEnabled( bool enable );

    void SetRedoEnabled( bool enable );

    void CheckCalculateEnabled();

    void xTrans_valueChanged( int v );

    void yTrans_valueChanged( int v );

    void zTrans_valueChanged( int v );

    void xRot_valueChanged( int v );

    void yRot_valueChanged( int v );

    void zRot_valueChanged( int v );

    void MovingImageChanged();

    /*!
    * \brief Starts the registration process.
    */
    void Calculate();

    void SetOptimizerValue( double value );

    void registrationTabChanged( int );

    void StopOptimizationClicked();

    void UpdateTimestep();

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
    * control widget to make all changes for Deformable registration 
    */  
    Ui::QmitkRigidRegistrationViewControls m_Controls;
    mitk::DataTreeNode* m_MovingNode;
    mitk::DataTreeNode* m_FixedNode;
    std::list<mitk::Geometry3D::Pointer> m_UndoGeometryList;
    std::list<mitk::Geometry3D::Pointer> m_RedoGeometryList;
    bool m_ShowRedGreen;
    bool m_ShowFixedImage;
    bool m_ShowMovingImage;
    bool m_ShowBothImages;
    float m_Opacity;
    float m_OriginalOpacity;
    int m_OldMovingLayer;
    int m_NewMovingLayer;
    bool m_OldMovingLayerSet;
    bool m_NewMovingLayerSet;
    bool m_Deactivated;
    int m_FixedDimension;
    int m_MovingDimension;
    int * translationParams;
    int * rotationParams;
    mitk::Color m_FixedColor;
    mitk::Color m_MovingColor;
    invisibleNodesList m_InvisibleNodesList;
    int m_TranslateSliderPos[3];
    int m_RotateSliderPos[3];

    QmitkStepperAdapter*      m_TimeStepperAdapter;
};

#endif //QMITKRigidREGISTRATION_H
