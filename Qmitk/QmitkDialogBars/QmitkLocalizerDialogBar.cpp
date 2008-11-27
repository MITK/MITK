/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-10-17 14:11:00 +0100 (Mi, 17 Okt 2008) $
Version:   $Revision: 11316 $ 

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkLocalizerDialogBar.h"
#include "QmitkStdMultiWidget.h"
#include "QmitkLocalizerDialogBar.xpm"
#include "mitkStatusBar.h"

#include <qaction.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qgrid.h>
#include <qcombobox.h>
#include <qlayout.h>

#include <QmitkTreeNodeSelector.h>
#include <QmitkRenderWindow.h>
#include "vtkVolumeProperty.h"
#include "vtkPiecewiseFunction.h"
#include "vtkColorTransferFunction.h"
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkImageThreshold.h>
#include <vtkImageData.h>

#include <mitkGlobalInteraction.h>
#include <QmitkStdMultiWidget.h>
#include <qgroupbox.h>


QmitkLocalizerDialogBar
::QmitkLocalizerDialogBar( 
  QObject *parent, const char *name, QmitkStdMultiWidget *multiWidget, 
  mitk::DataTreeIteratorBase *dataIt )
: QmitkDialogBar( "Localizer", parent, name, multiWidget, dataIt )
{
  m_Storage = mitk::DataStorage::GetInstance();

  m_Punkt = mitk::Ellipsoid::New();

  node = mitk::DataTreeNode::New();

  m_Volume = vtkVolume::New();
  

  m_FileUsed = "";

  float spacing[3];
  spacing[0] = (float) 10.0;
  spacing[1] = (float) 10.0;
  spacing[2] = (float) 10.0;
  m_Punkt->GetGeometry()->SetSpacing(spacing);
}

QmitkLocalizerDialogBar
::~QmitkLocalizerDialogBar()
{
}


QString 
QmitkLocalizerDialogBar
::GetFunctionalityName()
{
  return "LocalizerDialogBar";
}


QAction *
QmitkLocalizerDialogBar
::CreateAction( QObject *parent )
{
  QAction* action;
  action = new QAction( 
    tr( "Localizer" ), 
    QPixmap((const char**)QmitkLocalizerDialogBar_xpm), 
    tr( "Localizer" ), 
    0, 
    parent, 
    "QmitkLocalizerDialogBar" );

  return action;
}

QWidget *
QmitkLocalizerDialogBar
::CreateDialogBar( QWidget *parent )
{
          
  new QVBox( parent );

  QGrid *grid = new QGrid( 5, Qt::Vertical, parent );

  grid->setSpacing(7);

  QLabel *fileLabel = new QLabel(grid, "choose a picture");
  fileLabel->setText(fileLabel->name());

  m_TreeNodeSelector = new QmitkTreeNodeSelector(grid, "LocalizerFileChooser");
  m_TreeNodeSelector->SetDataTreeNodeIterator(this->GetDataTreeIterator());

  m_LocalizerRenderWindow = new QmitkRenderWindow(grid, "LocalizerRenderWindow");
  m_LocalizerRenderWindow->GetRenderWindow()->SetWindowName("LocalizerRenderWindow");
  m_LocalizerRenderWindow->GetRenderWindow()->GetInteractor()->Disable();

  QLabel *viewLabel = new QLabel(grid, "choose a view");
  viewLabel->setText(viewLabel->name());
  
  QComboBox *m_ViewChooser = new QComboBox(grid, "LocalizerViewChooser");
  m_ViewChooser->insertItem("Frontal", 0);
  m_ViewChooser->insertItem("Right", 1);
  m_ViewChooser->insertItem("Bottom", 2);

  node->SetVisibility(true, m_LocalizerRenderWindow->GetRenderer(), "visible");


  connect( (QObject*)(m_TreeNodeSelector), SIGNAL( Activated(mitk::DataTreeIteratorClone)),(QObject*) this, SLOT(ImageSelected(mitk::DataTreeIteratorClone)) );
  connect( (m_ViewChooser), SIGNAL(activated(const QString&) ),(QObject*) this, SLOT(SetCamera(const QString&) ) );

  connect( (QObject*)(m_MultiWidget), SIGNAL(WheelMoved( QWheelEvent* ) ), (QObject*) this, SLOT(SetSeeker( QWheelEvent* )) );

  m_LocalizerRenderWindow->setGeometry( QRect( 5, 5, 100, 100 ) );
  m_LocalizerRenderWindow->setMinimumSize(200, 200);

  ToggleVisible(true);
  
  return grid;
}

