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

#ifndef QmitkDemonsRegistrationViewWidgetHIncluded
#define QmitkDemonsRegistrationViewWidgetHIncluded

#include "mitkDataNode.h"
#include "ui_QmitkDemonsRegistrationViewControls.h"
#include "MitkDeformableRegistrationUIExports.h"


/*!
* \brief Widget for deformable demons registration
*
* Displays options for demons registration.
*/
class MITKDEFORMABLEREGISTRATIONUI_EXPORT QmitkDemonsRegistrationView : public QWidget
{
  Q_OBJECT

public:

  QmitkDemonsRegistrationView( QWidget* parent = nullptr, Qt::WindowFlags f = nullptr );
  ~QmitkDemonsRegistrationView();

  int GetNumberOfIterations();
  float GetStandardDeviation();

  void SetFixedNode( mitk::DataNode * fixedNode );
  void SetMovingNode( mitk::DataNode * movingNode );
  void UseHistogramMatching( bool useHM );
  mitk::Image::Pointer GetResultImage();
  mitk::Image::Pointer GetResultDeformationfield();

public slots:
  void CalculateTransformation();
protected:
  Ui::QmitkDemonsRegistrationViewControls m_Controls;
  mitk::DataNode::Pointer m_FixedNode;
  mitk::DataNode::Pointer m_MovingNode;
  mitk::Image::Pointer m_ResultImage;
  mitk::Image::Pointer m_ResultDeformationField;
};

#endif
