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
\brief RigidRegistration 

One needs to reimplement the methods CreateControlWidget(..), CreateMainWidget(..) 
and CreateAction(..) from QmitkFunctionality. 

\sa QmitkFunctionality
\ingroup Functionalities
*/
class QmitkRigidRegistration : public QmitkFunctionality
{  
  Q_OBJECT
  
  public:  

    typedef std::set<mitk::DataTreeNode*> invisibleNodesList;

    /*!  
    \brief default constructor  
    */  
    QmitkRigidRegistration(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);

    /*!  
    \brief default destructor  
    */  
    virtual ~QmitkRigidRegistration();

    /*!  
    \brief method for creating the widget containing the application   controls, like sliders, buttons etc.  
    */  
    virtual QWidget * CreateControlWidget(QWidget *parent);

    /*!  
    \brief method for creating the applications main widget  
    */  
    virtual QWidget * CreateMainWidget(QWidget * parent);

    /*!  
    \brief method for creating the connections of main and control widget  
    */  
    virtual void CreateConnections();

    /*!  
    \brief method for creating an QAction object, i.e. button & menu entry  @param parent the parent QWidget  
    */  
    virtual QAction * CreateAction(QActionGroup *parent);

    virtual void Activated();
    virtual void Deactivated();

  protected slots:  
    
    void TreeChanged();
    
    /*
    * sets the fixed Image according to TreeNodeSelector widget
    */
    void FixedSelected(mitk::DataTreeIteratorClone imageIt);
    
    /*
    * sets the moving Image according to TreeNodeSelector widget
    */
    void MovingSelected(mitk::DataTreeIteratorClone imageIt);

    /*
    * checks if registration is possible
    */
    bool CheckCalculate();

    void SaveModel();
    void UndoTransformation();
    void RedoTransformation();
    void AddNewTransformationToUndoList();
    void CalculateTransformation();
    void Translate(int* translateVector);
    void AlignCenters();

    /*
    * stores whether the image will be shown in grayvalues or in red for fixed image and green for moving image
    * @param if true, then images will be shown in red and green
    */
    void ShowRedGreen(bool show);

    /*
    * set the selected opacity for moving image
    * @param opacity the selected opacity
    */
    void OpacityUpdate(float opacity);

    /*
    * sets the images to grayvalues or fixed image to red and moving image to green
    * @param if true, then images will be shown in red and green
    */
    void SetImageColor(bool redGreen);

    void ClearTransformationLists();

    void ShowFixedImage();

    void ShowMovingImage();

    void ShowBothImages();

  protected:

    /*!  
    * default main widget containing 4 windows showing 3   
    * orthogonal slices of the volume and a 3d render window  
    */  
    QmitkStdMultiWidget * m_MultiWidget;

    /*!  
    * control widget to make all changes for Rigid registration 
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
