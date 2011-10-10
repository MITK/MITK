/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-22 11:00:35 +0200 (Fr, 22 Mai 2009) $
Version:   $Revision: 10185 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkPartialVolumeAnalysisView.h"

#include <limits>

#include <qlabel.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qclipboard.h>
#include <qfiledialog.h>

#include <berryIEditorPart.h>
#include <berryIWorkbenchPage.h>
#include <berryPlatform.h>


#include "QmitkStdMultiWidget.h"
#include "QmitkSliderNavigatorWidget.h"

#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateOr.h"
#include "mitkImageTimeSelector.h"
#include "mitkProperties.h"
#include "mitkProgressBar.h"

// Includes for image processing
#include "mitkImageCast.h"
#include "mitkImageToItk.h"
#include "mitkITKImageImport.h"
#include "mitkDataNodeObject.h"
#include "mitkNodePredicateData.h"

#include "mitkPlanarFigureInteractor.h"
#include "mitkGlobalInteraction.h"
#include "mitkTensorImage.h"

#include "mitkPlanarCircle.h"
#include "mitkPlanarRectangle.h"
#include "mitkPlanarPolygon.h"

#include "mitkPartialVolumeAnalysisClusteringCalculator.h"

#include <itkVectorImage.h>
#include "itkTensorDerivedMeasurementsFilter.h"
#include "itkDiffusionTensor3D.h"
#include "itkCartesianToPolarVectorImageFilter.h"
#include "itkPolarToCartesianVectorImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkMaskImageFilter.h"
#include "itkCastImageFilter.h"

#include "itkImageMomentsCalculator.h"

#include <vnl/vnl_vector.h>

#define PVA_PI 3.141592653589793238462643383

const std::string QmitkPartialVolumeAnalysisView::VIEW_ID =
"org.mitk.views.partialvolumeanalysisview";

class QmitkRequestStatisticsUpdateEvent : public QEvent
{
public:
  enum Type
  {
    StatisticsUpdateRequest = QEvent::MaxUser - 1025
                            };

  QmitkRequestStatisticsUpdateEvent()
    : QEvent( (QEvent::Type) StatisticsUpdateRequest ) {};
};



typedef itk::Image<short, 3>                 ImageType;
typedef itk::Image<float, 3>                 FloatImageType;
typedef itk::Image<itk::Vector<float,3>, 3>  VectorImageType;

inline bool my_isnan(float x)
{
  volatile float d = x;

  if(d!=d)
    return true;

  if(d==d)
    return false;
  return d != d;

}

QmitkPartialVolumeAnalysisView::QmitkPartialVolumeAnalysisView(QObject */*parent*/, const char */*name*/)
  : QmitkFunctionality(),
  m_Controls( NULL ),
  m_TimeStepperAdapter( NULL ),
  m_MeasurementInfoRenderer(0),
  m_MeasurementInfoAnnotation(0),
  m_SelectedImageNodes(  ),
  m_SelectedImage( NULL ),
  m_SelectedMaskNode( NULL ),
  m_SelectedImageMask( NULL ),
  m_SelectedPlanarFigureNodes(0),
  m_SelectedPlanarFigure( NULL ),
  m_IsTensorImage(false),
  m_FAImage(0),
  m_RDImage(0),
  m_ADImage(0),
  m_MDImage(0),
  m_CAImage(0),
  //  m_DirectionImage(0),
  m_DirectionComp1Image(0),
  m_DirectionComp2Image(0),
  m_AngularErrorImage(0),
  m_SelectedRenderWindow(NULL),
  m_LastRenderWindow(NULL),
  m_ImageObserverTag( -1 ),
  m_ImageMaskObserverTag( -1 ),
  m_PlanarFigureObserverTag( -1 ),
  m_CurrentStatisticsValid( false ),
  m_StatisticsUpdatePending( false ),
  m_GaussianSigmaChangedSliding(false),
  m_NumberBinsSliding(false),
  m_UpsamplingChangedSliding(false),
  m_ClusteringResult(NULL),
  m_EllipseCounter(0),
  m_RectangleCounter(0),
  m_PolygonCounter(0),
  m_CurrentFigureNodeInitialized(false),
  m_QuantifyClass(2),
  m_IconTexOFF(new QIcon(":/QmitkPartialVolumeAnalysisView/texIntOFFIcon.png")),
  m_IconTexON(new QIcon(":/QmitkPartialVolumeAnalysisView/texIntONIcon.png")),
  m_TexIsOn(true)
{

}


QmitkPartialVolumeAnalysisView::~QmitkPartialVolumeAnalysisView()
{
  if ( m_SelectedImage.IsNotNull() )
    m_SelectedImage->RemoveObserver( m_ImageObserverTag );
  if ( m_SelectedImageMask.IsNotNull() )
    m_SelectedImageMask->RemoveObserver( m_ImageMaskObserverTag );
  if ( m_SelectedPlanarFigure.IsNotNull() )
  {
    m_SelectedPlanarFigure->RemoveObserver( m_PlanarFigureObserverTag );
    m_SelectedPlanarFigure->RemoveObserver( m_InitializedObserverTag );
  }

  this->GetDefaultDataStorage()->AddNodeEvent -= mitk::MessageDelegate1<QmitkPartialVolumeAnalysisView
                                                 , const mitk::DataNode*>( this, &QmitkPartialVolumeAnalysisView::NodeAddedInDataStorage );

  m_SelectedPlanarFigureNodes->NodeChanged.RemoveListener( mitk::MessageDelegate1<QmitkPartialVolumeAnalysisView
                                                           , const mitk::DataNode*>( this, &QmitkPartialVolumeAnalysisView::NodeChanged ) );

  m_SelectedPlanarFigureNodes->NodeRemoved.RemoveListener( mitk::MessageDelegate1<QmitkPartialVolumeAnalysisView
                                                           , const mitk::DataNode*>( this, &QmitkPartialVolumeAnalysisView::NodeRemoved ) );

  m_SelectedPlanarFigureNodes->PropertyChanged.RemoveListener( mitk::MessageDelegate2<QmitkPartialVolumeAnalysisView
                                                               , const mitk::DataNode*, const mitk::BaseProperty*>( this, &QmitkPartialVolumeAnalysisView::PropertyChanged ) );

  m_SelectedImageNodes->NodeChanged.RemoveListener( mitk::MessageDelegate1<QmitkPartialVolumeAnalysisView
                                                    , const mitk::DataNode*>( this, &QmitkPartialVolumeAnalysisView::NodeChanged ) );

  m_SelectedImageNodes->NodeRemoved.RemoveListener( mitk::MessageDelegate1<QmitkPartialVolumeAnalysisView
                                                    , const mitk::DataNode*>( this, &QmitkPartialVolumeAnalysisView::NodeRemoved ) );

  m_SelectedImageNodes->PropertyChanged.RemoveListener( mitk::MessageDelegate2<QmitkPartialVolumeAnalysisView
                                                        , const mitk::DataNode*, const mitk::BaseProperty*>( this, &QmitkPartialVolumeAnalysisView::PropertyChanged ) );
}


void QmitkPartialVolumeAnalysisView::CreateQtPartControl(QWidget *parent)
{
  if (m_Controls == NULL)
  {
    m_Controls = new Ui::QmitkPartialVolumeAnalysisViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    m_Controls->m_ErrorMessageLabel->hide();
  }

  SetHistogramVisibility();

  m_Controls->m_TextureIntON->setIcon(*m_IconTexON);

  m_Controls->m_SimilarAnglesFrame->setVisible(false);
  m_Controls->m_SimilarAnglesLabel->setVisible(false);

  vtkTextProperty *textProp = vtkTextProperty::New();
  textProp->SetColor(1.0, 1.0, 1.0);

  m_MeasurementInfoAnnotation = vtkCornerAnnotation::New();
  m_MeasurementInfoAnnotation->SetMaximumFontSize(12);
  m_MeasurementInfoAnnotation->SetTextProperty(textProp);

  m_MeasurementInfoRenderer = vtkRenderer::New();
  m_MeasurementInfoRenderer->AddActor(m_MeasurementInfoAnnotation);

  m_SelectedPlanarFigureNodes = mitk::DataStorageSelection::New(this->GetDefaultDataStorage(), false);

  m_SelectedPlanarFigureNodes->NodeChanged.AddListener( mitk::MessageDelegate1<QmitkPartialVolumeAnalysisView
                                                        , const mitk::DataNode*>( this, &QmitkPartialVolumeAnalysisView::NodeChanged ) );

  m_SelectedPlanarFigureNodes->NodeRemoved.AddListener( mitk::MessageDelegate1<QmitkPartialVolumeAnalysisView
                                                        , const mitk::DataNode*>( this, &QmitkPartialVolumeAnalysisView::NodeRemoved ) );

  m_SelectedPlanarFigureNodes->PropertyChanged.AddListener( mitk::MessageDelegate2<QmitkPartialVolumeAnalysisView
                                                            , const mitk::DataNode*, const mitk::BaseProperty*>( this, &QmitkPartialVolumeAnalysisView::PropertyChanged ) );

  m_SelectedImageNodes = mitk::DataStorageSelection::New(this->GetDefaultDataStorage(), false);

  m_SelectedImageNodes->PropertyChanged.AddListener( mitk::MessageDelegate2<QmitkPartialVolumeAnalysisView
                                                     , const mitk::DataNode*, const mitk::BaseProperty*>( this, &QmitkPartialVolumeAnalysisView::PropertyChanged ) );

  m_SelectedImageNodes->NodeChanged.AddListener( mitk::MessageDelegate1<QmitkPartialVolumeAnalysisView
                                                 , const mitk::DataNode*>( this, &QmitkPartialVolumeAnalysisView::NodeChanged ) );

  m_SelectedImageNodes->NodeRemoved.AddListener( mitk::MessageDelegate1<QmitkPartialVolumeAnalysisView
                                                 , const mitk::DataNode*>( this, &QmitkPartialVolumeAnalysisView::NodeRemoved ) );

  this->GetDefaultDataStorage()->AddNodeEvent.AddListener( mitk::MessageDelegate1<QmitkPartialVolumeAnalysisView
                                                           , const mitk::DataNode*>( this, &QmitkPartialVolumeAnalysisView::NodeAddedInDataStorage ) );

  Select(NULL,true,true);

  SetAdvancedVisibility();
}

void QmitkPartialVolumeAnalysisView::SetHistogramVisibility()
{
  m_Controls->m_HistogramWidget->setVisible(m_Controls->m_DisplayHistogramCheckbox->isChecked());
}

void QmitkPartialVolumeAnalysisView::SetAdvancedVisibility()
{
  m_Controls->frame_7->setVisible(m_Controls->m_AdvancedCheckbox->isChecked());
}

