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


#ifndef QMITKRigidREGISTRATION_H
#define QMITKRigidREGISTRATION_H

#include "QmitkFunctionality.h"

class QmitkStdMultiWidget;
class QmitkRigidRegistrationControls;

/*!
\brief This functionality allows you to register 2D as well as 3D images in a rigid manner.  

Register means to align two images, so that they become as similar as possible. 
Therefore you can select from different transforms, metrics and optimizers. 
Registration results will directly be applied to the Moving Image.

\sa QmitkFunctionality
\ingroup Functionalities
\ingroup Registration

\author Daniel Stein
*/
class QmitkRigidRegistration : public QmitkFunctionality
{  
  Q_OBJECT
  
  public:  

    typedef std::set<mitk::DataTreeNode*> invisibleNodesList;

    /*!  
    \brief Default constructor  
    */  
    QmitkRigidRegistration(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);

    /*!  
    \brief Default destructor  
    */  
    virtual ~QmitkRigidRegistration();

    /*!  
    \brief Method for creating the widget containing the application controls, like sliders, buttons etc.  
    */  
    virtual QWidget * CreateControlWidget(QWidget *parent);

    /*!  
    \brief Method for creating the applications main widget  
    */  
    virtual QWidget * CreateMainWidget(QWidget * parent);

    /*!  
    \brief Method for creating the connections of main and control widget  
    */  
    virtual void CreateConnections();

    /*!  
    \brief Method for creating an QAction object, i.e. button & menu entry  @param parent the parent QWidget  
    */  
    virtual QAction * CreateAction(QActionGroup *parent);

    /*!  
    \brief Method which is called when this functionality is selected in MITK  
    */ 
    virtual void Activated();

    /*!  
    \brief Method which is called whenever the functionality is deselected in MITK  
    */ 
    virtual void Deactivated();

  protected slots:  
    
    /*!
    * \brief Called whenever the data tree has changed. 
    */
    void TreeChanged();
    
    /*!
    * \brief sets the fixed Image according to TreeNodeSelector widget
    */
    void FixedSelected(mitk::DataTreeIteratorClone imageIt);
    
    /*!
    * \brief sets the moving Image according to TreeNodeSelector widget
    */
    void MovingSelected(mitk::DataTreeIteratorClone imageIt);

    /*!
    * \brief checks if registration is possible
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
    * \brief Starts the registration process.
    */
    void CalculateTransformation();
    
    /*!
    * \brief Translates the moving image in x, y and z direction given by translateVector
    *
    * @param translateVector Contains the translation in x, y and z direction.
    */
    void Translate(int* translateVector);

    /*!
    * \brief Automatically aligns the image centers.
    */
    void AlignCenters();

    /*!
    * \brief Stores whether the image will be shown in gray values or in red for fixed image and green for moving image
    * @param if true, then images will be shown in red and green
    */
    void ShowRedGreen(bool show);

    /*!
    * \brief Sets the selected opacity for moving image
    * @param opacity the selected opacity
    */
    void OpacityUpdate(float opacity);

    /*!
    * \brief Sets the images to grayvalues or fixed image to red and moving image to green
    * @param if true, then images will be shown in red and green
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

  protected:

    /*!  
    * \brief Default main widget containing 4 windows showing 3   
    * orthogonal slices of the volume and a 3d render window  
    */  
    QmitkStdMultiWidget * m_MultiWidget;

    /*!  
    * \brief Control widget to make all changes for Rigid registration 
    */  
    QmitkRigidRegistrationControls * m_Controls;
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
    mitk::Color m_FixedColor;
    mitk::Color m_MovingColor;
    invisibleNodesList m_InvisibleNodesList;
};

#endif //QMITKRigidREGISTRATION_H
