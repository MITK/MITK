/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkSegmentationPostProcessing.h"
#include "QmitkNodeDescriptorManager.h"
#include "QmitkToolGUI.h"

#include "mitkAutoCropImageFilter.h"
#include "mitkBinaryThresholdTool.h"
#include "mitkRenderingManager.h"
#include "mitkShowSegmentationAsSurface.h"
#include "mitkProgressBar.h"
#include "mitkStatusBar.h"
#include "mitkImageCast.h"
#include "mitkDataNodeObject.h"

#include <QtGui>

#include <berryIWorkbenchPage.h>

#include <itkConstantPadImageFilter.h>

QmitkSegmentationPostProcessing::QmitkSegmentationPostProcessing(mitk::DataStorage* storage, QmitkFunctionality* functionality, QObject* parent)
:QObject(parent)
,m_BlueBerryView(functionality)
,m_DataStorage(storage)
{
  // register a couple of additional actions for DataManager's context menu
  QmitkNodeDescriptor* imageDataNodeDescriptor = 
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("Image");

  if (imageDataNodeDescriptor)
  {
    m_ThresholdAction = new QAction("Threshold..", parent);
    imageDataNodeDescriptor->AddAction(m_ThresholdAction);
    connect( m_ThresholdAction, SIGNAL( triggered(bool) ) , this, SLOT( ThresholdImage(bool) ) );
  }
  else
  {
    MITK_WARN << "Could not get datamanager's node descriptor for 'Image'";
  }

  // register a couple of additional actions for DataManager's context menu
  QmitkNodeDescriptor* binaryImageDataNodeDescriptor = 
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("ImageMask");

  if (binaryImageDataNodeDescriptor)
  {
    m_CreateSurfaceAction = new QAction("Create polygon model", parent);
    binaryImageDataNodeDescriptor->AddAction(m_CreateSurfaceAction);
    connect( m_CreateSurfaceAction, SIGNAL( triggered(bool) ) , this, SLOT( CreateSurface(bool) ) );

    m_CreateSmoothSurfaceAction = new QAction("Create smoothed polygon model", parent);
    binaryImageDataNodeDescriptor->AddAction(m_CreateSmoothSurfaceAction);
    connect( m_CreateSmoothSurfaceAction, SIGNAL( triggered(bool) ) , this, SLOT( CreateSmoothedSurface(bool) ) );

    m_StatisticsAction = new QAction("Statistics", parent);
    binaryImageDataNodeDescriptor->AddAction(m_StatisticsAction);
    connect( m_StatisticsAction, SIGNAL( triggered(bool) ) , this, SLOT( ImageStatistics(bool) ) );
   
    m_AutocropAction = new QAction("Autocrop", parent);
    binaryImageDataNodeDescriptor->AddAction(m_AutocropAction);
    connect( m_AutocropAction, SIGNAL( triggered(bool) ) , this, SLOT( AutocropSelected(bool) ) );
  }
  else
  {
    MITK_WARN << "Could not get datamanager's node descriptor for 'ImageMask'";
  }
  
  // register for blueberry selection events
  m_SelectionListener = berry::ISelectionListener::Pointer(new berry::SelectionChangedAdapter<QmitkSegmentationPostProcessing>(this, &QmitkSegmentationPostProcessing::SelectionChanged));
  m_BlueBerryView->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelectionListener);
}

