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

#include "QmitkIsoSurface.h"
#include "QmitkIsoSurfaceControls.h"//GUI
#include "isoSurface.xpm" //image

//MITK-Elements
#include <mitkManualSegmentationToSurfaceFilter.h>
#include <mitkColorSequenceRainbow.h>

//QMitk-Elements
#include "QmitkCommonFunctionality.h"
#include <QmitkMultiTreeNodeSelector.h>
#include "QmitkSelectableGLWidget.h"
#include "QmitkStdMultiWidget.h"
#include "QmitkTreeNodeSelector.h"

//QT-GUI-Elements
#include <qaction.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qlineedit.h>
#include "qpushbutton.h"

#include <string.h>


QmitkIsoSurface::QmitkIsoSurface(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
    : QmitkFunctionality(parent, name, it) ,m_Controls(NULL), m_MultiWidget(mitkStdMultiWidget)
{
  SetAvailability(true);
  m_SurfaceCounter = 0;
  m_MitkImage  = NULL;
  m_r = 0.75;
  m_g = 0.42;
  m_b = -0.75;
  m_Color.Set(1.0, 0.67, 0.0);
  m_RainbowColor = new mitk::ColorSequenceRainbow();
}



QWidget * QmitkIsoSurface::CreateMainWidget(QWidget *parent)
{
  return NULL;
}

QWidget * QmitkIsoSurface::CreateControlWidget(QWidget *parent)
{
  if (m_Controls == NULL)
  {
    m_Controls = new QmitkIsoSurfaceControls(parent);
  }
  return m_Controls;
}

void QmitkIsoSurface::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_TreeNodeSelector), SIGNAL(Activated(mitk::DataTreeIteratorClone)),(QObject*) this, SLOT(ImageSelected(mitk::DataTreeIteratorClone)) );
    connect(m_Controls->getCreateSurfaceButton(), SIGNAL(clicked()), this, SLOT(CreateSurface()) );  
  }
}

QAction * QmitkIsoSurface::CreateAction(QActionGroup *parent)
{
  QAction* action;
  action = new QAction( tr( "IsoSurface" ), QPixmap((const char**)isoSurface_xpm), tr( "IsoSurface" ), 0, parent, "IsoSurface" );
  return action;
}

void QmitkIsoSurface::TreeChanged()
{
  m_Controls->m_TreeNodeSelector->SetDataTreeNodeIterator(this->GetDataTree());
}

void QmitkIsoSurface::Activated()
{
  QmitkFunctionality::Activated();
}
void QmitkIsoSurface::ImageSelected(mitk::DataTreeIteratorClone imageIt)
{
  std::string name;
  if (imageIt->Get()->GetName(name))
  {
    std::cout << "Tree node selected with name '" << name << "'" << std::endl;
  }

  m_MitkImageIterator = imageIt;
  m_MitkImage = static_cast<mitk::Image*> (imageIt->Get()->GetData());
}


void QmitkIsoSurface::CreateSurface()
{
  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  if(m_MitkImage != NULL)
  {
  //Value Gauss
  float gsDev = 1.5;

  //Value for DecimatePro 
  float targetReduction = 0.05;

  //ImageToSurface Instance
  mitk::DataTreeIteratorClone  iteratorOnImageToBeSkinExtracted;
  if(m_MitkImageIterator != NULL)
  {
      
    iteratorOnImageToBeSkinExtracted = m_MitkImageIterator;
  }
  else 
  {
    iteratorOnImageToBeSkinExtracted = CommonFunctionality::GetIteratorToFirstImage(m_DataTreeIterator.GetPointer());
  }

  mitk::ManualSegmentationToSurfaceFilter::Pointer filter = mitk::ManualSegmentationToSurfaceFilter::New();
  if (filter.IsNull())
  {
    std::cout<<"NULL Pointer for ManualSegmentationToSurfaceFilter"<<std::endl;
    return;
  }

  filter->SetInput( m_MitkImage );  
  filter->SetStandardDeviation( 0.5 );
  filter->SetUseStandardDeviation( true );
  filter->SetThreshold( getThreshold()); //if( Gauss ) --> TH manipulated for vtkMarchingCube
  

  filter->SetTargetReduction( targetReduction );

  mitk::DataTreeNode::Pointer surfaceNode = mitk::DataTreeNode::New(); 
  surfaceNode->SetData( filter->GetOutput() );
  
  int layer = 0;

  ++m_SurfaceCounter;
  std::ostringstream buffer;
  buffer << m_SurfaceCounter;
  std::string surfaceNodeName = "Surface " + buffer.str();

  m_r += 0.25; if(m_r > 1){m_r = m_r - 1;}
  m_g += 0.25; if(m_g > 1){m_g = m_g - 1;}
  m_b += 0.25; if(m_b > 1){m_b = m_b - 1;}

  iteratorOnImageToBeSkinExtracted->Get()->GetIntProperty("layer", layer);
  //mitk::DataTreeNodeFactory::SetDefaultSurfaceProperties(surfaceNode);
  surfaceNode->SetIntProperty("layer", layer+1);
  surfaceNode->SetProperty("Surface", new mitk::BoolProperty(true));
  surfaceNode->SetProperty("name", new mitk::StringProperty(surfaceNodeName));
  
  
  mitk::DataTreeIteratorClone iteratorClone = m_DataTreeIterator;
  bool isSurface = false;

  while(!(iteratorClone->IsAtEnd())&&(isSurface == false))
  {
    iteratorClone->Get()->GetBoolProperty("Surface", isSurface);
    if(isSurface == false)
    {
      ++iteratorClone;
    }
  }
   iteratorClone= iteratorOnImageToBeSkinExtracted;  
   iteratorClone->Add(surfaceNode);
   iteratorClone->GetTree()->Modified();


  mitk::Surface::Pointer surface = filter->GetOutput();
 
  //to show surfaceContur
  m_Color = m_RainbowColor->GetNextColor();
  surfaceNode->SetColor(m_Color);
  surfaceNode->SetVisibility(true);

  m_MultiWidget->RequestUpdate();

  std::cout<<"*************"<<std::endl;
  std::cout<<"*  surface  *"<<std::endl;
  std::cout<<"*************"<<std::endl;
  }//if m_MitkImage != NULL

  else
    std::cout << "No Image selected" << std::endl;

  QApplication::restoreOverrideCursor();
}

float QmitkIsoSurface::getThreshold()
{
  QLineEdit* lineEdit = (QLineEdit*) m_Controls->getThreshold();
  float threshold = atof(lineEdit->text());
  return threshold;
}//end of getThreshold()

QmitkIsoSurface::~QmitkIsoSurface()
{
  delete m_RainbowColor;
}