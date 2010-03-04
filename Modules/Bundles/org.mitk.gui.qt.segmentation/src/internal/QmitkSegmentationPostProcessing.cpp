
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
#include "mitkDataTreeNodeObject.h"

#include <QtGui>

#include <berryIWorkbenchPage.h>

#include <itkConstantPadImageFilter.h>

QmitkSegmentationPostProcessing::QmitkSegmentationPostProcessing(mitk::DataStorage* storage, QmitkFunctionality* functionality, QObject* parent)
:QObject(parent)
,m_BlueBerryView(functionality)
,m_DataStorage(storage)
{
  // register a couple of additional actions for DataManager's context menu
  QmitkNodeDescriptor* imageDataTreeNodeDescriptor = 
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("Image");

  if (imageDataTreeNodeDescriptor)
  {
    m_ThresholdAction = new QAction("Threshold..", parent);
    imageDataTreeNodeDescriptor->AddAction(m_ThresholdAction);
    connect( m_ThresholdAction, SIGNAL( triggered(bool) ) , this, SLOT( ThresholdImage(bool) ) );
  }
  else
  {
    MITK_WARN << "Could not get datamanager's node descriptor for 'Image'";
  }

  // register a couple of additional actions for DataManager's context menu
  QmitkNodeDescriptor* binaryImageDataTreeNodeDescriptor = 
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("ImageMask");

  if (binaryImageDataTreeNodeDescriptor)
  {
    m_CreateSurfaceAction = new QAction("Create polygon model", parent);
    binaryImageDataTreeNodeDescriptor->AddAction(m_CreateSurfaceAction);
    connect( m_CreateSurfaceAction, SIGNAL( triggered(bool) ) , this, SLOT( CreateSurface(bool) ) );

    m_CreateSmoothSurfaceAction = new QAction("Create smoothed polygon model", parent);
    binaryImageDataTreeNodeDescriptor->AddAction(m_CreateSmoothSurfaceAction);
    connect( m_CreateSmoothSurfaceAction, SIGNAL( triggered(bool) ) , this, SLOT( CreateSmoothedSurface(bool) ) );

    m_StatisticsAction = new QAction("Statistics", parent);
    binaryImageDataTreeNodeDescriptor->AddAction(m_StatisticsAction);
    connect( m_StatisticsAction, SIGNAL( triggered(bool) ) , this, SLOT( ImageStatistics(bool) ) );
   
    m_AutocropAction = new QAction("Autocrop", parent);
    binaryImageDataTreeNodeDescriptor->AddAction(m_AutocropAction);
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
  QmitkNodeDescriptor* imageDataTreeNodeDescriptor = 
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("Image");

  if (imageDataTreeNodeDescriptor)
  {
    imageDataTreeNodeDescriptor->RemoveAction( m_ThresholdAction );
  }
  else
  {
    MITK_WARN << "Could not get datamanager's node descriptor for 'Image'";
  }

  // unregister a couple of additional actions for DataManager's context menu
  QmitkNodeDescriptor* binaryImageDataTreeNodeDescriptor = 
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("ImageMask");

  if (binaryImageDataTreeNodeDescriptor)
  {
    binaryImageDataTreeNodeDescriptor->RemoveAction( m_CreateSurfaceAction );
    binaryImageDataTreeNodeDescriptor->RemoveAction( m_CreateSmoothSurfaceAction );
    binaryImageDataTreeNodeDescriptor->RemoveAction( m_StatisticsAction );
    binaryImageDataTreeNodeDescriptor->RemoveAction( m_AutocropAction );
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
  m_CurrentSelection = selection.Cast<const mitk::DataTreeNodeSelection>();
}

QmitkSegmentationPostProcessing::NodeList QmitkSegmentationPostProcessing::GetSelectedNodes() const
{
  NodeList result;
  if (m_CurrentSelection)
  {

    // iterate selection
    for (mitk::DataTreeNodeSelection::iterator i = m_CurrentSelection->Begin(); i != m_CurrentSelection->End(); ++i)
    {
      // extract datatree node
      if (mitk::DataTreeNodeObject::Pointer nodeObj = i->Cast<mitk::DataTreeNodeObject>())
      {
        mitk::DataTreeNode::Pointer node = nodeObj->GetDataTreeNode();
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
      m_ThresholdingDialog->setFixedSize(m_ThresholdingDialog->width()+100, m_ThresholdingDialog->height());
      m_ThresholdingDialog->open();
    }
  }

  for ( NodeList::iterator iter = selection.begin(); iter != selection.end(); ++iter )
  {
    mitk::DataTreeNode* node = *iter;

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
  m_ThresholdingToolManager = NULL;

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSegmentationPostProcessing::OnThresholdingToolManagerToolModified()
{
  if ( m_ThresholdingToolManager.IsNull() ) return;

  //MITK_INFO << "Now got tool " << m_ThresholdingToolManager->GetActiveToolID();

  if ( m_ThresholdingToolManager->GetActiveToolID() < 0)
  {
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
    mitk::DataTreeNode* node = *iter;

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

        mitk::DataTreeNode::Pointer nodepointer = node;
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
    mitk::DataTreeNode* node = *iter;

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


  mitk::Image::Pointer segmentationImage = mitk::Image::New();
  mitk::CastToMitkImage(padFilter->GetOutput(), segmentationImage);

  segmentationImage->SetGeometry(image->GetGeometry());

  return segmentationImage;
}