void QmitkLocalizerDialogBar::DrawVolume()
{
  mitk::StatusBar::GetInstance()->DisplayText("The Localizer is computing the volume", 4000);
  mitk::DataTreeNode::Pointer imageNode;
  if (m_TreeNodeSelector->GetSelectedNode() != NULL){
    if (mitk::DataTreeNode::Pointer imageNode = m_Storage->GetNamedNode(m_TreeNodeSelector->GetSelectedNode()->GetName().c_str()))
  	  if (mitk::Image::Pointer mitkImage = dynamic_cast<mitk::Image*>(imageNode->GetData()))
  		  if (vtkImageData* vtkImage = mitkImage->GetVtkImageData()){
          m_FileUsed = m_TreeNodeSelector->GetSelectedNode()->GetName().c_str();
          m_Volume = createVolume(vtkImage);          
          m_LocalizerRenderWindow->GetRenderer()->GetVtkRenderer()->AddViewProp(m_Volume);
          
          SetSeeker(m_Punkt);

          if (m_Storage->GetNamedNode("Localizer") != NULL)
          {
            m_Storage->Remove(m_Storage->GetNamedNode("Localizer"));
          }
          SetViewProperties(m_LocalizerRenderWindow);

          CreateNode();
          
        }
  }
  m_LocalizerRenderWindow->repaint();
  SetSeeker(m_Punkt);
}

void QmitkLocalizerDialogBar::CreateNode()
{
  SetSeeker(m_Punkt);
  node->SetName("Localizer");
  node->SetData(m_Punkt);

  node->SetColor(0.0, 1.0, 1.0);
  
  node->SetVisibility(false, m_MultiWidget->GetRenderWindow1()->GetRenderer(), "visible");
  node->SetVisibility(false, m_MultiWidget->GetRenderWindow2()->GetRenderer(), "visible");
  node->SetVisibility(false, m_MultiWidget->GetRenderWindow3()->GetRenderer(), "visible");
  node->SetVisibility(false, m_MultiWidget->GetRenderWindow4()->GetRenderer(), "visible");
  node->SetVisibility(true, m_LocalizerRenderWindow->GetRenderer(), "visible");
  m_Storage->Add(node);
}

void QmitkLocalizerDialogBar::UpdateGeometry(const itk::EventObject &geometryUpdateEvent)
{
  SetSeeker(m_Punkt);
}


void QmitkLocalizerDialogBar::Activated()
{
  m_TreeNodeSelector->UpdateContent();

  mitk::BaseRenderer::GetInstance(m_MultiWidget->GetRenderWindow1()->GetRenderWindow())->GetSliceNavigationController()->ConnectGeometryUpdateEvent(this);
  mitk::BaseRenderer::GetInstance(m_MultiWidget->GetRenderWindow2()->GetRenderWindow())->GetSliceNavigationController()->ConnectGeometryUpdateEvent(this);
  mitk::BaseRenderer::GetInstance(m_MultiWidget->GetRenderWindow3()->GetRenderWindow())->GetSliceNavigationController()->ConnectGeometryUpdateEvent(this);
  

  m_Storage->AddNodeEvent += mitk::MessageDelegate1<QmitkLocalizerDialogBar, const mitk::DataTreeNode*>(this, &QmitkLocalizerDialogBar::OnNewNode);
  m_Storage->RemoveNodeEvent += mitk::MessageDelegate1<QmitkLocalizerDialogBar, const mitk::DataTreeNode*>(this, &QmitkLocalizerDialogBar::OnNewNode);


  m_LocalizerRenderWindow->GetRenderer()->SetData(m_Storage);
  m_LocalizerRenderWindow->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);
  SetViewProperties(m_LocalizerRenderWindow);  
  if ( (m_FileUsed != "") && wasActive )
  {
    CreateNode();
  }

  if ( (m_FileUsed == "") || (m_TreeNodeSelector->GetSelectedNode() == NULL) )
  {
    MakeClean();
  }
 
  SetSeeker(m_Punkt);
  m_LocalizerRenderWindow->repaint();
}

void QmitkLocalizerDialogBar::OnNewNode(const mitk::DataTreeNode* /*n*/)
{
  m_TreeNodeSelector->UpdateContent();
}

void QmitkLocalizerDialogBar::Deactivated()
{
  m_Storage->Remove(mitk::DataStorage::GetInstance()->GetNamedNode("Localizer"));

  m_Storage->AddNodeEvent -= mitk::MessageDelegate1<QmitkLocalizerDialogBar, const mitk::DataTreeNode*>(this, &QmitkLocalizerDialogBar::OnNewNode);
  m_Storage->RemoveNodeEvent -= mitk::MessageDelegate1<QmitkLocalizerDialogBar, const mitk::DataTreeNode*>(this, &QmitkLocalizerDialogBar::OnNewNode);

  if (m_TreeNodeSelector->GetSelectedNode() == NULL)
  {
    MakeClean();
  }
  wasActive = true;

  if (m_LocalizerRenderWindow->GetRenderer()->GetVtkRenderer()->HasViewProp(m_Volume) == false)
  {
    wasActive = false;
  }

  
}

void QmitkLocalizerDialogBar::MakeClean()
{
  m_Storage->Remove(m_Storage->GetNamedNode("Localizer"));
  m_LocalizerRenderWindow->GetRenderer()->GetVtkRenderer()->RemoveViewProp(m_Volume);

  m_FileUsed = "";
}


