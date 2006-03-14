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

#include "QmitkVolumetry.h"
#include "QmitkVolumetryControls.h"
#include "QmitkVolumetryWidget.h"
#include <qaction.h>
#include "icon.xpm"
#include <mitkEventMapper.h>
#include <mitkGlobalInteraction.h>
#include <mitkBaseRenderer.h>
#include "QmitkRenderWindow.h"
#include "QmitkSelectableGLWidget.h"
#include "QmitkStdMultiWidget.h"
#include <QmitkStepperAdapter.h>
#include "qpushbutton.h"
#include "QmitkTreeNodeSelector.h"

// for stereo setting
#include <mitkOpenGLRenderer.h>
#include <mitkVtkRenderWindow.h>
#include <vtkRenderWindow.h>

// for zoom/pan
#include <mitkDisplayCoordinateOperation.h>
#include <mitkDisplayVectorInteractor.h>
#include <mitkDisplayInteractor.h>
#include <mitkInteractionConst.h>

QmitkVolumetry::QmitkVolumetry(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it) 
: QmitkFunctionality(parent, name, it) ,m_Controls(NULL), m_MultiWidget(mitkStdMultiWidget)
{
  SetAvailability(true);
}

QmitkVolumetry::~QmitkVolumetry()
{
}

QWidget * QmitkVolumetry::CreateMainWidget(QWidget *parent)
{  
  if (m_MultiWidget == NULL) 
  {
    return m_MultiWidget = new QmitkStdMultiWidget(parent);
  }
  else
  {
    return NULL;
  }
}

QWidget * QmitkVolumetry::CreateControlWidget(QWidget *parent)
{  
  if (m_Controls == NULL)  
  {    
    m_Controls = new QmitkVolumetryControls(parent);          
    m_Controls->SetDataTreeIterator(GetDataTree());
    m_Controls->m_TreeNodeSelector->m_FilterFunction=QmitkVolumetry::IsImageNodeButNotOverlayNode;
  }  
  return m_Controls;
}

void QmitkVolumetry::CreateConnections()
{  
  if ( m_Controls )   
  {    
    // connect(m_Controls, SIGNAL( InitializeStandardViews(mitk::BaseData::Pointer) ), this, SLOT( InitializeStandardViews(mitk::BaseData::Pointer) ));
  }
}

QAction * QmitkVolumetry::CreateAction(QActionGroup *parent)
{  
  QAction* action;  
  action = new QAction( tr( "Volumetry" ), QPixmap((const char**)volumetry_xpm), tr( "Volumetry" ), NULL, parent, "Volumetry" );
  return action;
}

void QmitkVolumetry::TreeChanged()
{
  m_Controls->m_TreeNodeSelector->UpdateContent();
}

void QmitkVolumetry::Activated()
{  
  QmitkFunctionality::Activated();

  mitk::DataTreeNode* overlayNode = m_Controls->m_VolumetryWidget->GetOverlayNode();
  if(overlayNode)
    overlayNode->SetVisibility(true);
}

void QmitkVolumetry::Deactivated()
{  
  mitk::DataTreeNode* overlayNode = m_Controls->m_VolumetryWidget->GetOverlayNode();
  if(overlayNode)
    overlayNode->SetVisibility(false);

  QmitkFunctionality::Deactivated();
}

void QmitkVolumetry::InitializeStandardViews(mitk::BaseData::Pointer data)
{
  m_MultiWidget->InitializeStandardViews( data->GetGeometry() );
}

bool QmitkVolumetry::IsImageNodeButNotOverlayNode( mitk::DataTreeNode * node )
{
  bool result = node && node->GetData() && dynamic_cast<mitk::Image*>(node->GetData());
  if(result)
  {
    std::string name;
    node->GetName(name);
    result = ! (name=="volume threshold overlay image");
  }
  return result;
}


