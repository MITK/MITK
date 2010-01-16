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

#include "berryISelectionListener.h"
#include "berryIStructuredSelection.h"

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

  friend struct SelListenerDeformableRegistration;

  Q_OBJECT

  public:  

    static const std::string VIEW_ID; 

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

    virtual void Visible();

    virtual void Hidden();

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
    \brief Signal that informs about that the BSpline registration should be performed.
    */
    void calculateBSplineRegistration();

  protected slots:  
    
    /*!
    * sets the fixed Image according to TreeNodeSelector widget
    */
    void FixedSelected(mitk::DataTreeNode::Pointer fixedImage);
    
    /*!
    * sets the moving Image according to TreeNodeSelector widget
    */
    void MovingSelected(mitk::DataTreeNode::Pointer movingImage);

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
    \brief Checks whether the registration can be performed.
    */
    void CheckCalculateEnabled();

    /*!  
    \brief Performs the registration.
    */
    void Calculate();

     /*!
     * Prints the values of the deformationfield 
     */
    void ApplyDeformationField();

    void SetImagesVisible(berry::ISelection::ConstPointer selection);

    void TabChanged(int index);

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
    * control widget to make all changes for Deformable registration 
    */  
    Ui::QmitkDeformableRegistrationViewControls m_Controls;
    mitk::DataTreeNode::Pointer m_MovingNode;
    mitk::DataTreeNode::Pointer m_FixedNode;
    bool m_ShowRedGreen;
    float m_Opacity;
    float m_OriginalOpacity;
    mitk::Color m_FixedColor;
    mitk::Color m_MovingColor;
    bool m_Deactivated;
};

#endif //QMITKDEFORMABLEREGISTRATION_H