void QmitkPartialVolumeAnalysisView::CreateConnections()
{
  if ( m_Controls )
  {

    connect( m_Controls->m_DisplayHistogramCheckbox, SIGNAL( clicked() )
             , this, SLOT( SetHistogramVisibility() ) );

    connect( m_Controls->m_AdvancedCheckbox, SIGNAL( clicked() )
             , this, SLOT( SetAdvancedVisibility() ) );


    connect( m_Controls->m_NumberBinsSlider, SIGNAL( sliderReleased () ),
             this, SLOT( NumberBinsReleasedSlider( ) ) );
    connect( m_Controls->m_UpsamplingSlider, SIGNAL( sliderReleased(  ) ),
             this, SLOT( UpsamplingReleasedSlider( ) ) );
    connect( m_Controls->m_GaussianSigmaSlider, SIGNAL( sliderReleased(  ) ),
             this, SLOT( GaussianSigmaReleasedSlider(  ) ) );
    connect( m_Controls->m_SimilarAnglesSlider, SIGNAL( sliderReleased(  ) ),
             this, SLOT( SimilarAnglesReleasedSlider(  ) ) );

    connect( m_Controls->m_NumberBinsSlider, SIGNAL( valueChanged (int) ),
             this, SLOT( NumberBinsChangedSlider( int ) ) );
    connect( m_Controls->m_UpsamplingSlider, SIGNAL( valueChanged( int ) ),
             this, SLOT( UpsamplingChangedSlider( int ) ) );
    connect( m_Controls->m_GaussianSigmaSlider, SIGNAL( valueChanged( int ) ),
             this, SLOT( GaussianSigmaChangedSlider( int ) ) );
    connect( m_Controls->m_SimilarAnglesSlider, SIGNAL( valueChanged( int ) ),
             this, SLOT( SimilarAnglesChangedSlider(int) ) );

    connect( m_Controls->m_OpacitySlider, SIGNAL( valueChanged( int ) ),
             this, SLOT( OpacityChangedSlider(int) ) );

    connect( (QObject*)(m_Controls->m_ButtonCopyHistogramToClipboard), SIGNAL(clicked()),(QObject*) this, SLOT(ToClipBoard()));

    connect( m_Controls->m_CircleButton, SIGNAL( clicked() )
             , this, SLOT( ActionDrawEllipseTriggered() ) );

    connect( m_Controls->m_RectangleButton, SIGNAL( clicked() )
             , this, SLOT( ActionDrawRectangleTriggered() ) );

    connect( m_Controls->m_PolygonButton, SIGNAL( clicked() )
             , this, SLOT( ActionDrawPolygonTriggered() ) );

    connect( m_Controls->m_GreenRadio, SIGNAL( clicked(bool) )
             , this, SLOT( GreenRadio(bool) ) );

    connect( m_Controls->m_PartialVolumeRadio, SIGNAL( clicked(bool) )
             , this, SLOT( PartialVolumeRadio(bool) ) );

    connect( m_Controls->m_BlueRadio, SIGNAL( clicked(bool) )
             , this, SLOT( BlueRadio(bool) ) );

    connect( m_Controls->m_AllRadio, SIGNAL( clicked(bool) )
             , this, SLOT( AllRadio(bool) ) );

    connect( m_Controls->m_EstimateCircle, SIGNAL( clicked() )
             , this, SLOT( EstimateCircle() ) );

    connect( (QObject*)(m_Controls->m_TextureIntON), SIGNAL(clicked()), this, SLOT(TextIntON()) );

  }
}

void QmitkPartialVolumeAnalysisView::EstimateCircle()
{
  typedef itk::Image<unsigned char, 3> SegImageType;
  SegImageType::Pointer mask_itk = SegImageType::New();

  typedef mitk::ImageToItk<SegImageType> CastType;
  CastType::Pointer caster = CastType::New();
  caster->SetInput(m_SelectedImageMask);
  caster->Update();

  typedef itk::ImageMomentsCalculator< SegImageType > MomentsType;
  MomentsType::Pointer momentsCalc = MomentsType::New();
  momentsCalc->SetImage(caster->GetOutput());
  momentsCalc->Compute();
  MomentsType::VectorType cog = momentsCalc->GetCenterOfGravity();
  MomentsType::MatrixType axes = momentsCalc->GetPrincipalAxes();
  MomentsType::VectorType moments = momentsCalc->GetPrincipalMoments();

  // moments-coord conversion
  // third coordinate min oder max?

  // max-min = extent

  MomentsType::AffineTransformPointer trafo = momentsCalc->GetPhysicalAxesToPrincipalAxesTransform();

  itk::ImageRegionIterator<SegImageType>
      itimage(caster->GetOutput(), caster->GetOutput()->GetLargestPossibleRegion());
  itimage = itimage.Begin();

  double max = -9999999999.0;
  double min =  9999999999.0;

  while( !itimage.IsAtEnd() )
  {
    if(itimage.Get())
    {
      ImageType::IndexType index = itimage.GetIndex();

      itk::Point<float,3> point;
      caster->GetOutput()->TransformIndexToPhysicalPoint(index,point);

      itk::Point<float,3> newPoint;
      newPoint = trafo->TransformPoint(point);

      if(newPoint[2]<min)
        min = newPoint[2];

      if(newPoint[2]>max)
        max = newPoint[2];
    }
    ++itimage;
  }

  double extent = max - min;
  MITK_INFO << "EXTENT = " << extent;

  mitk::Point3D origin;
  mitk::Vector3D right, bottom, normal;

  double factor = 1000.0;
  mitk::FillVector3D(origin, cog[0]-factor*axes[1][0]-factor*axes[2][0],
                     cog[1]-factor*axes[1][1]-factor*axes[2][1],
                     cog[2]-factor*axes[1][2]-factor*axes[2][2]);
  //          mitk::FillVector3D(normal, axis[0][0],axis[0][1],axis[0][2]);
  mitk::FillVector3D(bottom, 2*factor*axes[1][0], 2*factor*axes[1][1], 2*factor*axes[1][2]);
  mitk::FillVector3D(right,  2*factor*axes[2][0], 2*factor*axes[2][1], 2*factor*axes[2][2]);

  mitk::PlaneGeometry::Pointer planegeometry = mitk::PlaneGeometry::New();
  planegeometry->InitializeStandardPlane(right.Get_vnl_vector(), bottom.Get_vnl_vector());
  planegeometry->SetOrigin(origin);

  double len1 = sqrt(axes[1][0]*axes[1][0] + axes[1][1]*axes[1][1] + axes[1][2]*axes[1][2]);
  double len2 = sqrt(axes[2][0]*axes[2][0] + axes[2][1]*axes[2][1] + axes[2][2]*axes[2][2]);

  mitk::Point2D point1;
  point1[0] = factor*len1;
  point1[1] = factor*len2;

  mitk::Point2D point2;
  point2[0] = factor*len1+extent*.5;
  point2[1] = factor*len2;

  mitk::PlanarCircle::Pointer circle = mitk::PlanarCircle::New();
  circle->SetGeometry2D(planegeometry);
  circle->PlaceFigure( point1 );
  circle->SetControlPoint(0,point1);
  circle->SetControlPoint(1,point2);
  //circle->SetCurrentControlPoint( point2 );

  mitk::PlanarFigure::PolyLineType polyline = circle->GetPolyLine( 0 );
  MITK_INFO << "SIZE of planar figure polyline: " << polyline.size();

  AddFigureToDataStorage(circle, "Circle");

}

void QmitkPartialVolumeAnalysisView::StdMultiWidgetAvailable( QmitkStdMultiWidget& stdMultiWidget )
{
  QmitkFunctionality::StdMultiWidgetAvailable(stdMultiWidget);
}

bool QmitkPartialVolumeAnalysisView::AssertDrawingIsPossible(bool checked)
{
  if (m_SelectedImageNodes->GetNode().IsNull())
  {
    checked = false;
    this->HandleException("Please select an image!", this->m_Parent, true);
    return false;
  }

  //this->GetActiveStdMultiWidget()->SetWidgetPlanesVisibility(false);

  return checked;
}

void QmitkPartialVolumeAnalysisView::ActionDrawEllipseTriggered()
{
  bool checked = m_Controls->m_CircleButton->isChecked();
  if(!this->AssertDrawingIsPossible(checked))
    return;

  mitk::PlanarCircle::Pointer figure = mitk::PlanarCircle::New();
  this->AddFigureToDataStorage(figure, QString("Circle%1").arg(++m_EllipseCounter));

  MITK_INFO << "PlanarCircle created ...";
}

void QmitkPartialVolumeAnalysisView::ActionDrawRectangleTriggered()
{
  bool checked = m_Controls->m_RectangleButton->isChecked();
  if(!this->AssertDrawingIsPossible(checked))
    return;

  mitk::PlanarRectangle::Pointer figure = mitk::PlanarRectangle::New();
  this->AddFigureToDataStorage(figure, QString("Rectangle%1").arg(++m_RectangleCounter));

  MITK_INFO << "PlanarRectangle created ...";
}

void QmitkPartialVolumeAnalysisView::ActionDrawPolygonTriggered()
{
  bool checked = m_Controls->m_PolygonButton->isChecked();
  if(!this->AssertDrawingIsPossible(checked))
    return;

  mitk::PlanarPolygon::Pointer figure = mitk::PlanarPolygon::New();
  figure->ClosedOn();
  this->AddFigureToDataStorage(figure, QString("Polygon%1").arg(++m_PolygonCounter));

  MITK_INFO << "PlanarPolygon created ...";
}

void QmitkPartialVolumeAnalysisView::AddFigureToDataStorage(mitk::PlanarFigure* figure, const QString& name,
                                                        const char *propertyKey, mitk::BaseProperty *property )
{

  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
  newNode->SetName(name.toStdString());
  newNode->SetData(figure);

  // Add custom property, if available
  if ( (propertyKey != NULL) && (property != NULL) )
  {
    newNode->AddProperty( propertyKey, property );
  }

  // figure drawn on the topmost layer / image
  this->GetDataStorage()->Add(newNode, m_SelectedImageNodes->GetNode() );

  std::vector<mitk::DataNode*> selectedNodes = GetDataManagerSelection();
  for(unsigned int i = 0; i < selectedNodes.size(); i++)
  {
    selectedNodes[i]->SetSelected(false);
  }
  selectedNodes = m_SelectedPlanarFigureNodes->GetNodes();
  for(unsigned int i = 0; i < selectedNodes.size(); i++)
  {
    selectedNodes[i]->SetSelected(false);
  }
  newNode->SetSelected(true);

  Select(newNode);
}