vtkVolume* QmitkLocalizerDialogBar::createVolume(vtkImageData* vtkImage)
{
  vtkPiecewiseFunction *oTFun = vtkPiecewiseFunction::New();
  vtkColorTransferFunction *cTFun = vtkColorTransferFunction::New();
  vtkPiecewiseFunction *goTFun = vtkPiecewiseFunction::New();
  vtkVolumeProperty *volumeProperty = vtkVolumeProperty::New();
  //Create a FixedPointVolumeRayCastMapper to calculate the VolumeRendering
  vtkFixedPointVolumeRayCastMapper *volumeMapper = vtkFixedPointVolumeRayCastMapper::New();
  vtkImageThreshold* thresh = vtkImageThreshold::New();

  // Create a transfer function mapping scalar value to opacity
  oTFun->AddPoint(50,0);
  oTFun->AddPoint(150,0.25);
  oTFun->AddPoint(350,0.65);
  oTFun->AddPoint(650,0.80);

  // Create a transfer function mapping scalar value to color (color)
  cTFun->AddRGBPoint(  64, 0.0, 0.0, 0.0 );
  cTFun->AddRGBPoint( 128, 0.5, 0.5, 0.5 );
  cTFun->AddRGBPoint( 255, 0.6, 0.6, 0.6 );
  cTFun->AddRGBPoint( 450, 0.8, 0.8, 0.8 );

  // Create a transfer function mapping magnitude of gradient to opacity
  goTFun->AddPoint(   0, 0.0 );
  goTFun->AddPoint(  50, 0.2 );
  goTFun->AddPoint(  120, 0.7 );
  goTFun->AddPoint( 400, 0.9 );

  //Create Property with just created transfer functions
  volumeProperty->SetColor(cTFun);
  volumeProperty->SetScalarOpacity(oTFun);
  volumeProperty->SetGradientOpacity(goTFun);

  //volumeMapper uses its own MIP-functionality
  volumeMapper->SetBlendModeToMaximumIntensity();

  //Create ImageThreshold to be used as Input
  thresh->SetInput(vtkImage);
  thresh->ReplaceInOn();
  thresh->SetInValue(0);
  thresh->ThresholdBetween(250, 270);
  
  volumeMapper->SetInputConnection(thresh->GetOutputPort());

  //Create new Volume
  m_Volume->SetMapper(volumeMapper);
	m_Volume->SetProperty(volumeProperty);
  
  //delete old stuff to deallocate memory
  thresh->Delete();
  volumeMapper->Delete();
  volumeProperty->Delete();

  return m_Volume;
}

void QmitkLocalizerDialogBar::SetCamera(const QString& view = "Frontal")
{
  m_LocalizerRenderWindow->GetRenderer()->GetCameraController()->SetZoomFactor(2.0);

  if ( view == "Frontal" ) m_LocalizerRenderWindow->GetRenderer()->GetCameraController()->SetStandardView(mitk::CameraController::ANTERIOR);
  else if ( view == "Right" ) m_LocalizerRenderWindow->GetRenderer()->GetCameraController()->SetStandardView(mitk::CameraController::SINISTER);
  else if ( view == "Bottom" ) m_LocalizerRenderWindow->GetRenderer()->GetCameraController()->SetStandardView(mitk::CameraController::CAUDAL);
}


void QmitkLocalizerDialogBar::SetViewProperties(QmitkRenderWindow *renderwindow)
{
  SetCamera("Frontal");
  renderwindow->GetRenderWindow()->SetPointSmoothing(1);
  renderwindow->GetRenderWindow()->SetPolygonSmoothing(1);  

  m_Storage->GetNamedNode("widget1Plane")->SetVisibility(false, renderwindow->GetRenderer(), "visible");
  m_Storage->GetNamedNode("widget2Plane")->SetVisibility(false, renderwindow->GetRenderer(), "visible");
  m_Storage->GetNamedNode("widget3Plane")->SetVisibility(false, renderwindow->GetRenderer(), "visible");
}


void QmitkLocalizerDialogBar::SetSeeker(mitk::Ellipsoid::Pointer m_Punkt){
  m_Punkt->GetGeometry()->SetOrigin( m_MultiWidget->GetCrossPosition());
}


void QmitkLocalizerDialogBar::SetSeeker(QWheelEvent* /*e*/)
{
  SetSeeker(m_Punkt);
}

void QmitkLocalizerDialogBar::ImageSelected(mitk::DataTreeIteratorClone /*imageIt*/)
{
  if (m_TreeNodeSelector->GetSelectedNode()->GetName().c_str() != m_FileUsed)
  {
    m_LocalizerRenderWindow->GetRenderer()->SetData(m_Storage);
    SetSeeker(m_Punkt);
    DrawVolume();
    m_LocalizerRenderWindow->update();
  }
}
