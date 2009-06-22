/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date: 2009-05-20 18:43:21 +0200 (Wed, 20 May 2009) $
Version:   $Revision: 1.12 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkBSplineRegistrationViewWidgetHIncluded
#define QmitkBSplineRegistrationViewWidgetHIncluded

#include "mitkDataTreeNode.h"
#include "ui_QmitkBSplineRegistrationViewControls.h"
#include "qobject.h"
#include "../DeformableregistrationDll.h"
#include "mitkOptimizerParameters.h"


/*!
* \brief Widget for deformable demons registration
*
* Displays options for demons registration.
*/
class DEFORMABLEREGISTRATION_EXPORT QmitkBSplineRegistrationView : public QWidget
{
  Q_OBJECT

public:

  QmitkBSplineRegistrationView( QWidget* parent = 0, Qt::WindowFlags f = 0 );
  ~QmitkBSplineRegistrationView();

  int GetNumberOfIterations();
  Ui::QmitkBSplineRegistrationViewControls m_Controls;
  
  void SetFixedNode( mitk::DataTreeNode * fixedNode );
  void SetMovingNode( mitk::DataTreeNode * movingNode );

  

  
public slots:
  void CalculateTransformation();


protected slots:
 /*!
   * Prints the values of the deformationfield 
   */
  void PrintDeformationField();

 

  /*!
   * Select a deformation field
   */
  void SelectDeformationField();

  void OptimizerSelected(int optimizer);
  void HideAllOptimizerFrames();


protected:
  
  mitk::DataTreeNode* m_FixedNode;
  mitk::DataTreeNode* m_MovingNode;
  mitk::OptimizerParameters::Pointer m_OptimizerParameters;

 
  void setOptimizerParameters();
};

#endif