void QmitkPartialVolumeAnalysisView::PlanarFigureInitialized()
{
  if(m_SelectedPlanarFigureNodes->GetNode().IsNull())
    return;

  m_CurrentFigureNodeInitialized = true;

  this->Select(m_SelectedPlanarFigureNodes->GetNode());

  m_Controls->m_CircleButton->setChecked(false);
  m_Controls->m_RectangleButton->setChecked(false);
  m_Controls->m_PolygonButton->setChecked(false);

  //this->GetActiveStdMultiWidget()->SetWidgetPlanesVisibility(true);
  this->RequestStatisticsUpdate();

}

void QmitkPartialVolumeAnalysisView::PlanarFigureFocus(mitk::DataNode* node)
{
  mitk::PlanarFigure* _PlanarFigure = 0;
  _PlanarFigure = dynamic_cast<mitk::PlanarFigure*> (node->GetData());

  if (_PlanarFigure)
  {

    FindRenderWindow(node);

    const mitk::PlaneGeometry
        * _PlaneGeometry =
        dynamic_cast<const mitk::PlaneGeometry*> (_PlanarFigure->GetGeometry2D());

    // make node visible
    if (m_SelectedRenderWindow)
    {
      mitk::Point3D centerP = _PlaneGeometry->GetOrigin();
      m_SelectedRenderWindow->GetSliceNavigationController()->ReorientSlices(
          centerP, _PlaneGeometry->GetNormal());
      m_SelectedRenderWindow->GetSliceNavigationController()->SelectSliceByPoint(
          centerP);
    }
  }
}

void QmitkPartialVolumeAnalysisView::FindRenderWindow(mitk::DataNode* node)
{

  if(node)
  {
    mitk::PlanarFigure* _PlanarFigure = 0;
    _PlanarFigure = dynamic_cast<mitk::PlanarFigure*> (node->GetData());

    if (_PlanarFigure)
    {
      m_SelectedRenderWindow = 0;
      QmitkRenderWindow* RenderWindow1 =
          this->GetActiveStdMultiWidget()->GetRenderWindow1();
      QmitkRenderWindow* RenderWindow2 =
          this->GetActiveStdMultiWidget()->GetRenderWindow2();
      QmitkRenderWindow* RenderWindow3 =
          this->GetActiveStdMultiWidget()->GetRenderWindow3();
      QmitkRenderWindow* RenderWindow4 =
          this->GetActiveStdMultiWidget()->GetRenderWindow4();

      bool PlanarFigureInitializedWindow = false;

      // find initialized renderwindow
      if (node->GetBoolProperty("PlanarFigureInitializedWindow",
                                PlanarFigureInitializedWindow, RenderWindow1->GetRenderer()))
      {
        m_SelectedRenderWindow = RenderWindow1;
      }

      if (!m_SelectedRenderWindow && node->GetBoolProperty(
          "PlanarFigureInitializedWindow", PlanarFigureInitializedWindow,
          RenderWindow2->GetRenderer()))
      {
        m_SelectedRenderWindow = RenderWindow2;
      }

      if (!m_SelectedRenderWindow && node->GetBoolProperty(
          "PlanarFigureInitializedWindow", PlanarFigureInitializedWindow,
          RenderWindow3->GetRenderer()))
      {
        m_SelectedRenderWindow = RenderWindow3;
      }

      if (!m_SelectedRenderWindow && node->GetBoolProperty(
          "PlanarFigureInitializedWindow", PlanarFigureInitializedWindow,
          RenderWindow4->GetRenderer()))
      {
        m_SelectedRenderWindow = RenderWindow4;
      }

    }
  }
}

void QmitkPartialVolumeAnalysisView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{

  if ( !this->IsVisible() )
  {
    return;
  }

  if ( nodes.empty() || nodes.size() > 1 )
  {
    // Nothing to do: invalidate image, clear statistics, histogram, and GUI
    return;
  }

  Select(nodes.front());

}

void QmitkPartialVolumeAnalysisView::Select( mitk::DataNode::Pointer node, bool clearMaskOnFirstArgNULL, bool clearImageOnFirstArgNULL )
{
  // Clear any unreferenced images
  this->RemoveOrphanImages();

  bool somethingChanged = false;
  if(node.IsNull())
  {
    somethingChanged = true;

    if(clearMaskOnFirstArgNULL)
    {
      if ( (m_SelectedImageMask.IsNotNull()) && (m_ImageMaskObserverTag >= 0) )
      {
        m_SelectedImageMask->RemoveObserver( m_ImageMaskObserverTag );
        m_ImageMaskObserverTag = -1;
      }

      if ( (m_SelectedPlanarFigure.IsNotNull()) && (m_PlanarFigureObserverTag >= 0) )
      {
        m_SelectedPlanarFigure->RemoveObserver( m_PlanarFigureObserverTag );
        m_PlanarFigureObserverTag = -1;
      }

      if ( (m_SelectedPlanarFigure.IsNotNull()) && (m_InitializedObserverTag >= 0) )
      {
        m_SelectedPlanarFigure->RemoveObserver( m_InitializedObserverTag );
        m_InitializedObserverTag = -1;
      }

      m_SelectedPlanarFigure = NULL;
      m_SelectedPlanarFigureNodes->RemoveAllNodes();
      m_CurrentFigureNodeInitialized = false;
      m_SelectedRenderWindow = 0;

      m_SelectedMaskNode = NULL;
      m_SelectedImageMask = NULL;

    }

    if(clearImageOnFirstArgNULL)
    {
      if ( (m_SelectedImage.IsNotNull()) && (m_ImageObserverTag >= 0) )
      {
        m_SelectedImage->RemoveObserver( m_ImageObserverTag );
        m_ImageObserverTag = -1;
      }

      m_SelectedImageNodes->RemoveAllNodes();
      m_SelectedImage = NULL;

      m_IsTensorImage = false;
      m_FAImage = NULL;
      m_RDImage = NULL;
      m_ADImage = NULL;
      m_MDImage = NULL;
      m_CAImage = NULL;
      m_DirectionComp1Image = NULL;
      m_DirectionComp2Image = NULL;
      m_AngularErrorImage = NULL;

      m_Controls->m_SimilarAnglesFrame->setVisible(false);
      m_Controls->m_SimilarAnglesLabel->setVisible(false);
    }
  }
  else
  {
    typedef itk::SimpleMemberCommand< QmitkPartialVolumeAnalysisView > ITKCommandType;
    ITKCommandType::Pointer changeListener;
    changeListener = ITKCommandType::New();
    changeListener->SetCallbackFunction( this, &QmitkPartialVolumeAnalysisView::RequestStatisticsUpdate );

    // Get selected element
    mitk::TensorImage *selectedTensorImage = dynamic_cast< mitk::TensorImage * >( node->GetData() );
    mitk::Image *selectedImage = dynamic_cast< mitk::Image * >( node->GetData() );
    mitk::PlanarFigure *selectedPlanar = dynamic_cast< mitk::PlanarFigure * >( node->GetData() );

    bool isMask = false;
    bool isImage = false;
    bool isPlanar = false;
    bool isTensorImage = false;

    if (selectedTensorImage != NULL)
    {
      isTensorImage = true;
    }
    else if(selectedImage != NULL)
    {
      node->GetPropertyValue("binary", isMask);
      isImage = !isMask;
    }
    else if ( (selectedPlanar != NULL) )
    {
      isPlanar = true;
    }

    // image
    if(isImage && selectedImage->GetDimension()==3)
    {
      if(selectedImage != m_SelectedImage.GetPointer())
      {
        somethingChanged = true;

        if ( (m_SelectedImage.IsNotNull()) && (m_ImageObserverTag >= 0) )
        {
          m_SelectedImage->RemoveObserver( m_ImageObserverTag );
          m_ImageObserverTag = -1;
        }

        *m_SelectedImageNodes = node;
        m_SelectedImage = selectedImage;
        m_IsTensorImage = false;
        m_FAImage = NULL;
        m_RDImage = NULL;
        m_ADImage = NULL;
        m_MDImage = NULL;
        m_CAImage = NULL;
        m_DirectionComp1Image = NULL;
        m_DirectionComp2Image = NULL;
        m_AngularErrorImage = NULL;

        // Add change listeners to selected objects
        m_ImageObserverTag = m_SelectedImage->AddObserver(
            itk::ModifiedEvent(), changeListener );

        m_Controls->m_SimilarAnglesFrame->setVisible(false);
        m_Controls->m_SimilarAnglesLabel->setVisible(false);

        m_Controls->m_SelectedImageLabel->setText( m_SelectedImageNodes->GetNode()->GetName().c_str() );
      }
    }

    //planar
    if(isPlanar)
    {
      if(selectedPlanar != m_SelectedPlanarFigure.GetPointer())
      {
        MITK_INFO << "Planar selection changed";
        somethingChanged = true;

        // Possibly previous change listeners
        if ( (m_SelectedPlanarFigure.IsNotNull()) && (m_PlanarFigureObserverTag >= 0) )
        {
          m_SelectedPlanarFigure->RemoveObserver( m_PlanarFigureObserverTag );
          m_PlanarFigureObserverTag = -1;
        }

        if ( (m_SelectedPlanarFigure.IsNotNull()) && (m_InitializedObserverTag >= 0) )
        {
          m_SelectedPlanarFigure->RemoveObserver( m_InitializedObserverTag );
          m_InitializedObserverTag = -1;
        }

        m_SelectedPlanarFigure = selectedPlanar;
        *m_SelectedPlanarFigureNodes = node;
        m_CurrentFigureNodeInitialized = selectedPlanar->IsPlaced();

        m_SelectedMaskNode = NULL;
        m_SelectedImageMask = NULL;

        m_PlanarFigureObserverTag = m_SelectedPlanarFigure->AddObserver(
            mitk::EndInteractionPlanarFigureEvent(), changeListener );

        if(!m_CurrentFigureNodeInitialized)
        {
          typedef itk::SimpleMemberCommand< QmitkPartialVolumeAnalysisView > ITKCommandType;
          ITKCommandType::Pointer initializationCommand;
          initializationCommand = ITKCommandType::New();

          // set the callback function of the member command
          initializationCommand->SetCallbackFunction( this, &QmitkPartialVolumeAnalysisView::PlanarFigureInitialized );

          // add an observer
          m_InitializedObserverTag = selectedPlanar->AddObserver( mitk::EndPlacementPlanarFigureEvent(), initializationCommand );

        }

        m_Controls->m_SelectedMaskLabel->setText( m_SelectedPlanarFigureNodes->GetNode()->GetName().c_str() );
        PlanarFigureFocus(node);
      }
    }

    //mask
    if(isMask && selectedImage->GetDimension()==3)
    {
      if(selectedImage != m_SelectedImage.GetPointer())
      {
        somethingChanged = true;

        if ( (m_SelectedImageMask.IsNotNull()) && (m_ImageMaskObserverTag >= 0) )
        {
          m_SelectedImageMask->RemoveObserver( m_ImageMaskObserverTag );
          m_ImageMaskObserverTag = -1;
        }

        m_SelectedMaskNode = node;
        m_SelectedImageMask = selectedImage;
        m_SelectedPlanarFigure = NULL;
        m_SelectedPlanarFigureNodes->RemoveAllNodes();

        m_ImageMaskObserverTag = m_SelectedImageMask->AddObserver(
            itk::ModifiedEvent(), changeListener );

        m_Controls->m_SelectedMaskLabel->setText( m_SelectedMaskNode->GetName().c_str() );

      }
    }

    //tensor image
    if(isTensorImage && selectedTensorImage->GetDimension()==3)
    {
      if(selectedImage != m_SelectedImage.GetPointer())
      {
        somethingChanged = true;

        if ( (m_SelectedImage.IsNotNull()) && (m_ImageObserverTag >= 0) )
        {
          m_SelectedImage->RemoveObserver( m_ImageObserverTag );
          m_ImageObserverTag = -1;
        }

        *m_SelectedImageNodes = node;
        m_SelectedImage = selectedImage;

        m_IsTensorImage = true;

        ExtractTensorImages(selectedImage);

        // Add change listeners to selected objects
        m_ImageObserverTag = m_SelectedImage->AddObserver(
            itk::ModifiedEvent(), changeListener );

        m_Controls->m_SimilarAnglesFrame->setVisible(true);
        m_Controls->m_SimilarAnglesLabel->setVisible(true);

        m_Controls->m_SelectedImageLabel->setText( m_SelectedImageNodes->GetNode()->GetName().c_str() );
      }
    }
  }

  if(somethingChanged)
  {
    this->SetMeasurementInfoToRenderWindow("");

    if(m_SelectedPlanarFigure.IsNull() && m_SelectedImageMask.IsNull() )
    {
      m_Controls->m_SelectedMaskLabel->setText( "None" );
      m_Controls->m_ResampleOptionsFrame->setEnabled(false);
      m_Controls->m_HistogramWidget->setEnabled(false);
      m_Controls->m_ClassSelector->setEnabled(false);
      m_Controls->m_DisplayHistogramCheckbox->setEnabled(false);
      m_Controls->m_AdvancedCheckbox->setEnabled(false);
      m_Controls->frame_7->setEnabled(false);
    }
    else
    {
      m_Controls->m_ResampleOptionsFrame->setEnabled(true);
      m_Controls->m_HistogramWidget->setEnabled(true);
      m_Controls->m_ClassSelector->setEnabled(true);
      m_Controls->m_DisplayHistogramCheckbox->setEnabled(true);
      m_Controls->m_AdvancedCheckbox->setEnabled(true);
      m_Controls->frame_7->setEnabled(true);
    }

    // Clear statistics / histogram GUI if nothing is selected
    if ( m_SelectedImage.IsNull() )
    {
      m_Controls->m_PlanarFigureButtonsFrame->setEnabled(false);
      m_Controls->m_OpacityFrame->setEnabled(false);
      m_Controls->m_SelectedImageLabel->setText( "None" );
    }
    else
    {
      m_Controls->m_PlanarFigureButtonsFrame->setEnabled(true);
      m_Controls->m_OpacityFrame->setEnabled(true);
    }

    if( m_SelectedImage.IsNull()
      || (m_SelectedPlanarFigure.IsNull() && m_SelectedImageMask.IsNull()) )
      {
      m_Controls->m_HistogramWidget->ClearItemModel();
      m_CurrentStatisticsValid = false;
      m_Controls->m_ErrorMessageLabel->hide();
    }
    else
    {
      this->RequestStatisticsUpdate();
    }
  }
}