QmitkSegmentationPostProcessing::~QmitkSegmentationPostProcessing()
{
  berry::ISelectionService* s = m_BlueBerryView->GetSite()->GetWorkbenchWindow()->GetSelectionService();
  if(s)
  {
    s->RemovePostSelectionListener(m_SelectionListener);
  }

  // unregister a couple of additional actions for DataManager's context menu
  QmitkNodeDescriptor* imageDataNodeDescriptor = 
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("Image");

  if (imageDataNodeDescriptor)
  {
    imageDataNodeDescriptor->RemoveAction( m_ThresholdAction );
  }
  else
  {
    MITK_WARN << "Could not get datamanager's node descriptor for 'Image'";
  }

  // unregister a couple of additional actions for DataManager's context menu
  QmitkNodeDescriptor* binaryImageDataNodeDescriptor = 
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("ImageMask");

  if (binaryImageDataNodeDescriptor)
  {
    binaryImageDataNodeDescriptor->RemoveAction( m_CreateSurfaceAction );
    binaryImageDataNodeDescriptor->RemoveAction( m_CreateSmoothSurfaceAction );
    binaryImageDataNodeDescriptor->RemoveAction( m_StatisticsAction );
    binaryImageDataNodeDescriptor->RemoveAction( m_AutocropAction );
  }
  else
  {
    MITK_WARN << "Could not get datamanager's node descriptor for 'ImageMask'";
  }
}

void QmitkSegmentationPostProcessing::SelectionChanged(berry::IWorkbenchPart::Pointer sourcepart, berry::ISelection::ConstPointer selection)
{
  if ( selection.IsNull()  )
  {
    return;
  }

  // save current selection in member variable
  m_CurrentSelection = selection.Cast<const mitk::DataNodeSelection>();
}

QmitkSegmentationPostProcessing::NodeList QmitkSegmentationPostProcessing::GetSelectedNodes() const
{
  NodeList result;
  if (m_CurrentSelection)
  {

    // iterate selection
    for (mitk::DataNodeSelection::iterator i = m_CurrentSelection->Begin(); i != m_CurrentSelection->End(); ++i)
    {
      // extract datatree node
      if (mitk::DataNodeObject::Pointer nodeObj = i->Cast<mitk::DataNodeObject>())
      {
        mitk::DataNode::Pointer node = nodeObj->GetDataNode();
        result.push_back( node );
      }
    }
  }

  return result;
}
 
void QmitkSegmentationPostProcessing::ThresholdImage(bool)
{
  NodeList selection = this->GetSelectedNodes();

  m_ThresholdingToolManager = mitk::ToolManager::New( m_DataStorage );
  m_ThresholdingToolManager->RegisterClient();
  m_ThresholdingToolManager->ActiveToolChanged += 
    mitk::MessageDelegate<QmitkSegmentationPostProcessing>( this, &QmitkSegmentationPostProcessing::OnThresholdingToolManagerToolModified );

  m_ThresholdingDialog = new QDialog(NULL);
  connect( m_ThresholdingDialog, SIGNAL(finished(int)), this, SLOT(ThresholdingDone(int)) );

  QVBoxLayout* layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);

  mitk::Tool* tool = m_ThresholdingToolManager->GetToolById( m_ThresholdingToolManager->GetToolIdByToolType<mitk::BinaryThresholdTool>() );
  if (tool)
  {
    itk::Object::Pointer possibleGUI = tool->GetGUI("Qmitk", "GUI");
    QmitkToolGUI* gui = dynamic_cast<QmitkToolGUI*>( possibleGUI.GetPointer() );
    if (gui)
    {
      gui->SetTool(tool);
      gui->setParent(m_ThresholdingDialog);
      layout->addWidget(gui);
      m_ThresholdingDialog->setLayout(layout);
      layout->activate();
      m_ThresholdingDialog->setFixedSize(300,50);
      m_ThresholdingDialog->open();
    }
  }

  for ( NodeList::iterator iter = selection.begin(); iter != selection.end(); ++iter )
  {
    mitk::DataNode* node = *iter;

    if (node)
    {
      m_ThresholdingToolManager->SetReferenceData( node );
      m_ThresholdingToolManager->ActivateTool( m_ThresholdingToolManager->GetToolIdByToolType<mitk::BinaryThresholdTool>() );
    }
  }
}

