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
  setAvailability(true);
}

QmitkVolumetry::~QmitkVolumetry()
{
}

QWidget * QmitkVolumetry::createMainWidget(QWidget *parent)
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

QWidget * QmitkVolumetry::createControlWidget(QWidget *parent)
{  
  if (m_Controls == NULL)  
  {    
    m_Controls = new QmitkVolumetryControls(parent);          
    m_Controls->SetDataTreeIterator(getDataTree());
  }  
  return m_Controls;
}

void QmitkVolumetry::createConnections()
{  
  if ( m_Controls )   
  {    
    connect(m_Controls, SIGNAL( InitializeStandardViews(mitk::BaseData::Pointer) ), this, SLOT( InitializeStandardViews(mitk::BaseData::Pointer) ));
  }
}

QAction * QmitkVolumetry::createAction(QActionGroup *parent)
{  
  QAction* action;  
  action = new QAction( tr( "Volumetry" ), QPixmap((const char**)volumetry_xpm), tr( "Volumetry" ), CTRL + Key_G, parent, "Volumetry" );
  return action;
}

void QmitkVolumetry::treeChanged()
{
  m_Controls->m_TreeNodeSelector->UpdateContent();
}

void QmitkVolumetry::activated()
{  
  QmitkFunctionality::activated();
}
void QmitkVolumetry::InitializeStandardViews(mitk::BaseData::Pointer data)
{
  m_MultiWidget->InitializeStandardViews( data->GetGeometry() );
}