void QmitkPartialVolumeAnalysisView::ShowClusteringResults()
{

  typedef itk::Image<unsigned char, 3> MaskImageType;
  mitk::Image::Pointer mask = 0;
  MaskImageType::Pointer itkmask = 0;

  if(m_IsTensorImage && m_Controls->m_SimilarAnglesSlider->value() != 0)
  {
    typedef itk::Image<float, 3> AngularErrorImageType;
    typedef mitk::ImageToItk<AngularErrorImageType> CastType;
    CastType::Pointer caster = CastType::New();
    caster->SetInput(m_AngularErrorImage);
    caster->Update();

    typedef itk::BinaryThresholdImageFilter< AngularErrorImageType, MaskImageType > ThreshType;
    ThreshType::Pointer thresh = ThreshType::New();
    thresh->SetUpperThreshold((90-m_Controls->m_SimilarAnglesSlider->value())*(PVA_PI/180.0));
    thresh->SetInsideValue(1.0);
    thresh->SetInput(caster->GetOutput());
    thresh->Update();
    itkmask = thresh->GetOutput();

    mask = mitk::Image::New();
    mask->InitializeByItk(itkmask.GetPointer());
    mask->SetVolume(itkmask->GetBufferPointer());

    //    GetDefaultDataStorage()->Remove(m_newnode);
    //    m_newnode = mitk::DataNode::New();
    //    m_newnode->SetData(mask);
    //    m_newnode->SetName("masking node");
    //    m_newnode->SetIntProperty( "layer", 1002 );
    //    GetDefaultDataStorage()->Add(m_newnode, m_SelectedImageNodes->GetNode());

  }

  mitk::Image::Pointer clusteredImage;
  ClusteringType::Pointer clusterer = ClusteringType::New();

  if(m_QuantifyClass==3)
  {

    if(m_IsTensorImage)
    {
      double *green_fa, *green_rd, *green_ad, *green_md;
      //double *greengray_fa, *greengray_rd, *greengray_ad, *greengray_md;
      double *gray_fa, *gray_rd, *gray_ad, *gray_md;
      //double *redgray_fa, *redgray_rd, *redgray_ad, *redgray_md;
      double *red_fa, *red_rd, *red_ad, *red_md;

      mitk::Image* tmpImg = m_CurrentStatisticsCalculator->GetInternalAdditionalResampledImage(0);
      mitk::Image::ConstPointer imgToCluster = tmpImg;

      red_fa   = clusterer->PerformQuantification(imgToCluster, m_CurrentRGBClusteringResults->rgbChannels->r, mask);
      green_fa = clusterer->PerformQuantification(imgToCluster, m_CurrentRGBClusteringResults->rgbChannels->g, mask);
      gray_fa  = clusterer->PerformQuantification(imgToCluster, m_CurrentRGBClusteringResults->rgbChannels->b, mask);

      tmpImg = m_CurrentStatisticsCalculator->GetInternalAdditionalResampledImage(3);
      mitk::Image::ConstPointer imgToCluster3 = tmpImg;

      red_rd   = clusterer->PerformQuantification(imgToCluster3, m_CurrentRGBClusteringResults->rgbChannels->r, mask);
      green_rd = clusterer->PerformQuantification(imgToCluster3, m_CurrentRGBClusteringResults->rgbChannels->g, mask);
      gray_rd  = clusterer->PerformQuantification(imgToCluster3, m_CurrentRGBClusteringResults->rgbChannels->b, mask);

      tmpImg = m_CurrentStatisticsCalculator->GetInternalAdditionalResampledImage(4);
      mitk::Image::ConstPointer imgToCluster4 = tmpImg;

      red_ad   = clusterer->PerformQuantification(imgToCluster4, m_CurrentRGBClusteringResults->rgbChannels->r, mask);
      green_ad = clusterer->PerformQuantification(imgToCluster4, m_CurrentRGBClusteringResults->rgbChannels->g, mask);
      gray_ad  = clusterer->PerformQuantification(imgToCluster4, m_CurrentRGBClusteringResults->rgbChannels->b, mask);

      tmpImg = m_CurrentStatisticsCalculator->GetInternalAdditionalResampledImage(5);
      mitk::Image::ConstPointer imgToCluster5 = tmpImg;

      red_md   = clusterer->PerformQuantification(imgToCluster5, m_CurrentRGBClusteringResults->rgbChannels->r, mask);
      green_md = clusterer->PerformQuantification(imgToCluster5, m_CurrentRGBClusteringResults->rgbChannels->g, mask);
      gray_md  = clusterer->PerformQuantification(imgToCluster5, m_CurrentRGBClusteringResults->rgbChannels->b, mask);

      // clipboard
      QString clipboardText("FA\t%1\t%2\t\t%3\t%4\t\t%5\t%6\t");
      clipboardText = clipboardText
                      .arg(red_fa[0]).arg(red_fa[1])
                      .arg(gray_fa[0]).arg(gray_fa[1])
                      .arg(green_fa[0]).arg(green_fa[1]);
      QString clipboardText3("RD\t%1\t%2\t\t%3\t%4\t\t%5\t%6\t");
      clipboardText3 = clipboardText3
                       .arg(red_rd[0]).arg(red_rd[1])
                       .arg(gray_rd[0]).arg(gray_rd[1])
                       .arg(green_rd[0]).arg(green_rd[1]);
      QString clipboardText4("AD\t%1\t%2\t\t%3\t%4\t\t%5\t%6\t");
      clipboardText4 = clipboardText4
                       .arg(red_ad[0]).arg(red_ad[1])
                       .arg(gray_ad[0]).arg(gray_ad[1])
                       .arg(green_ad[0]).arg(green_ad[1]);
      QString clipboardText5("MD\t%1\t%2\t\t%3\t%4\t\t%5\t%6");
      clipboardText5 = clipboardText5
                       .arg(red_md[0]).arg(red_md[1])
                       .arg(gray_md[0]).arg(gray_md[1])
                       .arg(green_md[0]).arg(green_md[1]);

      QApplication::clipboard()->setText(clipboardText+clipboardText3+clipboardText4+clipboardText5, QClipboard::Clipboard);

      // now paint infos also on renderwindow
      QString plainInfoText("%1  %2  %3        \n");
      plainInfoText = plainInfoText
                      .arg("Red  ", 20)
                      .arg("Gray ", 20)
                      .arg("Green", 20);

      QString plainInfoText0("FA:%1 ± %2%3 ± %4%5 ± %6\n");
      plainInfoText0 = plainInfoText0
                       .arg(red_fa[0], 10, 'g', 2, QLatin1Char( ' ' )).arg(red_fa[1], -10, 'g', 2, QLatin1Char( ' ' ))
                       .arg(gray_fa[0], 10, 'g', 2, QLatin1Char( ' ' )).arg(gray_fa[1], -10, 'g', 2, QLatin1Char( ' ' ))
                       .arg(green_fa[0], 10, 'g', 2, QLatin1Char( ' ' )).arg(green_fa[1], -10, 'g', 2, QLatin1Char( ' ' ));

      QString plainInfoText3("RDx10³:%1 ± %2%3 ± %4%5 ± %6\n");
      plainInfoText3 = plainInfoText3
                       .arg(1000.0 * red_rd[0], 10, 'g', 2, QLatin1Char( ' ' )).arg(1000.0 * red_rd[1], -10, 'g', 2, QLatin1Char( ' ' ))
                       .arg(1000.0 * gray_rd[0], 10, 'g', 2, QLatin1Char( ' ' )).arg(1000.0 * gray_rd[1], -10, 'g', 2, QLatin1Char( ' ' ))
                       .arg(1000.0 * green_rd[0], 10, 'g', 2, QLatin1Char( ' ' )).arg(1000.0 * green_rd[1], -10, 'g', 2, QLatin1Char( ' ' ));

      QString plainInfoText4("ADx10³:%1 ± %2%3 ± %4%5 ± %6\n");
      plainInfoText4 = plainInfoText4
                       .arg(1000.0 * red_ad[0], 10, 'g', 2, QLatin1Char( ' ' )).arg(1000.0 * red_ad[1], -10, 'g', 2, QLatin1Char( ' ' ))
                       .arg(1000.0 * gray_ad[0], 10, 'g', 2, QLatin1Char( ' ' )).arg(1000.0 * gray_ad[1], -10, 'g', 2, QLatin1Char( ' ' ))
                       .arg(1000.0 * green_ad[0], 10, 'g', 2, QLatin1Char( ' ' )).arg(1000.0 * green_ad[1], -10, 'g', 2, QLatin1Char( ' ' ));

      QString plainInfoText5("MDx10³:%1 ± %2%3 ± %4%5 ± %6");
      plainInfoText5 = plainInfoText5
                       .arg(1000.0 * red_md[0], 10, 'g', 2, QLatin1Char( ' ' )).arg(1000.0 * red_md[1], -10, 'g', 2, QLatin1Char( ' ' ))
                       .arg(1000.0 * gray_md[0], 10, 'g', 2, QLatin1Char( ' ' )).arg(1000.0 * gray_md[1], -10, 'g', 2, QLatin1Char( ' ' ))
                       .arg(1000.0 * green_md[0], 10, 'g', 2, QLatin1Char( ' ' )).arg(1000.0 * green_md[1], -10, 'g', 2, QLatin1Char( ' ' ));

      this->SetMeasurementInfoToRenderWindow(plainInfoText+plainInfoText0+plainInfoText3+plainInfoText4+plainInfoText5);

    }
    else
    {
      double* green;
      double* gray;
      double* red;

      mitk::Image* tmpImg = m_CurrentStatisticsCalculator->GetInternalImage();
      mitk::Image::ConstPointer imgToCluster = tmpImg;

      red = clusterer->PerformQuantification(imgToCluster, m_CurrentRGBClusteringResults->rgbChannels->r);
      green = clusterer->PerformQuantification(imgToCluster, m_CurrentRGBClusteringResults->rgbChannels->g);
      gray = clusterer->PerformQuantification(imgToCluster, m_CurrentRGBClusteringResults->rgbChannels->b);

      // clipboard
      QString clipboardText("%1\t%2\t\t%3\t%4\t\t%5\t%6");
      clipboardText = clipboardText.arg(red[0]).arg(red[1])
                      .arg(gray[0]).arg(gray[1])
                      .arg(green[0]).arg(green[1]);
      QApplication::clipboard()->setText(clipboardText, QClipboard::Clipboard);

      // now paint infos also on renderwindow
      QString plainInfoText("Red: %1 ± %2\nGray: %3 ± %4\nGreen: %5 ± %6");
      plainInfoText = plainInfoText.arg(red[0]).arg(red[1])
                      .arg(gray[0]).arg(gray[1])
                      .arg(green[0]).arg(green[1]);

      this->SetMeasurementInfoToRenderWindow(plainInfoText);

    }


    clusteredImage = m_CurrentRGBClusteringResults->rgb;

  }
  else
  {
    if(m_IsTensorImage)
    {
      double *red_fa, *red_rd, *red_ad, *red_md;

      mitk::Image* tmpImg = m_CurrentStatisticsCalculator->GetInternalAdditionalResampledImage(0);
      mitk::Image::ConstPointer imgToCluster = tmpImg;
      red_fa = clusterer->PerformQuantification(imgToCluster, m_CurrentPerformClusteringResults->clusteredImage, mask);

      tmpImg = m_CurrentStatisticsCalculator->GetInternalAdditionalResampledImage(3);
      mitk::Image::ConstPointer imgToCluster3 = tmpImg;
      red_rd = clusterer->PerformQuantification(imgToCluster3, m_CurrentPerformClusteringResults->clusteredImage, mask);

      tmpImg = m_CurrentStatisticsCalculator->GetInternalAdditionalResampledImage(4);
      mitk::Image::ConstPointer imgToCluster4 = tmpImg;
      red_ad = clusterer->PerformQuantification(imgToCluster4, m_CurrentPerformClusteringResults->clusteredImage, mask);

      tmpImg = m_CurrentStatisticsCalculator->GetInternalAdditionalResampledImage(5);
      mitk::Image::ConstPointer imgToCluster5 = tmpImg;
      red_md = clusterer->PerformQuantification(imgToCluster5, m_CurrentPerformClusteringResults->clusteredImage, mask);

      // clipboard
      QString clipboardText("FA\t%1\t%2\t");
      clipboardText = clipboardText
                      .arg(red_fa[0]).arg(red_fa[1]);
      QString clipboardText3("RD\t%1\t%2\t");
      clipboardText3 = clipboardText3
                       .arg(red_rd[0]).arg(red_rd[1]);
      QString clipboardText4("AD\t%1\t%2\t");
      clipboardText4 = clipboardText4
                       .arg(red_ad[0]).arg(red_ad[1]);
      QString clipboardText5("MD\t%1\t%2\t");
      clipboardText5 = clipboardText5
                       .arg(red_md[0]).arg(red_md[1]);

      QApplication::clipboard()->setText(clipboardText+clipboardText3+clipboardText4+clipboardText5, QClipboard::Clipboard);

      // now paint infos also on renderwindow
      QString plainInfoText("%1        \n");
      plainInfoText = plainInfoText
                      .arg("Red  ", 20);

      QString plainInfoText0("FA:%1 ± %2\n");
      plainInfoText0 = plainInfoText0
                       .arg(red_fa[0], 10, 'g', 2, QLatin1Char( ' ' )).arg(red_fa[1], -10, 'g', 2, QLatin1Char( ' ' ));

      QString plainInfoText3("RDx10³:%1 ± %2\n");
      plainInfoText3 = plainInfoText3
                       .arg(1000.0 * red_rd[0], 10, 'g', 2, QLatin1Char( ' ' )).arg(1000.0 * red_rd[1], -10, 'g', 2, QLatin1Char( ' ' ));

      QString plainInfoText4("ADx10³:%1 ± %2\n");
      plainInfoText4 = plainInfoText4
                       .arg(1000.0 * red_ad[0], 10, 'g', 2, QLatin1Char( ' ' )).arg(1000.0 * red_ad[1], -10, 'g', 2, QLatin1Char( ' ' ));

      QString plainInfoText5("MDx10³:%1 ± %2");
      plainInfoText5 = plainInfoText5
                       .arg(1000.0 * red_md[0], 10, 'g', 2, QLatin1Char( ' ' )).arg(1000.0 * red_md[1], -10, 'g', 2, QLatin1Char( ' ' ));

      this->SetMeasurementInfoToRenderWindow(plainInfoText+plainInfoText0+plainInfoText3+plainInfoText4+plainInfoText5);

    }
    else
    {
      double* quant;

      mitk::Image* tmpImg = m_CurrentStatisticsCalculator->GetInternalImage();
      mitk::Image::ConstPointer imgToCluster = tmpImg;
      quant = clusterer->PerformQuantification(imgToCluster, m_CurrentPerformClusteringResults->clusteredImage);

      // clipboard
      QString clipboardText("%1\t%2");
      clipboardText = clipboardText.arg(quant[0]).arg(quant[1]);
      QApplication::clipboard()->setText(clipboardText, QClipboard::Clipboard);

      // now paint infos also on renderwindow
      QString plainInfoText("Measurement: %1 ± %2");
      plainInfoText = plainInfoText.arg(quant[0]).arg(quant[1]);

      this->SetMeasurementInfoToRenderWindow(plainInfoText);
    }

    clusteredImage = m_CurrentPerformClusteringResults->displayImage;

  }

  if(mask.IsNotNull())
  {
    typedef itk::Image<itk::RGBAPixel<unsigned char>,3> RGBImageType;
    typedef mitk::ImageToItk<RGBImageType> ClusterCasterType;
    ClusterCasterType::Pointer clCaster = ClusterCasterType::New();
    clCaster->SetInput(clusteredImage);
    clCaster->Update();
    clCaster->GetOutput();

    typedef itk::MaskImageFilter< RGBImageType, MaskImageType, RGBImageType > MaskType;
    MaskType::Pointer masker = MaskType::New();
    masker->SetInput1(clCaster->GetOutput());
    masker->SetInput2(itkmask);
    masker->Update();

    clusteredImage = mitk::Image::New();
    clusteredImage->InitializeByItk(masker->GetOutput());
    clusteredImage->SetVolume(masker->GetOutput()->GetBufferPointer());
  }

  if(m_ClusteringResult.IsNotNull())
  {
    GetDefaultDataStorage()->Remove(m_ClusteringResult);
  }

  m_ClusteringResult = mitk::DataNode::New();
  m_ClusteringResult->SetBoolProperty("helper object", true);
  m_ClusteringResult->SetIntProperty( "layer", 1000 );
  m_ClusteringResult->SetBoolProperty("texture interpolation", m_TexIsOn);
  m_ClusteringResult->SetData(clusteredImage);
  m_ClusteringResult->SetName("Clusterprobs");
  GetDefaultDataStorage()->Add(m_ClusteringResult, m_SelectedImageNodes->GetNode());

  if(m_SelectedPlanarFigure.IsNotNull() && m_SelectedPlanarFigureNodes->GetNode().IsNotNull())
  {
    m_SelectedPlanarFigureNodes->GetNode()->SetIntProperty( "layer", 1001 );
  }

  GetActiveStdMultiWidget()->RequestUpdate();

}