void QmitkSegmentationPostProcessing::ThresholdingDone(int)
{
  MITK_INFO << "Thresholding done, cleaning up";
  m_ThresholdingDialog->deleteLater();
  m_ThresholdingDialog = NULL;
  m_ThresholdingToolManager->SetReferenceData( NULL );
  m_ThresholdingToolManager->SetWorkingData( NULL );

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSegmentationPostProcessing::OnThresholdingToolManagerToolModified()
{
  if ( m_ThresholdingToolManager.IsNull() ) return;

  //MITK_INFO << "Now got tool " << m_ThresholdingToolManager->GetActiveToolID();

  if ( m_ThresholdingToolManager->GetActiveToolID() < 0)
  {
    if (m_ThresholdingDialog)
      m_ThresholdingDialog->accept();
  }
}

void QmitkSegmentationPostProcessing::CreateSmoothedSurface(bool)
{
  InternalCreateSurface(true);
}

void QmitkSegmentationPostProcessing::CreateSurface(bool)
{
  InternalCreateSurface(false);
}

void QmitkSegmentationPostProcessing::InternalCreateSurface(bool smoothed)
{
  NodeList selection = this->GetSelectedNodes();

  for ( NodeList::iterator iter = selection.begin(); iter != selection.end(); ++iter )
  {
    mitk::DataNode* node = *iter;

    if (node)
    {
      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );
      if (image.IsNull()) return;
        
      try
      {
        mitk::ShowSegmentationAsSurface::Pointer surfaceFilter = mitk::ShowSegmentationAsSurface::New();

        // attach observer to get notified about result
        itk::SimpleMemberCommand<QmitkSegmentationPostProcessing>::Pointer goodCommand = itk::SimpleMemberCommand<QmitkSegmentationPostProcessing>::New();
        goodCommand->SetCallbackFunction(this, &QmitkSegmentationPostProcessing::OnSurfaceCalculationDone);
        surfaceFilter->AddObserver(mitk::ResultAvailable(), goodCommand);
        itk::SimpleMemberCommand<QmitkSegmentationPostProcessing>::Pointer badCommand = itk::SimpleMemberCommand<QmitkSegmentationPostProcessing>::New();
        badCommand->SetCallbackFunction(this, &QmitkSegmentationPostProcessing::OnSurfaceCalculationDone);
        surfaceFilter->AddObserver(mitk::ProcessingError(), badCommand);

        mitk::DataNode::Pointer nodepointer = node;
        surfaceFilter->SetPointerParameter("Input", image);
        surfaceFilter->SetPointerParameter("Group node", nodepointer);
        surfaceFilter->SetParameter("Show result", true );
        surfaceFilter->SetParameter("Sync visibility", false );
        surfaceFilter->SetDataStorage( *m_DataStorage );

        if (smoothed)
        {
          surfaceFilter->SetParameter("Smooth", true );
          //surfaceFilter->SetParameter("Apply median", true );
          surfaceFilter->SetParameter("Apply median", false );  // median makes the resulting surfaces look like lego models
          surfaceFilter->SetParameter("Median kernel size", 3u );
          surfaceFilter->SetParameter("Gaussian SD", 2.5f );
          surfaceFilter->SetParameter("Decimate mesh", true );
          surfaceFilter->SetParameter("Decimation rate", 0.80f );
        }
        else
        {
          surfaceFilter->SetParameter("Smooth", false );
          surfaceFilter->SetParameter("Apply median", false );
          surfaceFilter->SetParameter("Median kernel size", 3u );
          surfaceFilter->SetParameter("Gaussian SD", 1.5f );
          surfaceFilter->SetParameter("Decimate mesh", true );
          surfaceFilter->SetParameter("Decimation rate", 0.8f );
        }
        
        mitk::ProgressBar::GetInstance()->AddStepsToDo(10);
        mitk::ProgressBar::GetInstance()->Progress(2);
        mitk::StatusBar::GetInstance()->DisplayText("Surface creation started in background...");
        surfaceFilter->StartAlgorithm();
      }
      catch(...)
      {
        MITK_ERROR << "surface creation filter had an error";
      }
    }
    else
    {
      MITK_INFO << "   a NULL node selected";
    }
  }
}

