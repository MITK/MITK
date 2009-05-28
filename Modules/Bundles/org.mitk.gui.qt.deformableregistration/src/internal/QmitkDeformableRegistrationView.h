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


#ifndef QMITKDEFORMABLEREGISTRATION_H
#define QMITKDEFORMABLEREGISTRATION_H

#include "QmitkFunctionality.h"
#include "ui_QmitkDeformableRegistrationViewControls.h"

#include "../DeformableregistrationDll.h"

/*!
\brief The DeformableRegistration functionality is used to perform deformable registration.

This functionality allows you to register two 2D as well as two 3D images in a non rigid manner.
Register means to align two images, so that they become as similar as possible. 
Therefore you can select from different deformable registration methods. 
Registration results will directly be applied to the Moving Image. The result is shown in the multi-widget. 

For more informations see: \ref QmitkDeformableRegistrationUserManual

\sa QmitkFunctionality
\ingroup Functionalities
\ingroup DeformableRegistration
*/

class DEFORMABLEREGISTRATION_EXPORT QmitkDeformableRegistrationView : public QObject, public QmitkFunctionality
{  
  Q_OBJECT
  
  public:  

    /*!  
    \brief default constructor  
    */  
    QmitkDeformableRegistrationView(QObject *parent=0, const char *name=0);

    /*!  
    \brief default destructor  
    */  
    virtual ~QmitkDeformableRegistrationView();

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
    * stores whether the image will be shown in grayvalues or in red for fixed image and green for moving image
    * @param show if true, then images will be shown in red and green
    */
    void ShowRedGreen(bool show);

    /*!
    * set the selected opacity for moving image
    * @param opacity the selected opacity
    */
    void OpacityUpdate(float opacity);

    /*!  
    \brief Sets the selected opacity for moving image

    @param opacity the selected opacity
    */
    void OpacityUpdate(int opacity);

    /*!
    * sets the images to grayvalues or fixed image to red and moving image to green
    * @param redGreen if true, then images will be shown in red and green
    */
    void SetImageColor(bool redGreen);

    /*!
    * sets all other images except the fixed image to invisible
    */
    void ShowFixedImage();

    /*!
    * sets all other images except the moving image to invisible
    */
    void ShowMovingImage();

    /*!
    * sets all other images except the fixed and moving images to invisible
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

    /*!  
    \brief Checks whether the registration can be performed.
    */
    void CheckCalculateEnabled();

    /*!  
    \brief Performs the registration.
    */
    void Calculate();

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
    Ui::QmitkDeformableRegistrationViewControls m_Controls;
    mitk::DataTreeNode* m_MovingNode;
    mitk::DataTreeNode* m_FixedNode;
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
    mitk::Color m_FixedColor;
    mitk::Color m_MovingColor;
    invisibleNodesList m_InvisibleNodesList;
    bool m_Deactivated;
};

#endif //QMITKDEFORMABLEREGISTRATION_H