void QmitkPartialVolumeAnalysisView::UpdateStatistics()
{

  MITK_INFO << "UpdateStatistics()";

  if(!m_CurrentFigureNodeInitialized && m_SelectedPlanarFigure.IsNotNull())
  {
    MITK_INFO << "Selected planar figure not initialized. No stats calculation performed.";
    return;
  }

  // Remove any cached images that are no longer referenced elsewhere
  this->RemoveOrphanImages();

  QmitkStdMultiWidget *multiWidget = this->GetActiveStdMultiWidget();
  if ( multiWidget == NULL )
  {
    return;
  }

  if ( m_SelectedImage.IsNotNull() )
  {
    // Check if a the selected image is a multi-channel image. If yes, statistics
    // cannot be calculated currently.
    if ( !m_IsTensorImage && m_SelectedImage->GetPixelType().GetNumberOfComponents() > 1 )
    {
      std::stringstream message;
      message << "Non-tensor multi-component images not supported.";
      m_Controls->m_ErrorMessageLabel->setText( message.str().c_str() );
      m_Controls->m_ErrorMessageLabel->show();

      m_Controls->m_HistogramWidget->ClearItemModel();
      m_CurrentStatisticsValid = false;
      return;
    }

    // Retrieve HistogramStatisticsCalculator from has map (or create a new one
    // for this image if non-existant)
    PartialVolumeAnalysisMapType::iterator it =
        m_PartialVolumeAnalysisMap.find( m_SelectedImage );

    if ( it != m_PartialVolumeAnalysisMap.end() )
    {
      m_CurrentStatisticsCalculator = it->second;
      MITK_INFO << "Retrieving StatisticsCalculator";
    }
    else
    {
      m_CurrentStatisticsCalculator = mitk::PartialVolumeAnalysisHistogramCalculator::New();
      m_CurrentStatisticsCalculator->SetPlanarFigureThickness(m_Controls->m_PlanarFiguresThickness->value());
      if(m_IsTensorImage)
      {
        m_CurrentStatisticsCalculator->SetImage( m_CAImage );
        m_CurrentStatisticsCalculator->AddAdditionalResamplingImage( m_FAImage );
        m_CurrentStatisticsCalculator->AddAdditionalResamplingImage( m_DirectionComp1Image );
        m_CurrentStatisticsCalculator->AddAdditionalResamplingImage( m_DirectionComp2Image );
        m_CurrentStatisticsCalculator->AddAdditionalResamplingImage( m_RDImage );
        m_CurrentStatisticsCalculator->AddAdditionalResamplingImage( m_ADImage );
        m_CurrentStatisticsCalculator->AddAdditionalResamplingImage( m_MDImage );
      }
      else
      {
        m_CurrentStatisticsCalculator->SetImage( m_SelectedImage );
      }
      m_PartialVolumeAnalysisMap[m_SelectedImage] = m_CurrentStatisticsCalculator;
      MITK_INFO << "Creating StatisticsCalculator";
    }

    std::string maskName;
    std::string maskType;
    unsigned int maskDimension;

    if ( m_SelectedImageMask.IsNotNull() )
    {

      mitk::PixelType pixelType = m_SelectedImageMask->GetPixelType();
      std::cout << pixelType.GetType() << std::endl;

      if(pixelType.GetBitsPerComponent() == 16)
      {
        //convert from short to uchar
        typedef itk::Image<short, 3> ShortImageType;
        typedef itk::Image<unsigned char, 3> CharImageType;

        CharImageType::Pointer charImage;
        ShortImageType::Pointer shortImage;
        mitk::CastToItkImage(m_SelectedImageMask, shortImage);

        typedef itk::CastImageFilter<ShortImageType, CharImageType> ImageCasterType;

        ImageCasterType::Pointer caster = ImageCasterType::New();
        caster->SetInput( shortImage );
        caster->Update();
        charImage = caster->GetOutput();


        mitk::CastToMitkImage(charImage, m_SelectedImageMask);
      }

      m_CurrentStatisticsCalculator->SetImageMask( m_SelectedImageMask );


      m_CurrentStatisticsCalculator->SetMaskingModeToImage();

      maskName = m_SelectedMaskNode->GetName();
      maskType = m_SelectedImageMask->GetNameOfClass();
      maskDimension = 3;

      std::stringstream maskLabel;
      maskLabel << maskName;
      if ( maskDimension > 0 )
      {
        maskLabel << "  [" << maskDimension << "D " << maskType << "]";
      }
      m_Controls->m_SelectedMaskLabel->setText( maskLabel.str().c_str() );
    }
    else if ( m_SelectedPlanarFigure.IsNotNull() && m_SelectedPlanarFigureNodes->GetNode().IsNotNull())
    {
      m_CurrentStatisticsCalculator->SetPlanarFigure( m_SelectedPlanarFigure );
      m_CurrentStatisticsCalculator->SetMaskingModeToPlanarFigure();

      maskName = m_SelectedPlanarFigureNodes->GetNode()->GetName();
      maskType = m_SelectedPlanarFigure->GetNameOfClass();
      maskDimension = 2;
    }
    else
    {
      m_CurrentStatisticsCalculator->SetMaskingModeToNone();

      maskName = "None";
      maskType = "";
      maskDimension = 0;
    }

    bool statisticsChanged = false;
    bool statisticsCalculationSuccessful = false;

    // Initialize progress bar
    mitk::ProgressBar::GetInstance()->AddStepsToDo( 100 );

    // Install listener for progress events and initialize progress bar
    typedef itk::SimpleMemberCommand< QmitkPartialVolumeAnalysisView > ITKCommandType;
    ITKCommandType::Pointer progressListener;
    progressListener = ITKCommandType::New();
    progressListener->SetCallbackFunction( this, &QmitkPartialVolumeAnalysisView::UpdateProgressBar );
    unsigned long progressObserverTag = m_CurrentStatisticsCalculator
                                        ->AddObserver( itk::ProgressEvent(), progressListener );

    ClusteringType::ParamsType *cparams = 0;
    ClusteringType::ClusterResultType *cresult = 0;
    ClusteringType::HistType *chist = 0;

    try
    {
      m_CurrentStatisticsCalculator->SetNumberOfBins(m_Controls->m_NumberBins->text().toInt());
      m_CurrentStatisticsCalculator->SetUpsamplingFactor(m_Controls->m_Upsampling->text().toDouble());
      m_CurrentStatisticsCalculator->SetGaussianSigma(m_Controls->m_GaussianSigma->text().toDouble());

      // Compute statistics
      statisticsChanged =
          m_CurrentStatisticsCalculator->ComputeStatistics( );

      mitk::Image* tmpImg = m_CurrentStatisticsCalculator->GetInternalImage();
      mitk::Image::ConstPointer imgToCluster = tmpImg;
      if(imgToCluster.IsNotNull())
      {

        // perform clustering
        const HistogramType *histogram = m_CurrentStatisticsCalculator->GetHistogram( );
        ClusteringType::Pointer clusterer = ClusteringType::New();
        clusterer->SetStepsNumIntegration(200);
        clusterer->SetMaxIt(1000);

        mitk::Image::Pointer pFiberImg;
        if(m_QuantifyClass==3)
        {
          if(m_Controls->m_Quantiles->isChecked())
          {
            m_CurrentRGBClusteringResults = clusterer->PerformRGBQuantiles(imgToCluster, histogram, m_Controls->m_q1->value(),m_Controls->m_q2->value());
          }
          else
          {
            m_CurrentRGBClusteringResults = clusterer->PerformRGBClustering(imgToCluster, histogram);
          }

          pFiberImg = m_CurrentRGBClusteringResults->rgbChannels->r;
          cparams = m_CurrentRGBClusteringResults->params;
          cresult = m_CurrentRGBClusteringResults->result;
          chist = m_CurrentRGBClusteringResults->hist;
        }
        else
        {
          if(m_Controls->m_Quantiles->isChecked())
          {
            m_CurrentPerformClusteringResults =
                clusterer->PerformQuantiles(imgToCluster, histogram, m_Controls->m_q1->value(),m_Controls->m_q2->value());
          }
          else
          {
            m_CurrentPerformClusteringResults =
              clusterer->PerformClustering(imgToCluster, histogram, m_QuantifyClass);
          }

          pFiberImg = m_CurrentPerformClusteringResults->clusteredImage;
          cparams = m_CurrentPerformClusteringResults->params;
          cresult = m_CurrentPerformClusteringResults->result;
          chist = m_CurrentPerformClusteringResults->hist;
        }

        if(m_IsTensorImage)
        {
          m_AngularErrorImage = clusterer->CaculateAngularErrorImage(
              m_CurrentStatisticsCalculator->GetInternalAdditionalResampledImage(1),
              m_CurrentStatisticsCalculator->GetInternalAdditionalResampledImage(2),
              pFiberImg);

          //          GetDefaultDataStorage()->Remove(m_newnode2);
          //          m_newnode2 = mitk::DataNode::New();
          //          m_newnode2->SetData(m_AngularErrorImage);
          //          m_newnode2->SetName(("AngularError"));
          //          m_newnode2->SetIntProperty( "layer", 1003 );
          //          GetDefaultDataStorage()->Add(m_newnode2, m_SelectedImageNodes->GetNode());

          //          newnode = mitk::DataNode::New();
          //          newnode->SetData(m_CurrentStatisticsCalculator->GetInternalAdditionalResampledImage(1));
          //          newnode->SetName(("Comp1"));
          //          GetDefaultDataStorage()->Add(newnode, m_SelectedImageNodes->GetNode());

          //          newnode = mitk::DataNode::New();
          //          newnode->SetData(m_CurrentStatisticsCalculator->GetInternalAdditionalResampledImage(2));
          //          newnode->SetName(("Comp2"));
          //          GetDefaultDataStorage()->Add(newnode, m_SelectedImageNodes->GetNode());
        }
        ShowClusteringResults();
      }

      statisticsCalculationSuccessful = true;
    }
    catch ( const std::runtime_error &e )
    {
      // In case of exception, print error message on GUI
      std::stringstream message;
      message << e.what();
      m_Controls->m_ErrorMessageLabel->setText( message.str().c_str() );
      m_Controls->m_ErrorMessageLabel->show();
    }
    catch ( const std::exception &e )
    {
      MITK_ERROR << "Caught exception: " << e.what();

      // In case of exception, print error message on GUI
      std::stringstream message;
      message << "Error in calculating histogram: " << e.what();
      m_Controls->m_ErrorMessageLabel->setText( message.str().c_str() );
      m_Controls->m_ErrorMessageLabel->show();
    }

    m_CurrentStatisticsCalculator->RemoveObserver( progressObserverTag );

    // Make sure that progress bar closes
    mitk::ProgressBar::GetInstance()->Progress( 100 );

    if ( statisticsCalculationSuccessful )
    {
      if ( statisticsChanged )
      {
        // Do not show any error messages
        m_Controls->m_ErrorMessageLabel->hide();

        m_CurrentStatisticsValid = true;
      }

      //      m_Controls->m_HistogramWidget->SetHistogramModeToDirectHistogram();
      m_Controls->m_HistogramWidget->SetParameters(
          cparams, cresult, chist );
      //      m_Controls->m_HistogramWidget->UpdateItemModelFromHistogram();

    }
    else
    {
      m_Controls->m_SelectedMaskLabel->setText( "None" );

      // Clear statistics and histogram
      m_Controls->m_HistogramWidget->ClearItemModel();
      m_CurrentStatisticsValid = false;


      // If a (non-closed) PlanarFigure is selected, display a line profile widget
      if ( m_SelectedPlanarFigure.IsNotNull() )
      {
        // TODO: enable line profile widget
        //m_Controls->m_StatisticsWidgetStack->setCurrentIndex( 1 );
        //m_Controls->m_LineProfileWidget->SetImage( m_SelectedImage );
        //m_Controls->m_LineProfileWidget->SetPlanarFigure( m_SelectedPlanarFigure );
        //m_Controls->m_LineProfileWidget->UpdateItemModelFromPath();
      }
    }
  }

}