void QmitkSegmentationPostProcessing::OnSurfaceCalculationDone()
{
  mitk::ProgressBar::GetInstance()->Progress(8);
}


void QmitkSegmentationPostProcessing::ImageStatistics(bool)
{
  if (m_BlueBerryView)
  {
    m_BlueBerryView->GetSite()->GetWorkbenchWindow()->GetActivePage()->ShowView("org.mitk.views.imagestatistics");
  }
}

void QmitkSegmentationPostProcessing::AutocropSelected(bool)
{
  NodeList selection = this->GetSelectedNodes();

  for ( NodeList::iterator iter = selection.begin(); iter != selection.end(); ++iter )
  {
    mitk::DataNode* node = *iter;

    if (node)
    {
      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );
      if (image.IsNull()) return;

      mitk::ProgressBar::GetInstance()->AddStepsToDo(10);
      mitk::ProgressBar::GetInstance()->Progress(2);

      qApp->processEvents();

      mitk::AutoCropImageFilter::Pointer cropFilter = mitk::AutoCropImageFilter::New();
      cropFilter->SetInput( image );
      cropFilter->SetBackgroundValue( 0 );
      try
      {
        cropFilter->Update();

        image = cropFilter->GetOutput();
        
        if (image.IsNotNull())
        {
          node->SetData( this->IncreaseCroppedImageSize(image) ); // bug fix 3145
        }
      }
      catch(...)
      {
        MITK_ERROR << "Cropping image failed...";
      }
      mitk::ProgressBar::GetInstance()->Progress(8);
    }
    else
    {
      MITK_INFO << "   a NULL node selected";
    }
  }
}

mitk::Image::Pointer QmitkSegmentationPostProcessing::IncreaseCroppedImageSize( mitk::Image::Pointer image )
{
  typedef itk::Image< short, 3 > ImageType;
  typedef itk::Image< unsigned char, 3 > PADOutputImageType;
  ImageType::Pointer itkTransformImage = ImageType::New();
  mitk::CastToItkImage( image, itkTransformImage );

  typedef itk::ConstantPadImageFilter< ImageType, PADOutputImageType > PadFilterType;
  PadFilterType::Pointer padFilter = PadFilterType::New();

  unsigned long upperPad[3];
  unsigned long lowerPad[3];
  int borderLiner = 6;

  mitk::Point3D mitkOriginPoint;
  double origin[3];
  origin[0]=0;
  origin[1]=0;
  origin[2]=0;
  itkTransformImage->SetOrigin(origin);

  lowerPad[0]=borderLiner/2;
  lowerPad[1]=borderLiner/2;
  lowerPad[2]=borderLiner/2;

  upperPad[0]=borderLiner/2;
  upperPad[1]=borderLiner/2;
  upperPad[2]=borderLiner/2;

  padFilter->SetInput(itkTransformImage);
  padFilter->SetConstant(0);
  padFilter->SetPadUpperBound(upperPad);
  padFilter->SetPadLowerBound(lowerPad);
  padFilter->UpdateLargestPossibleRegion();


  mitk::Image::Pointer paddedImage = mitk::Image::New();
  mitk::CastToMitkImage(padFilter->GetOutput(), paddedImage);

  paddedImage->SetGeometry(image->GetGeometry());
  
  //calculate translation vector according to padding to get the new origin
  mitk::Vector3D transVector = image->GetGeometry()->GetSpacing();
  transVector[0] = -(borderLiner/2);
  transVector[1] = -(borderLiner/2);
  transVector[2] = -(borderLiner/2);

  mitk::Vector3D newTransVectorInmm = image->GetGeometry()->GetSpacing();

  image->GetGeometry()->IndexToWorld(mitkOriginPoint, transVector, newTransVectorInmm);
  paddedImage->GetGeometry()->Translate(newTransVectorInmm);
  //paddedImage->SetRequestedRegionToLargestPossibleRegion();

  return paddedImage;
}

