/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 15417 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkIsoSurface.h"
//#include <QmitkIsoSurfaceControls.h>//GUI
//#include <isoSurface.xpm> //image

//MITK headers
#include <mitkSurface.h>
#include <mitkManualSegmentationToSurfaceFilter.h>
#include <mitkIDataStorageService.h>
#include <mitkNodePredicateDataType.h>

//Qmitk headers
#include <QmitkCommonFunctionality.h>
#include <QmitkSelectableGLWidget.h>
#include <QmitkStdMultiWidget.h>

//Qt-GUI headers
#include <qaction.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qmessagebox.h>

#include <string.h>


QmitkIsoSurface::QmitkIsoSurface(QObject * /*parent*/, const char * /*name*/)
: QmitkFunctionality() ,
  m_Controls(NULL),
  m_MitkImage(NULL),
  m_SurfaceCounter(0)
{
  //SetAvailability(true);
}



//QWidget * QmitkIsoSurface::CreateMainWidget(QWidget *)
//{
//  return NULL;
//}


//QWidget * QmitkIsoSurface::CreateControlWidget(QWidget *parent)
//{
//  if (m_Controls == NULL)
//  {
//    m_Controls = new QmitkIsoSurfaceControls(parent);
//  }
//  return m_Controls;
//}

void QmitkIsoSurface::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    m_Controls = new Ui::QmitkIsoSurfaceControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    m_Controls->m_ImageSelector->SetDataStorage(this->GetDefaultDataStorage());
    m_Controls->m_ImageSelector->SetPredicate(mitk::NodePredicateDataType::New("Image"));

    cherry::IPreferences::Pointer prefs = this->GetPreferences();
    if(prefs.IsNotNull())
      m_Controls->thresholdLineEdit->setText(QString::fromStdString(prefs->Get("defaultThreshold", "0")));
  }
}

void QmitkIsoSurface::CreateConnections()
{
  if ( m_Controls )
  {
    connect( m_Controls->m_ImageSelector, SIGNAL(OnSelectionChanged(const mitk::DataTreeNode*)), this, SLOT(ImageSelected(const mitk::DataTreeNode*)) );
    connect( m_Controls->createSurfacePushButton, SIGNAL(clicked()), this, SLOT(CreateSurface()) );
  }
}


//QAction * QmitkIsoSurface::CreateAction(QActionGroup *parent)
//{
//  QAction* action;
//  action = new QAction( tr( "IsoSurface" ), QPixmap((const char**)isoSurface_xpm), tr( "IsoSurface" ), 0, parent, "IsoSurface" );
//  return action;
//}


void QmitkIsoSurface::DataStorageChanged()
{
    //m_Controls->m_ImageSelector->Update();
}


void QmitkIsoSurface::ImageSelected(const mitk::DataTreeNode* item)
{
  m_MitkImage = dynamic_cast<mitk::Image*> (item->GetData());
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
    mitk::DataTreeNode::Pointer node = m_Controls->m_ImageSelector->GetSelectedNode();

    /*
    mitk::DataTreeIteratorClone  iteratorOnImageToBeSkinExtracted;

    iteratorOnImageToBeSkinExtracted = m_Controls->m_ImageSelector->GetFilter()->GetIteratorToSelectedItem();
    if(iteratorOnImageToBeSkinExtracted.IsNull())
    {
      iteratorOnImageToBeSkinExtracted = CommonFunctionality::GetIteratorToFirstImage(m_DataTreeIterator.GetPointer());
    }
    */

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

    node->GetIntProperty("layer", layer);
    surfaceNode->SetIntProperty("layer", layer+1);
    surfaceNode->SetProperty("Surface", mitk::BoolProperty::New(true));
    surfaceNode->SetProperty("name", mitk::StringProperty::New(surfaceNodeName));

    /*
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
    */
    this->GetDefaultDataStorage()->Add(surfaceNode, node);

    mitk::Surface::Pointer surface = filter->GetOutput();

    //to show surfaceContur
    surfaceNode->SetColor( m_RainbowColor.GetNextColor() );
    surfaceNode->SetVisibility(true);

    this->GetActiveStdMultiWidget()->RequestUpdate();

  }//if m_MitkImage != NULL

  QApplication::restoreOverrideCursor();
}


float QmitkIsoSurface::getThreshold()
{
  return m_Controls->thresholdLineEdit->text().toFloat();
}

void QmitkIsoSurface::StdMultiWidgetAvailable( QmitkStdMultiWidget&  /*stdMultiWidget*/ )
{
  m_Parent->setEnabled(true);
}

void QmitkIsoSurface::StdMultiWidgetNotAvailable()
{
  m_Parent->setEnabled(false);
}

QmitkIsoSurface::~QmitkIsoSurface()
{

  cherry::IPreferences::Pointer prefs = this->GetPreferences();
  if(prefs.IsNotNull())
    prefs->Put("defaultThreshold", m_Controls->thresholdLineEdit->text().toStdString());
}