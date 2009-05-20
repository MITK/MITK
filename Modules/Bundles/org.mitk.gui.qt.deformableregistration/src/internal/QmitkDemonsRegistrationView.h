/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkDemonsRegistrationViewWidgetHIncluded
#define QmitkDemonsRegistrationViewWidgetHIncluded

#include "mitkDataTreeNode.h"
#include "ui_QmitkDemonsRegistrationViewControls.h"
#include "qobject.h"
#include "../DeformableregistrationDll.h"


/*!
* \brief Widget for deformable demons registration
*
* Displays options for demons registration.
*/
class DEFORMABLEREGISTRATION_EXPORTS QmitkDemonsRegistrationView : public QWidget
{
  Q_OBJECT

public:

  QmitkDemonsRegistrationView( QWidget* parent = 0, Qt::WindowFlags f = 0 );
  ~QmitkDemonsRegistrationView();

  int GetNumberOfIterations();
  float GetStandardDeviation();
  
  void SetFixedNode( mitk::DataTreeNode * fixedNode );
  void SetMovingNode( mitk::DataTreeNode * movingNode );
  void UseHistogramMatching( bool useHM );
public slots:
  void CalculateTransformation();
protected:
  Ui::QmitkDemonsRegistrationViewControls m_Controls;
  mitk::DataTreeNode* m_FixedNode;
  mitk::DataTreeNode* m_MovingNode;
};

#endif