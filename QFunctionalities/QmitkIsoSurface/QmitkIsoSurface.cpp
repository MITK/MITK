/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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

#include <QmitkIsoSurface.h>
#include <QmitkIsoSurfaceControls.h>//GUI
#include <isoSurface.xpm> //image

//MITK headers
#include <mitkSurface.h>
#include <mitkManualSegmentationToSurfaceFilter.h>

//Qmitk headers
#include <QmitkCommonFunctionality.h>
#include <QmitkSelectableGLWidget.h>
#include <QmitkStdMultiWidget.h>
#include <QmitkDataTreeComboBox.h>

//Qt-GUI headers
#include <qaction.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qmessagebox.h>

#include <string.h>


QmitkIsoSurface::QmitkIsoSurface(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
: QmitkFunctionality(parent, name, it) , 
  m_MultiWidget(mitkStdMultiWidget), 
  m_Controls(NULL),
  m_MitkImage(NULL),
  m_SurfaceCounter(0)
{
  SetAvailability(true);
}



QWidget * QmitkIsoSurface::CreateMainWidget(QWidget *)
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
    connect( (QObject*)(m_Controls->m_ImageSelector), SIGNAL(activated(const mitk::DataTreeFilter::Item*)),(QObject*) this, SLOT(ImageSelected(const mitk::DataTreeFilter::Item*)) );
    m_Controls->m_ImageSelector->SetDataTree(GetDataTreeIterator());
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
    m_Controls->m_ImageSelector->Update();
}


void QmitkIsoSurface::ImageSelected(const mitk::DataTreeFilter::Item* item)
{
  m_MitkImage = dynamic_cast<mitk::Image*> (item->GetNode()->GetData());
}


void QmitkIsoSurface::CreateSurface()
{
  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  if(m_MitkImage != NULL)
  {
    //Value Gauss
    //float gsDev = 1.5;

    //Value for DecimatePro 
    float targetReduction = 0.05;

    //ImageToSurface Instance
    mitk::DataTreeIteratorClone  iteratorOnImageToBeSkinExtracted;
    
    iteratorOnImageToBeSkinExtracted = m_Controls->m_ImageSelector->GetFilter()->GetIteratorToSelectedItem();
    if(iteratorOnImageToBeSkinExtracted.IsNull())
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
    filter->SetGaussianStandardDeviation( 0.5 );
    filter->SetUseGaussianImageSmooth( true );
    filter->SetThreshold( getThreshold()); //if( Gauss ) --> TH manipulated for vtkMarchingCube

    filter->SetTargetReduction( targetReduction );


    int numOfPolys = filter->GetOutput()->GetVtkPolyData()->GetNumberOfPolys();
    if(numOfPolys>2000000)
    {
      QApplication::restoreOverrideCursor();
      if(QMessageBox::question(NULL, "CAUTION!!!", "The number of polygons is greater than 2 000 000. If you continue, the program might crash. How do you want to go on?", "Proceed anyway!", "Cancel immediately! (maybe you want to insert an other threshold)!",QString::null,0 ,1)==1)
      {
        
        return;
      }
          QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
    }
    mitk::DataTreeNode::Pointer surfaceNode = mitk::DataTreeNode::New(); 
    surfaceNode->SetData( filter->GetOutput() );

    int layer = 0;

    ++m_SurfaceCounter;
    std::ostringstream buffer;
    buffer << m_SurfaceCounter;
    std::string surfaceNodeName = "Surface " + buffer.str();

    iteratorOnImageToBeSkinExtracted->Get()->GetIntProperty("layer", layer);
    mitk::DataTreeNodeFactory::SetDefaultSurfaceProperties(surfaceNode);
    surfaceNode->SetIntProperty("layer", layer+1);
    surfaceNode->SetProperty("Surface", mitk::BoolProperty::New(true));
    surfaceNode->SetProperty("name", mitk::StringProperty::New(surfaceNodeName));


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
    surfaceNode->SetColor( m_RainbowColor.GetNextColor() );
    surfaceNode->SetVisibility(true);

    m_MultiWidget->RequestUpdate();

  }//if m_MitkImage != NULL

  QApplication::restoreOverrideCursor();
}


float QmitkIsoSurface::getThreshold()
{
  return m_Controls->getThreshold()->text().toFloat();
}