void QmitkPartialVolumeAnalysisView::SetMeasurementInfoToRenderWindow(const QString& text)
{

  FindRenderWindow(m_SelectedPlanarFigureNodes->GetNode());

  if(m_LastRenderWindow != m_SelectedRenderWindow)
  {

    if(m_LastRenderWindow)
    {
      QObject::disconnect( m_LastRenderWindow, SIGNAL( destroyed(QObject*) )
                           , this, SLOT( OnRenderWindowDelete(QObject*) ) );
    }
    m_LastRenderWindow = m_SelectedRenderWindow;
    if(m_LastRenderWindow)
    {
      QObject::connect( m_LastRenderWindow, SIGNAL( destroyed(QObject*) )
                        , this, SLOT( OnRenderWindowDelete(QObject*) ) );
    }
  }

  if(m_LastRenderWindow && m_SelectedPlanarFigureNodes->GetNode().IsNotNull())
  {
    if (!text.isEmpty())
    {
      m_MeasurementInfoAnnotation->SetText(1, text.toLatin1().data());
      mitk::VtkLayerController::GetInstance(m_LastRenderWindow->GetRenderWindow())->InsertForegroundRenderer(
          m_MeasurementInfoRenderer, true);
    }
    else
    {
      if (mitk::VtkLayerController::GetInstance(
          m_LastRenderWindow->GetRenderWindow()) ->IsRendererInserted(
              m_MeasurementInfoRenderer))
        mitk::VtkLayerController::GetInstance(m_LastRenderWindow->GetRenderWindow())->RemoveRenderer(
            m_MeasurementInfoRenderer);
    }
  }
  else
  {
    if (!text.isEmpty())
    {
      m_MeasurementInfoAnnotation->SetText(1, text.toLatin1().data());
      mitk::VtkLayerController::GetInstance(this->GetActiveStdMultiWidget()->GetRenderWindow1()->GetRenderWindow())->InsertForegroundRenderer(
          m_MeasurementInfoRenderer, true);
    }
    else
    {
      if (mitk::VtkLayerController::GetInstance(
          this->GetActiveStdMultiWidget()->GetRenderWindow1()->GetRenderWindow()) ->IsRendererInserted(
              m_MeasurementInfoRenderer))
        mitk::VtkLayerController::GetInstance(this->GetActiveStdMultiWidget()->GetRenderWindow1()->GetRenderWindow())->RemoveRenderer(
            m_MeasurementInfoRenderer);
    }
  }
}

