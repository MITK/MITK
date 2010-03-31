/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include "mitkDataNode.h"
#include "MitkDeformableRegistrationUIExports.h"
#include "ui_QmitkBSplineRegistrationViewControls.h"
#include "mitkOptimizerParameters.h"


/*!
* \brief Widget for deformable demons registration
*
* Displays options for demons registration.
*/
class MITK_DEFORMABLEREGISTRATION_UI_EXPORT QmitkBSplineRegistrationView : public QWidget
{
  Q_OBJECT

public:

  QmitkBSplineRegistrationView( QWidget* parent = 0, Qt::WindowFlags f = 0 );
  ~QmitkBSplineRegistrationView();

  int GetNumberOfIterations();
  Ui::QmitkBSplineRegistrationViewControls m_Controls;
  
  void SetFixedNode( mitk::DataNode * fixedNode );
  void SetMovingNode( mitk::DataNode * movingNode );

  

  
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
  
  mitk::DataNode* m_FixedNode;
  mitk::DataNode* m_MovingNode;
  mitk::OptimizerParameters::Pointer m_OptimizerParameters;

 
  void setOptimizerParameters();
};

#endif