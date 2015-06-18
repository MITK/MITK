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
class MITKDEFORMABLEREGISTRATIONUI_EXPORT QmitkBSplineRegistrationView : public QWidget
{
  Q_OBJECT

public:

  QmitkBSplineRegistrationView( QWidget* parent = nullptr, Qt::WindowFlags f = nullptr );
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