void QmitkPartialVolumeAnalysisView::UpdateProgressBar()
{
  mitk::ProgressBar::GetInstance()->Progress();
}

void QmitkPartialVolumeAnalysisView::RequestStatisticsUpdate()
{
  if ( !m_StatisticsUpdatePending )
  {
    QApplication::postEvent( this, new QmitkRequestStatisticsUpdateEvent );
    m_StatisticsUpdatePending = true;
  }
}


void QmitkPartialVolumeAnalysisView::RemoveOrphanImages()
{
  PartialVolumeAnalysisMapType::iterator it = m_PartialVolumeAnalysisMap.begin();

  while ( it != m_PartialVolumeAnalysisMap.end() )
  {
    mitk::Image *image = it->first;
    mitk::PartialVolumeAnalysisHistogramCalculator *calculator = it->second;
    ++it;

    mitk::NodePredicateData::Pointer hasImage = mitk::NodePredicateData::New( image );
    if ( this->GetDefaultDataStorage()->GetNode( hasImage ) == NULL )
    {
      if ( m_SelectedImage == image )
      {
        m_SelectedImage = NULL;
        m_SelectedImageNodes->RemoveAllNodes();
      }
      if ( m_CurrentStatisticsCalculator == calculator )
      {
        m_CurrentStatisticsCalculator = NULL;
      }
      m_PartialVolumeAnalysisMap.erase( image );
      it = m_PartialVolumeAnalysisMap.begin();
    }
  }
}

void QmitkPartialVolumeAnalysisView::ExtractTensorImages(
    mitk::Image::ConstPointer tensorimage)
{
  typedef itk::Image< itk::DiffusionTensor3D<float>, 3> TensorImageType;

  typedef mitk::ImageToItk<TensorImageType> CastType;
  CastType::Pointer caster = CastType::New();
  caster->SetInput(tensorimage);
  caster->Update();
  TensorImageType::Pointer image = caster->GetOutput();

  typedef itk::TensorDerivedMeasurementsFilter<float> MeasurementsType;
  MeasurementsType::Pointer measurementsCalculator = MeasurementsType::New();
  measurementsCalculator->SetInput(image );
  measurementsCalculator->SetMeasure(MeasurementsType::FA);
  measurementsCalculator->Update();
  MeasurementsType::OutputImageType::Pointer fa = measurementsCalculator->GetOutput();

  m_FAImage = mitk::Image::New();
  m_FAImage->InitializeByItk(fa.GetPointer());
  m_FAImage->SetVolume(fa->GetBufferPointer());

  //  mitk::DataNode::Pointer node = mitk::DataNode::New();
  //  node->SetData(m_FAImage);
  //  GetDefaultDataStorage()->Add(node);

  measurementsCalculator = MeasurementsType::New();
  measurementsCalculator->SetInput(image );
  measurementsCalculator->SetMeasure(MeasurementsType::CA);
  measurementsCalculator->Update();
  MeasurementsType::OutputImageType::Pointer ca = measurementsCalculator->GetOutput();

  m_CAImage = mitk::Image::New();
  m_CAImage->InitializeByItk(ca.GetPointer());
  m_CAImage->SetVolume(ca->GetBufferPointer());

  //  node = mitk::DataNode::New();
  //  node->SetData(m_CAImage);
  //  GetDefaultDataStorage()->Add(node);

  measurementsCalculator = MeasurementsType::New();
  measurementsCalculator->SetInput(image );
  measurementsCalculator->SetMeasure(MeasurementsType::RD);
  measurementsCalculator->Update();
  MeasurementsType::OutputImageType::Pointer rd = measurementsCalculator->GetOutput();

  m_RDImage = mitk::Image::New();
  m_RDImage->InitializeByItk(rd.GetPointer());
  m_RDImage->SetVolume(rd->GetBufferPointer());

  //  node = mitk::DataNode::New();
  //  node->SetData(m_CAImage);
  //  GetDefaultDataStorage()->Add(node);

  measurementsCalculator = MeasurementsType::New();
  measurementsCalculator->SetInput(image );
  measurementsCalculator->SetMeasure(MeasurementsType::AD);
  measurementsCalculator->Update();
  MeasurementsType::OutputImageType::Pointer ad = measurementsCalculator->GetOutput();

  m_ADImage = mitk::Image::New();
  m_ADImage->InitializeByItk(ad.GetPointer());
  m_ADImage->SetVolume(ad->GetBufferPointer());

  //  node = mitk::DataNode::New();
  //  node->SetData(m_CAImage);
  //  GetDefaultDataStorage()->Add(node);

  measurementsCalculator = MeasurementsType::New();
  measurementsCalculator->SetInput(image );
  measurementsCalculator->SetMeasure(MeasurementsType::RA);
  measurementsCalculator->Update();
  MeasurementsType::OutputImageType::Pointer md = measurementsCalculator->GetOutput();

  m_MDImage = mitk::Image::New();
  m_MDImage->InitializeByItk(md.GetPointer());
  m_MDImage->SetVolume(md->GetBufferPointer());

  //  node = mitk::DataNode::New();
  //  node->SetData(m_CAImage);
  //  GetDefaultDataStorage()->Add(node);

  typedef DirectionsFilterType::OutputImageType DirImageType;
  DirectionsFilterType::Pointer dirFilter = DirectionsFilterType::New();
  dirFilter->SetInput(image );
  dirFilter->Update();

  itk::ImageRegionIterator<DirImageType>
      itd(dirFilter->GetOutput(), dirFilter->GetOutput()->GetLargestPossibleRegion());
  itd = itd.Begin();
  while( !itd.IsAtEnd() )
  {
    DirImageType::PixelType direction = itd.Get();
    direction[0] = fabs(direction[0]);
    direction[1] = fabs(direction[1]);
    direction[2] = fabs(direction[2]);
    itd.Set(direction);
    ++itd;
  }

  typedef itk::CartesianToPolarVectorImageFilter<
      DirImageType, DirImageType, true> C2PFilterType;
  C2PFilterType::Pointer cpFilter = C2PFilterType::New();
  cpFilter->SetInput(dirFilter->GetOutput());
  cpFilter->Update();
  DirImageType::Pointer dir = cpFilter->GetOutput();

  typedef itk::Image<float, 3> CompImageType;
  CompImageType::Pointer comp1 = CompImageType::New();
  comp1->SetSpacing( dir->GetSpacing() );   // Set the image spacing
  comp1->SetOrigin( dir->GetOrigin() );     // Set the image origin
  comp1->SetDirection( dir->GetDirection() );  // Set the image direction
  comp1->SetRegions( dir->GetLargestPossibleRegion() );
  comp1->Allocate();

  CompImageType::Pointer comp2 = CompImageType::New();
  comp2->SetSpacing( dir->GetSpacing() );   // Set the image spacing
  comp2->SetOrigin( dir->GetOrigin() );     // Set the image origin
  comp2->SetDirection( dir->GetDirection() );  // Set the image direction
  comp2->SetRegions( dir->GetLargestPossibleRegion() );
  comp2->Allocate();

  itk::ImageRegionConstIterator<DirImageType>
      it(dir, dir->GetLargestPossibleRegion());

  itk::ImageRegionIterator<CompImageType>
      it1(comp1, comp1->GetLargestPossibleRegion());

  itk::ImageRegionIterator<CompImageType>
      it2(comp2, comp2->GetLargestPossibleRegion());

  it = it.Begin();
  it1 = it1.Begin();
  it2 = it2.Begin();

  while( !it.IsAtEnd() )
  {
    it1.Set(it.Get()[1]);
    it2.Set(it.Get()[2]);
    ++it;
    ++it1;
    ++it2;
  }

  m_DirectionComp1Image = mitk::Image::New();
  m_DirectionComp1Image->InitializeByItk(comp1.GetPointer());
  m_DirectionComp1Image->SetVolume(comp1->GetBufferPointer());

  m_DirectionComp2Image = mitk::Image::New();
  m_DirectionComp2Image->InitializeByItk(comp2.GetPointer());
  m_DirectionComp2Image->SetVolume(comp2->GetBufferPointer());

}

void QmitkPartialVolumeAnalysisView::OnRenderWindowDelete(QObject * obj)
{
  if(obj == m_LastRenderWindow)
    m_LastRenderWindow = 0;
}

bool QmitkPartialVolumeAnalysisView::event( QEvent *event )
{
  if ( event->type() == (QEvent::Type) QmitkRequestStatisticsUpdateEvent::StatisticsUpdateRequest )
  {
    // Update statistics

    m_StatisticsUpdatePending = false;

    this->UpdateStatistics();
    return true;
  }

  return false;
}

void QmitkPartialVolumeAnalysisView::Visible()
{
  this->OnSelectionChanged( this->GetDataManagerSelection() );
}

bool QmitkPartialVolumeAnalysisView::IsExclusiveFunctionality() const
{
  return true;
}

void QmitkPartialVolumeAnalysisView::Activated()
{
  this->GetActiveStdMultiWidget()->SetWidgetPlanesVisibility(false);
  //this->GetActiveStdMultiWidget()->GetRenderWindow1()->FullScreenMode(true);

  mitk::DataStorage::SetOfObjects::ConstPointer _NodeSet = this->GetDefaultDataStorage()->GetAll();
  mitk::DataNode* node = 0;
  mitk::PlanarFigure* figure = 0;
  mitk::PlanarFigureInteractor::Pointer figureInteractor = 0;

  // finally add all nodes to the model
  for(mitk::DataStorage::SetOfObjects::ConstIterator it=_NodeSet->Begin(); it!=_NodeSet->End()
    ; it++)
    {
    node = const_cast<mitk::DataNode*>(it->Value().GetPointer());
    figure = dynamic_cast<mitk::PlanarFigure*>(node->GetData());

    if(figure)
    {
      figureInteractor = dynamic_cast<mitk::PlanarFigureInteractor*>(node->GetInteractor());

      if(figureInteractor.IsNull())
        figureInteractor = mitk::PlanarFigureInteractor::New("PlanarFigureInteractor", node);

      mitk::GlobalInteraction::GetInstance()->AddInteractor(figureInteractor);
    }
  }

  m_Visible = true;

}

void QmitkPartialVolumeAnalysisView::Deactivated()
{
  this->GetActiveStdMultiWidget()->SetWidgetPlanesVisibility(true);
  //this->GetActiveStdMultiWidget()->GetRenderWindow1()->FullScreenMode(false);

  this->SetMeasurementInfoToRenderWindow("");

  mitk::DataStorage::SetOfObjects::ConstPointer _NodeSet = this->GetDefaultDataStorage()->GetAll();
  mitk::DataNode* node = 0;
  mitk::PlanarFigure* figure = 0;
  mitk::PlanarFigureInteractor::Pointer figureInteractor = 0;

  // finally add all nodes to the model
  for(mitk::DataStorage::SetOfObjects::ConstIterator it=_NodeSet->Begin(); it!=_NodeSet->End()
    ; it++)
    {
    node = const_cast<mitk::DataNode*>(it->Value().GetPointer());
    figure = dynamic_cast<mitk::PlanarFigure*>(node->GetData());

    if(figure)
    {
      figureInteractor = dynamic_cast<mitk::PlanarFigureInteractor*>(node->GetInteractor());

      if(figureInteractor)
        mitk::GlobalInteraction::GetInstance()->RemoveInteractor(figureInteractor);
    }
  }

  m_Visible = false;
}

void QmitkPartialVolumeAnalysisView::GreenRadio(bool checked)
{
  if(checked)
  {
    m_Controls->m_PartialVolumeRadio->setChecked(false);
    m_Controls->m_BlueRadio->setChecked(false);
    m_Controls->m_AllRadio->setChecked(false);
  }

  m_QuantifyClass = 0;

  RequestStatisticsUpdate();
}


void QmitkPartialVolumeAnalysisView::PartialVolumeRadio(bool checked)
{
  if(checked)
  {
    m_Controls->m_GreenRadio->setChecked(false);
    m_Controls->m_BlueRadio->setChecked(false);
    m_Controls->m_AllRadio->setChecked(false);
  }

  m_QuantifyClass = 1;

  RequestStatisticsUpdate();
}

void QmitkPartialVolumeAnalysisView::BlueRadio(bool checked)
{
  if(checked)
  {
    m_Controls->m_PartialVolumeRadio->setChecked(false);
    m_Controls->m_GreenRadio->setChecked(false);
    m_Controls->m_AllRadio->setChecked(false);
  }

  m_QuantifyClass = 2;

  RequestStatisticsUpdate();
}

void QmitkPartialVolumeAnalysisView::AllRadio(bool checked)
{
  if(checked)
  {
    m_Controls->m_BlueRadio->setChecked(false);
    m_Controls->m_PartialVolumeRadio->setChecked(false);
    m_Controls->m_GreenRadio->setChecked(false);
  }

  m_QuantifyClass = 3;

  RequestStatisticsUpdate();
}

void QmitkPartialVolumeAnalysisView::NumberBinsChangedSlider(int v )
{
  m_Controls->m_NumberBins->setText(QString("%1").arg(m_Controls->m_NumberBinsSlider->value()*5.0));
}

void QmitkPartialVolumeAnalysisView::UpsamplingChangedSlider( int v)
{
  m_Controls->m_Upsampling->setText(QString("%1").arg(m_Controls->m_UpsamplingSlider->value()/10.0));
}

void QmitkPartialVolumeAnalysisView::GaussianSigmaChangedSlider(int v )
{
  m_Controls->m_GaussianSigma->setText(QString("%1").arg(m_Controls->m_GaussianSigmaSlider->value()/100.0));
}

void QmitkPartialVolumeAnalysisView::SimilarAnglesChangedSlider(int v )
{
  m_Controls->m_SimilarAngles->setText(QString("%1°").arg(90-m_Controls->m_SimilarAnglesSlider->value()));
  ShowClusteringResults();
}

void QmitkPartialVolumeAnalysisView::OpacityChangedSlider(int v )
{

  if(m_SelectedImageNodes->GetNode().IsNotNull())
  {
    float opacImag = 1.0f-(v-5)/5.0f;
    opacImag = opacImag < 0 ? 0 : opacImag;
    m_SelectedImageNodes->GetNode()->SetFloatProperty("opacity", opacImag);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }

  if(m_ClusteringResult.IsNotNull())
  {
    float opacClust = v/5.0f;
    opacClust = opacClust > 1 ? 1 : opacClust;
    m_ClusteringResult->SetFloatProperty("opacity", opacClust);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkPartialVolumeAnalysisView::NumberBinsReleasedSlider( )
{
  RequestStatisticsUpdate();
}

void QmitkPartialVolumeAnalysisView::UpsamplingReleasedSlider( )
{
  RequestStatisticsUpdate();
}

void QmitkPartialVolumeAnalysisView::GaussianSigmaReleasedSlider( )
{
  RequestStatisticsUpdate();
}

void QmitkPartialVolumeAnalysisView::SimilarAnglesReleasedSlider( )
{

}

void QmitkPartialVolumeAnalysisView::ToClipBoard()
{

  std::vector<std::vector<double>* > vals  = m_Controls->m_HistogramWidget->m_Vals;
  QString clipboardText;
  for (std::vector<std::vector<double>* >::iterator it = vals.begin(); it
                                                         != vals.end(); ++it)
  {
    for (std::vector<double>::iterator it2 = (**it).begin(); it2 != (**it).end(); ++it2)
    {
      clipboardText.append(QString("%1 \t").arg(*it2));
    }
    clipboardText.append(QString("\n"));
  }

  QApplication::clipboard()->setText(clipboardText, QClipboard::Clipboard);
}

void QmitkPartialVolumeAnalysisView::PropertyChanged(const mitk::DataNode* /*node*/, const mitk::BaseProperty* /*prop*/)
{
}

void QmitkPartialVolumeAnalysisView::NodeChanged(const mitk::DataNode* /*node*/)
{
}

void QmitkPartialVolumeAnalysisView::NodeRemoved(const mitk::DataNode* node)
{

  if(  node == m_SelectedPlanarFigureNodes->GetNode().GetPointer()
    || node == m_SelectedMaskNode.GetPointer() )
    {
    this->Select(NULL,true,false);
    SetMeasurementInfoToRenderWindow("");
  }

  if(  node == m_SelectedImageNodes->GetNode().GetPointer() )
  {
    this->Select(NULL,false,true);
    SetMeasurementInfoToRenderWindow("");
  }
}

void QmitkPartialVolumeAnalysisView::NodeAddedInDataStorage(const mitk::DataNode* node)
{
  if(!m_Visible)
    return;

  mitk::DataNode* nonConstNode = const_cast<mitk::DataNode*>(node);
  mitk::PlanarFigure* figure = dynamic_cast<mitk::PlanarFigure*>(nonConstNode->GetData());

  if(figure)
  {

    // set interactor for new node (if not already set)
    mitk::PlanarFigureInteractor::Pointer figureInteractor
        = dynamic_cast<mitk::PlanarFigureInteractor*>(node->GetInteractor());

    if(figureInteractor.IsNull())
      figureInteractor = mitk::PlanarFigureInteractor::New("PlanarFigureInteractor", nonConstNode);

    mitk::GlobalInteraction::GetInstance()->AddInteractor(figureInteractor);

    // remove uninitialized old planars
    if( m_SelectedPlanarFigureNodes->GetNode().IsNotNull() && m_CurrentFigureNodeInitialized == false )
    {
      mitk::Interactor::Pointer oldInteractor = m_SelectedPlanarFigureNodes->GetNode()->GetInteractor();
      if(oldInteractor.IsNotNull())
        mitk::GlobalInteraction::GetInstance()->RemoveInteractor(oldInteractor);

      this->GetDefaultDataStorage()->Remove(m_SelectedPlanarFigureNodes->GetNode());
    }

  }
}

void QmitkPartialVolumeAnalysisView::TextIntON()
{
  if(m_ClusteringResult.IsNotNull())
  {
    if(m_TexIsOn)
    {
      m_Controls->m_TextureIntON->setIcon(*m_IconTexOFF);
    }
    else
    {
      m_Controls->m_TextureIntON->setIcon(*m_IconTexON);
    }

    m_ClusteringResult->SetBoolProperty("texture interpolation", !m_TexIsOn);
    m_TexIsOn = !m_TexIsOn;
    GetActiveStdMultiWidget()->RequestUpdate();
  }
}
