/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkControlVisualizationPropertiesView.h"

#include "mitkNodePredicateDataType.h"
#include "mitkDataNodeObject.h"
#include "mitkOdfNormalizationMethodProperty.h"
#include "mitkOdfScaleByProperty.h"
#include "mitkResliceMethodProperty.h"
#include "mitkRenderingManager.h"

#include "mitkImageCast.h"
#include "mitkShImage.h"
#include "mitkPlanarFigure.h"
#include "mitkFiberBundle.h"
#include "QmitkDataStorageComboBox.h"
#include "mitkPlanarFigureInteractor.h"
#include <mitkOdfImage.h>
#include <mitkTensorImage.h>
#include <mitkImage.h>
#include <mitkDiffusionPropertyHelper.h>
#include <mitkConnectomicsNetwork.h>
#include "usModuleRegistry.h"
#include <mitkPeakImage.h>
#include <mitkBaseRenderer.h>
#include "mitkPlaneGeometry.h"
#include <QmitkRenderWindow.h>
#include <itkFlipPeaksFilter.h>
#include <mitkImageToItk.h>
#include <mitkWorkbenchUtil.h>
#include "berryIWorkbenchWindow.h"
#include "berryIWorkbenchPage.h"
#include "berryISelectionService.h"
#include "berryConstants.h"
#include "berryPlatformUI.h"

#include "itkRGBAPixel.h"
#include <itkTractDensityImageFilter.h>

#include "qwidgetaction.h"
#include "qcolordialog.h"
#include <QRgb>
#include <itkMultiThreader.h>
#include <mitkClippingProperty.h>
#include <ciso646>

#define ROUND(a) ((a)>0 ? (int)((a)+0.5) : -(int)(0.5-(a)))

const std::string QmitkControlVisualizationPropertiesView::VIEW_ID = "org.mitk.views.controlvisualizationpropertiesview";

using namespace berry;

QmitkControlVisualizationPropertiesView::QmitkControlVisualizationPropertiesView()
  : QmitkAbstractView(),
    m_Controls(nullptr),
    m_CurrentSelection(nullptr),
    m_IconTexOFF(new QIcon(":/QmitkDiffusionImaging/texIntOFFIcon.png")),
    m_IconTexON(new QIcon(":/QmitkDiffusionImaging/texIntONIcon.png")),
    m_IconGlyOFF_T(new QIcon(":/QmitkDiffusionImaging/glyphsoff_T.png")),
    m_IconGlyON_T(new QIcon(":/QmitkDiffusionImaging/glyphson_T.png")),
    m_IconGlyOFF_C(new QIcon(":/QmitkDiffusionImaging/glyphsoff_C.png")),
    m_IconGlyON_C(new QIcon(":/QmitkDiffusionImaging/glyphson_C.png")),
    m_IconGlyOFF_S(new QIcon(":/QmitkDiffusionImaging/glyphsoff_S.png")),
    m_IconGlyON_S(new QIcon(":/QmitkDiffusionImaging/glyphson_S.png")),
    m_GlyIsOn_T(false),
    m_GlyIsOn_C(false),
    m_GlyIsOn_S(false),
    m_CurrentThickSlicesMode(1),
    m_CurrentThickSlicesNum(0),
    m_CurrentPickingNode(nullptr),
    m_ColorPropertyObserverTag(0),
    m_OpacityPropertyObserverTag(0)
{
  m_MyMenu = nullptr;
  auto numThread = itk::MultiThreader::GetGlobalMaximumNumberOfThreads();
  itk::MultiThreader::SetGlobalDefaultNumberOfThreads(numThread);
}


QmitkControlVisualizationPropertiesView::~QmitkControlVisualizationPropertiesView()
{

}

void QmitkControlVisualizationPropertiesView::SetTs(int currentThickSlicesMode, int num, std::string render_window)
{
  if (auto renderWindowPart = this->GetRenderWindowPart(mitk::WorkbenchUtil::IRenderWindowPartStrategy::OPEN))
  {
    mitk::BaseRenderer::Pointer renderer = renderWindowPart->GetQmitkRenderWindow(QString(render_window.c_str()))->GetRenderer();
    renderer->GetCurrentWorldPlaneGeometryNode()->SetProperty("reslice.thickslices.num", mitk::IntProperty::New(num));
    if(num>0)
    {
      renderer->GetCurrentWorldPlaneGeometryNode()->SetProperty("reslice.thickslices", mitk::ResliceMethodProperty::New(currentThickSlicesMode));
      renderer->GetCurrentWorldPlaneGeometryNode()->SetProperty("reslice.thickslices.showarea", mitk::BoolProperty::New(true));
    }
    else
    {
      renderer->GetCurrentWorldPlaneGeometryNode()->SetProperty("reslice.thickslices", mitk::ResliceMethodProperty::New(0));
      renderer->GetCurrentWorldPlaneGeometryNode()->SetProperty("reslice.thickslices.showarea", mitk::BoolProperty::New(false));
    }

    renderer->SendUpdateSlice();
    renderer->GetRenderingManager()->RequestUpdateAll();
  }
}

void QmitkControlVisualizationPropertiesView::OnThickSlicesModeSelected( QAction* action )
{
  m_CurrentThickSlicesMode = action->data().toInt();

  switch( m_CurrentThickSlicesMode )
  {
  case 0:
    return;

  case 1:
    this->m_Controls->m_TSMenu->setText("MIP");
    break;

  case 2:
    this->m_Controls->m_TSMenu->setText("SUM");
    break;

  case 3:
    this->m_Controls->m_TSMenu->setText("WEIGH");
    break;

  default:
    return;
  }

  SetTs(m_CurrentThickSlicesMode, m_CurrentThickSlicesNum, "axial");
  SetTs(m_CurrentThickSlicesMode, m_CurrentThickSlicesNum, "sagittal");
  SetTs(m_CurrentThickSlicesMode, m_CurrentThickSlicesNum, "coronal");
}


void QmitkControlVisualizationPropertiesView::OnTSNumChanged( int num )
{
  m_CurrentThickSlicesNum = num;
  SetTs(m_CurrentThickSlicesMode, m_CurrentThickSlicesNum, "axial");
  SetTs(m_CurrentThickSlicesMode, m_CurrentThickSlicesNum, "sagittal");
  SetTs(m_CurrentThickSlicesMode, m_CurrentThickSlicesNum, "coronal");
  m_TSLabel->setText(QString::number( num*2 + 1 ));
}


void QmitkControlVisualizationPropertiesView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkControlVisualizationPropertiesViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    // hide warning (ODFs in rotated planes)
    m_Controls->m_lblRotatedPlanesWarning->hide();

    m_MyMenu = new QMenu(parent);
    m_Controls->m_TSMenu->setMenu( m_MyMenu );

    QIcon iconFiberFade(":/QmitkDiffusionImaging/MapperEfx2D.png");
    m_Controls->m_FiberFading2D->setIcon(iconFiberFade);

    m_Controls->m_NormalizationFrame->setVisible(false);
    m_Controls->m_Crosshair->setVisible(false);

    mitk::IRenderWindowPart* renderWindow = this->GetRenderWindowPart();
    if (renderWindow)
    {
      m_SliceChangeListener.RenderWindowPartActivated(renderWindow);
      connect(&m_SliceChangeListener, SIGNAL(SliceChanged()), this, SLOT(OnSliceChanged()));
    }

    connect(m_Controls->m_SetColor1, SIGNAL(clicked()), this, SLOT(SetColor()));
    connect(m_Controls->m_SetColor2, SIGNAL(clicked()), this, SLOT(SetColor()));
  }
}

void QmitkControlVisualizationPropertiesView::SetColor()
{
  if(m_SelectedNode)
  {
    QColor c = QColorDialog::getColor();
    if (c.isValid())
    {
      float rgb[3];
      rgb[0] = static_cast<float>(c.redF());
      rgb[1] = static_cast<float>(c.greenF());
      rgb[2] = static_cast<float>(c.blueF());
      m_SelectedNode->SetColor(rgb);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}

void QmitkControlVisualizationPropertiesView::SetFocus()
{
  m_Controls->m_TSMenu->setFocus();
}


void QmitkControlVisualizationPropertiesView::SliceRotation(const itk::EventObject&)
{
  // test if plane rotated
  if( m_GlyIsOn_T || m_GlyIsOn_C || m_GlyIsOn_S )
  {
    if( this->IsPlaneRotated() )
    {
      // show label
      m_Controls->m_lblRotatedPlanesWarning->show();
    }
    else
    {
      //hide label
      m_Controls->m_lblRotatedPlanesWarning->hide();
    }
  }
}


void QmitkControlVisualizationPropertiesView::NodeRemoved(const mitk::DataNode* /*node*/)
{
}


#include <mitkMessage.h>
void QmitkControlVisualizationPropertiesView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( static_cast<QObject*>(m_Controls->m_VisibleOdfsON_T), SIGNAL(clicked()), this, SLOT(VisibleOdfsON_T()) );
    connect( static_cast<QObject*>(m_Controls->m_VisibleOdfsON_S), SIGNAL(clicked()), this, SLOT(VisibleOdfsON_S()) );
    connect( static_cast<QObject*>(m_Controls->m_VisibleOdfsON_C), SIGNAL(clicked()), this, SLOT(VisibleOdfsON_C()) );
    connect( static_cast<QObject*>(m_Controls->m_ShowMaxNumber), SIGNAL(editingFinished()), this, SLOT(ShowMaxNumberChanged()) );
    connect( static_cast<QObject*>(m_Controls->m_NormalizationDropdown), SIGNAL(currentIndexChanged(int)), this, SLOT(NormalizationDropdownChanged(int)) );
    connect( static_cast<QObject*>(m_Controls->m_ScalingFactor), SIGNAL(valueChanged(double)), this, SLOT(ScalingFactorChanged(double)) );
    connect( static_cast<QObject*>(m_Controls->m_AdditionalScaling), SIGNAL(currentIndexChanged(int)), this, SLOT(AdditionalScaling(int)) );
    connect(static_cast<QObject*>(m_Controls->m_ResetColoring), SIGNAL(clicked()), static_cast<QObject*>(this), SLOT(ResetColoring()));
    connect(static_cast<QObject*>(m_Controls->m_ResetColoring2), SIGNAL(clicked()), static_cast<QObject*>(this), SLOT(ResetColoring()));
    connect(static_cast<QObject*>(m_Controls->m_FiberFading2D), SIGNAL(clicked()), static_cast<QObject*>(this), SLOT( Fiber2DfadingEFX() ) );
    connect(static_cast<QObject*>(m_Controls->m_FiberThicknessSlider), SIGNAL(sliderReleased()), static_cast<QObject*>(this), SLOT( FiberSlicingThickness2D() ) );
    connect(static_cast<QObject*>(m_Controls->m_FiberThicknessSlider), SIGNAL(valueChanged(int)), static_cast<QObject*>(this), SLOT( FiberSlicingUpdateLabel(int) ));
    connect(static_cast<QObject*>(m_Controls->m_Crosshair), SIGNAL(clicked()), static_cast<QObject*>(this), SLOT(SetInteractor()));
    connect(static_cast<QObject*>(m_Controls->m_LineWidth), SIGNAL(editingFinished()), static_cast<QObject*>(this), SLOT(LineWidthChanged()));
    connect(static_cast<QObject*>(m_Controls->m_TubeWidth), SIGNAL(editingFinished()), static_cast<QObject*>(this), SLOT(TubeRadiusChanged()));
    connect(static_cast<QObject*>(m_Controls->m_RibbonWidth), SIGNAL(editingFinished()), static_cast<QObject*>(this), SLOT(RibbonWidthChanged()));
    connect( static_cast<QObject*>(m_Controls->m_OdfColorBox), SIGNAL(currentIndexChanged(int)), static_cast<QObject*>(this), SLOT(OnColourisationModeChanged() ) );
    connect(static_cast<QObject*>(m_Controls->m_Clip0), SIGNAL(toggled(bool)), static_cast<QObject*>(this), SLOT(Toggle3DClipping(bool)));
    connect(static_cast<QObject*>(m_Controls->m_Clip1), SIGNAL(toggled(bool)), static_cast<QObject*>(this), SLOT(Toggle3DClipping(bool)));
    connect(static_cast<QObject*>(m_Controls->m_Clip2), SIGNAL(toggled(bool)), static_cast<QObject*>(this), SLOT(Toggle3DClipping(bool)));
    connect(static_cast<QObject*>(m_Controls->m_Clip3), SIGNAL(toggled(bool)), static_cast<QObject*>(this), SLOT(Toggle3DClipping(bool)));
    connect(static_cast<QObject*>(m_Controls->m_FlipClipBox), SIGNAL(stateChanged(int)), static_cast<QObject*>(this), SLOT(Toggle3DClipping()));
    connect(static_cast<QObject*>(m_Controls->m_Enable3dPeaks), SIGNAL(stateChanged(int)), static_cast<QObject*>(this), SLOT(Toggle3DPeaks()));
    connect(static_cast<QObject*>(m_Controls->m_FlipPeaksButton), SIGNAL(clicked()), static_cast<QObject*>(this), SLOT(FlipPeaks()));

    m_Controls->m_BundleControlsFrame->setVisible(false);
    m_Controls->m_ImageControlsFrame->setVisible(false);
    m_Controls->m_PeakImageFrame->setVisible(false);
    m_Controls->m_lblRotatedPlanesWarning->setVisible(false);
    m_Controls->m_3DClippingBox->setVisible(false);
  }
}


// set diffusion image channel to b0 volume
void QmitkControlVisualizationPropertiesView::NodeAdded(const mitk::DataNode *node)
{
  mitk::DataNode* notConst = const_cast<mitk::DataNode*>(node);

  bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage( dynamic_cast<mitk::Image *>(node->GetData())) );

  if (isDiffusionImage)
  {
    mitk::Image::Pointer dimg = dynamic_cast<mitk::Image*>(notConst->GetData());

    // if there is no b0 image in the dataset, the GetB0Indices() returns a vector of size 0
    // and hence we cannot set the Property directly to .front()
    int displayChannelPropertyValue = 0;
    mitk::DiffusionPropertyHelper::BValueMapType map = mitk::DiffusionPropertyHelper::GetBValueMap(dimg);

    if( map[0].size() > 0) { displayChannelPropertyValue = map[0].front(); }

    notConst->SetIntProperty("DisplayChannel", displayChannelPropertyValue );
  }
}


/* OnSelectionChanged is registered to SelectionService, therefore no need to
implement SelectionService Listener explicitly */
void QmitkControlVisualizationPropertiesView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/, const QList<mitk::DataNode::Pointer>& nodes)
{
  m_Controls->m_BundleControlsFrame->setVisible(false);
  m_Controls->m_ImageControlsFrame->setVisible(false);
  m_Controls->m_PeakImageFrame->setVisible(false);
  m_Controls->m_3DClippingBox->setVisible(false);
  m_Controls->m_FlipClipBox->setVisible(false);
  m_Controls->m_Enable3dPeaks->setVisible(false);

  if (nodes.size()>1) // only do stuff if one node is selected
    return;

  m_Controls->m_NumberGlyphsFrame->setVisible(false);
  m_Controls->m_GlyphFrame->setVisible(false);
  m_Controls->m_TSMenu->setVisible(false);

  m_SelectedNode = nullptr;

  int numOdfImages = 0;
  for (mitk::DataNode::Pointer node: nodes)
  {
    if(node.IsNull())
      continue;

    mitk::BaseData* nodeData = node->GetData();
    if(nodeData == nullptr)
      continue;

    m_SelectedNode = node;

    if (dynamic_cast<mitk::PeakImage*>(nodeData))
    {
      m_Controls->m_PeakImageFrame->setVisible(true);

      if (m_Color.IsNotNull())
        m_Color->RemoveObserver(m_ColorPropertyObserverTag);

      itk::ReceptorMemberCommand<QmitkControlVisualizationPropertiesView>::Pointer command = itk::ReceptorMemberCommand<QmitkControlVisualizationPropertiesView>::New();
      command->SetCallbackFunction( this, &QmitkControlVisualizationPropertiesView::SetCustomColor );
      m_Color = dynamic_cast<mitk::ColorProperty*>(node->GetProperty("color", nullptr));
      if (m_Color.IsNotNull())
        m_ColorPropertyObserverTag = m_Color->AddObserver( itk::ModifiedEvent(), command );

      int ClippingPlaneId = -1;
      m_SelectedNode->GetPropertyValue("3DClippingPlaneId",ClippingPlaneId);
      switch(ClippingPlaneId)
      {
      case 0:
        m_Controls->m_Clip0->setChecked(1);
        break;
      case 1:
        m_Controls->m_Clip1->setChecked(1);
        break;
      case 2:
        m_Controls->m_Clip2->setChecked(1);
        break;
      case 3:
        m_Controls->m_Clip3->setChecked(1);
        break;
      default :
        m_Controls->m_Clip0->setChecked(1);
      }

      m_Controls->m_Enable3dPeaks->setVisible(true);
      m_Controls->m_3DClippingBox->setVisible(true);
    }
    else if (dynamic_cast<mitk::FiberBundle*>(nodeData))
    {
      int ClippingPlaneId = -1;
      m_SelectedNode->GetPropertyValue("3DClippingPlaneId",ClippingPlaneId);
      switch(ClippingPlaneId)
      {
      case 0:
        m_Controls->m_Clip0->setChecked(1);
        break;
      case 1:
        m_Controls->m_Clip1->setChecked(1);
        break;
      case 2:
        m_Controls->m_Clip2->setChecked(1);
        break;
      case 3:
        m_Controls->m_Clip3->setChecked(1);
        break;
      default :
        m_Controls->m_Clip0->setChecked(1);
      }

      // handle fiber property observers
      if (m_Color.IsNotNull())
        m_Color->RemoveObserver(m_ColorPropertyObserverTag);
      itk::ReceptorMemberCommand<QmitkControlVisualizationPropertiesView>::Pointer command = itk::ReceptorMemberCommand<QmitkControlVisualizationPropertiesView>::New();
      command->SetCallbackFunction( this, &QmitkControlVisualizationPropertiesView::SetCustomColor );
      m_Color = dynamic_cast<mitk::ColorProperty*>(node->GetProperty("color", nullptr));
      if (m_Color.IsNotNull())
        m_ColorPropertyObserverTag = m_Color->AddObserver( itk::ModifiedEvent(), command );

      m_Controls->m_FlipClipBox->setVisible(true);
      m_Controls->m_3DClippingBox->setVisible(true);
      m_Controls->m_BundleControlsFrame->setVisible(true);

      if(m_CurrentPickingNode != 0 && node.GetPointer() != m_CurrentPickingNode)
      { m_Controls->m_Crosshair->setEnabled(false); }
      else
      { m_Controls->m_Crosshair->setEnabled(true); }

      int width;
      node->GetIntProperty("shape.linewidth", width);
      m_Controls->m_LineWidth->setValue(width);

      float radius;
      node->GetFloatProperty("shape.tuberadius", radius);
      m_Controls->m_TubeWidth->setValue(radius);

      float range;
      node->GetFloatProperty("Fiber2DSliceThickness",range);
      mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(node->GetData());
      mitk::BaseGeometry::Pointer geo = fib->GetGeometry();
      mitk::ScalarType max = geo->GetExtentInMM(0);
      max = std::max(max, geo->GetExtentInMM(1));
      max = std::max(max, geo->GetExtentInMM(2));

      m_Controls->m_FiberThicknessSlider->setMaximum(max * 10);
      m_Controls->m_FiberThicknessSlider->setValue(range * 10);
    }
    else if(dynamic_cast<mitk::OdfImage*>(nodeData) || dynamic_cast<mitk::TensorImage*>(nodeData) || dynamic_cast<mitk::ShImage*>(nodeData))
    {
      m_Controls->m_ImageControlsFrame->setVisible(true);
      m_Controls->m_NumberGlyphsFrame->setVisible(true);
      m_Controls->m_GlyphFrame->setVisible(true);
      m_Controls->m_NormalizationFrame->setVisible(true);

      if(m_NodeUsedForOdfVisualization.IsNotNull())
      {
        m_NodeUsedForOdfVisualization->SetBoolProperty("VisibleOdfs_S", false);
        m_NodeUsedForOdfVisualization->SetBoolProperty("VisibleOdfs_C", false);
        m_NodeUsedForOdfVisualization->SetBoolProperty("VisibleOdfs_T", false);
      }
      m_NodeUsedForOdfVisualization = node;
      m_NodeUsedForOdfVisualization->SetBoolProperty("VisibleOdfs_S", m_GlyIsOn_S);
      m_NodeUsedForOdfVisualization->SetBoolProperty("VisibleOdfs_C", m_GlyIsOn_C);
      m_NodeUsedForOdfVisualization->SetBoolProperty("VisibleOdfs_T", m_GlyIsOn_T);

      if (dynamic_cast<mitk::TensorImage*>(nodeData))
      {
        m_Controls->m_NormalizationDropdown->setVisible(false);
        m_Controls->m_NormalizationLabel->setVisible(false);
      }
      else
      {
        m_Controls->m_NormalizationDropdown->setVisible(true);
        m_Controls->m_NormalizationLabel->setVisible(true);
      }


      int val;
      node->GetIntProperty("ShowMaxNumber", val);
      m_Controls->m_ShowMaxNumber->setValue(val);

      m_Controls->m_NormalizationDropdown->setCurrentIndex(dynamic_cast<mitk::EnumerationProperty*>(node->GetProperty("Normalization"))->GetValueAsId());

      float fval;
      node->GetFloatProperty("Scaling",fval);
      m_Controls->m_ScalingFactor->setValue(fval);

      m_Controls->m_AdditionalScaling->setCurrentIndex(dynamic_cast<mitk::EnumerationProperty*>(node->GetProperty("ScaleBy"))->GetValueAsId());

      bool switchTensorViewValue = false;
      node->GetBoolProperty( "DiffusionCore.Rendering.OdfVtkMapper.SwitchTensorView", switchTensorViewValue );

      bool colourisationModeBit = false;
      node->GetBoolProperty("DiffusionCore.Rendering.OdfVtkMapper.ColourisationModeBit", colourisationModeBit );
      m_Controls->m_OdfColorBox->setCurrentIndex(colourisationModeBit);

      numOdfImages++;
    }
    else if(dynamic_cast<mitk::PlanarFigure*>(nodeData))
    {
      PlanarFigureFocus();
    }
    else if( dynamic_cast<mitk::Image*>(nodeData) )
    {
      m_Controls->m_ImageControlsFrame->setVisible(true);
      m_Controls->m_TSMenu->setVisible(true);
    }
  }

  if( nodes.empty() ) { return; }

  mitk::DataNode::Pointer node = nodes.at(0);

  if( node.IsNull() ) { return; }

  QMenu *myMenu = m_MyMenu;
  myMenu->clear();

  QActionGroup* thickSlicesActionGroup = new QActionGroup(myMenu);
  thickSlicesActionGroup->setExclusive(true);

  int currentTSMode = 0;
  {
    mitk::ResliceMethodProperty::Pointer m = dynamic_cast<mitk::ResliceMethodProperty*>(node->GetProperty( "reslice.thickslices" ));
    if( m.IsNotNull() )
      currentTSMode = m->GetValueAsId();
  }

  int maxTS = 30;

  for (auto node: nodes)
  {
    mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());
    if (image)
    {
      int size = std::max(image->GetDimension(0), std::max(image->GetDimension(1), image->GetDimension(2)));

      if (size>maxTS) { maxTS=size; }
    }
  }
  maxTS /= 2;

  int currentNum = 0;
  {
    mitk::IntProperty::Pointer m = dynamic_cast<mitk::IntProperty*>(node->GetProperty( "reslice.thickslices.num" ));
    if( m.IsNotNull() )
    {
      currentNum = m->GetValue();
      if(currentNum < 0) { currentNum = 0; }
      if(currentNum > maxTS) { currentNum = maxTS; }
    }
  }

  if(currentTSMode==0) { currentNum=0; }

  QSlider *m_TSSlider = new QSlider(myMenu);
  m_TSSlider->setMinimum(0);
  m_TSSlider->setMaximum(maxTS-1);
  m_TSSlider->setValue(currentNum);

  m_TSSlider->setOrientation(Qt::Horizontal);

  connect( m_TSSlider, SIGNAL( valueChanged(int) ), this, SLOT( OnTSNumChanged(int) ) );

  QHBoxLayout* _TSLayout = new QHBoxLayout;
  _TSLayout->setContentsMargins(4,4,4,4);
  _TSLayout->addWidget(m_TSSlider);
  _TSLayout->addWidget(m_TSLabel=new QLabel(QString::number(currentNum*2+1),myMenu));

  QWidget* _TSWidget = new QWidget;
  _TSWidget->setLayout(_TSLayout);

  QActionGroup* thickSliceModeActionGroup = new QActionGroup(myMenu);
  thickSliceModeActionGroup->setExclusive(true);

  QWidgetAction *m_TSSliderAction = new QWidgetAction(myMenu);
  m_TSSliderAction->setDefaultWidget(_TSWidget);
  myMenu->addAction(m_TSSliderAction);

  QAction* mipThickSlicesAction = new QAction(myMenu);
  mipThickSlicesAction->setActionGroup(thickSliceModeActionGroup);
  mipThickSlicesAction->setText("MIP (max. intensity proj.)");
  mipThickSlicesAction->setCheckable(true);
  mipThickSlicesAction->setChecked(m_CurrentThickSlicesMode==1);
  mipThickSlicesAction->setData(1);
  myMenu->addAction( mipThickSlicesAction );

  QAction* sumThickSlicesAction = new QAction(myMenu);
  sumThickSlicesAction->setActionGroup(thickSliceModeActionGroup);
  sumThickSlicesAction->setText("SUM (sum intensity proj.)");
  sumThickSlicesAction->setCheckable(true);
  sumThickSlicesAction->setChecked(m_CurrentThickSlicesMode==2);
  sumThickSlicesAction->setData(2);
  myMenu->addAction( sumThickSlicesAction );

  QAction* weightedThickSlicesAction = new QAction(myMenu);
  weightedThickSlicesAction->setActionGroup(thickSliceModeActionGroup);
  weightedThickSlicesAction->setText("WEIGHTED (gaussian proj.)");
  weightedThickSlicesAction->setCheckable(true);
  weightedThickSlicesAction->setChecked(m_CurrentThickSlicesMode==3);
  weightedThickSlicesAction->setData(3);
  myMenu->addAction( weightedThickSlicesAction );

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  connect( thickSliceModeActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(OnThickSlicesModeSelected(QAction*)) );
}


void QmitkControlVisualizationPropertiesView::VisibleOdfsON_S()
{
  m_GlyIsOn_S = m_Controls->m_VisibleOdfsON_S->isChecked();
  if (m_NodeUsedForOdfVisualization.IsNull())
  {
    MITK_WARN << "ODF visualization activated but m_NodeUsedForOdfVisualization is nullptr";
    return;
  }
  m_NodeUsedForOdfVisualization->SetBoolProperty("VisibleOdfs_S", m_GlyIsOn_S);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkControlVisualizationPropertiesView::Visible()
{
  mitk::IRenderWindowPart* renderWindow = this->GetRenderWindowPart();
  if (renderWindow)
  {
    m_SliceChangeListener.RenderWindowPartActivated(renderWindow);
    connect(&m_SliceChangeListener, SIGNAL(SliceChanged()), this, SLOT(OnSliceChanged()));
  }
}

void QmitkControlVisualizationPropertiesView::Hidden()
{

}

void QmitkControlVisualizationPropertiesView::Activated()
{
  mitk::IRenderWindowPart* renderWindow = this->GetRenderWindowPart();
  if (renderWindow)
  {
    m_SliceChangeListener.RenderWindowPartActivated(renderWindow);
    connect(&m_SliceChangeListener, SIGNAL(SliceChanged()), this, SLOT(OnSliceChanged()));
  }
}

void QmitkControlVisualizationPropertiesView::Deactivated()
{
}

void QmitkControlVisualizationPropertiesView::FlipPeaks()
{
  if (m_SelectedNode.IsNull() || dynamic_cast<mitk::PeakImage*>(m_SelectedNode->GetData())==nullptr)
    return;

  std::string name = m_SelectedNode->GetName();

  mitk::Image::Pointer image = dynamic_cast<mitk::PeakImage*>(m_SelectedNode->GetData());

  typedef mitk::ImageToItk< mitk::PeakImage::ItkPeakImageType > CasterType;
  CasterType::Pointer caster = CasterType::New();
  caster->SetInput(image);
  caster->Update();
  mitk::PeakImage::ItkPeakImageType::Pointer itkImg = caster->GetOutput();

  itk::FlipPeaksFilter< float >::Pointer flipper = itk::FlipPeaksFilter< float >::New();
  flipper->SetInput(itkImg);
  flipper->SetFlipX(m_Controls->m_FlipPeaksX->isChecked());
  flipper->SetFlipY(m_Controls->m_FlipPeaksY->isChecked());
  flipper->SetFlipZ(m_Controls->m_FlipPeaksZ->isChecked());
  flipper->Update();

  mitk::Image::Pointer resultImage = dynamic_cast<mitk::Image*>(mitk::PeakImage::New().GetPointer());
  mitk::CastToMitkImage(flipper->GetOutput(), resultImage);
  resultImage->SetVolume(flipper->GetOutput()->GetBufferPointer());
  m_SelectedNode->SetData(resultImage);
  m_SelectedNode->SetName(name);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkControlVisualizationPropertiesView::Toggle3DPeaks()
{
  if (m_SelectedNode.IsNull() || dynamic_cast<mitk::PeakImage*>(m_SelectedNode->GetData())==nullptr)
    return;

  bool enabled = false;
  m_SelectedNode->GetBoolProperty("Enable3DPeaks", enabled);
  m_SelectedNode->SetBoolProperty( "Enable3DPeaks", !enabled );
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkControlVisualizationPropertiesView::Toggle3DClipping(bool enabled)
{
  if (!enabled || m_SelectedNode.IsNull() || (dynamic_cast<mitk::FiberBundle*>(m_SelectedNode->GetData())==nullptr && dynamic_cast<mitk::PeakImage*>(m_SelectedNode->GetData())==nullptr))
    return;

  m_SelectedNode->SetBoolProperty( "3DClippingPlaneFlip", m_Controls->m_FlipClipBox->isChecked() );

  if (m_Controls->m_Clip0->isChecked())
  {
    m_SelectedNode->SetIntProperty( "3DClippingPlaneId", 0 );
    Set3DClippingPlane(true, m_SelectedNode, "");
  }
  else if (m_Controls->m_Clip1->isChecked())
  {
    m_SelectedNode->SetIntProperty( "3DClippingPlaneId", 1 );
    Set3DClippingPlane(false, m_SelectedNode, "axial");
  }
  else if (m_Controls->m_Clip2->isChecked())
  {
    m_SelectedNode->SetIntProperty( "3DClippingPlaneId", 2 );
    Set3DClippingPlane(false, m_SelectedNode, "sagittal");
  }
  else if (m_Controls->m_Clip3->isChecked())
  {
    m_SelectedNode->SetIntProperty( "3DClippingPlaneId", 3 );
    Set3DClippingPlane(false, m_SelectedNode, "coronal");
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkControlVisualizationPropertiesView::OnSliceChanged()
{
  mitk::DataStorage::SetOfObjects::ConstPointer nodes = this->GetDataStorage()->GetAll();
  for (unsigned int i=0; i<nodes->Size(); ++i)
  {
    mitk::DataNode::Pointer node = nodes->GetElement(i);
    int plane_id = -1;
    node->GetIntProperty("3DClippingPlaneId", plane_id);

    if (plane_id==1)
      Set3DClippingPlane(false, node, "axial");
    else if (plane_id==2)
      Set3DClippingPlane(false, node, "sagittal");
    else if (plane_id==3)
      Set3DClippingPlane(false, node, "coronal");
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkControlVisualizationPropertiesView::Set3DClippingPlane(bool disable, mitk::DataNode* node, std::string plane)
{
  mitk::IRenderWindowPart* renderWindow = this->GetRenderWindowPart();
  if (renderWindow && node && (dynamic_cast<mitk::FiberBundle*>(node->GetData()) || dynamic_cast<mitk::PeakImage*>(node->GetData())))
  {
    mitk::Vector3D planeNormal; planeNormal.Fill(0.0);
    mitk::Point3D planeOrigin; planeOrigin.Fill(0.0);
    if (!disable)
    {
      mitk::SliceNavigationController* slicer = renderWindow->GetQmitkRenderWindow(QString(plane.c_str()))->GetSliceNavigationController();
      mitk::PlaneGeometry::ConstPointer planeGeo = slicer->GetCurrentPlaneGeometry();
      planeOrigin = this->GetRenderWindowPart()->GetSelectedPosition();
      planeNormal = planeGeo->GetNormal();
    }
    node->SetProperty( "3DClipping", mitk::ClippingProperty::New( planeOrigin, planeNormal ) );
    if (dynamic_cast<mitk::FiberBundle*>(node->GetData()))
      dynamic_cast<mitk::FiberBundle*>(node->GetData())->RequestUpdate();
    else
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkControlVisualizationPropertiesView::VisibleOdfsON_T()
{
  m_GlyIsOn_T = m_Controls->m_VisibleOdfsON_T->isChecked();
  if (m_NodeUsedForOdfVisualization.IsNull())
  {
    MITK_WARN << "ODF visualization activated but m_NodeUsedForOdfVisualization is nullptr";
    return;
  }
  m_NodeUsedForOdfVisualization->SetBoolProperty("VisibleOdfs_T", m_GlyIsOn_T);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void QmitkControlVisualizationPropertiesView::VisibleOdfsON_C()
{
  m_GlyIsOn_C = m_Controls->m_VisibleOdfsON_C->isChecked();
  if (m_NodeUsedForOdfVisualization.IsNull())
  {
    MITK_WARN << "ODF visualization activated but m_NodeUsedForOdfVisualization is nullptr";
    return;
  }
  m_NodeUsedForOdfVisualization->SetBoolProperty("VisibleOdfs_C", m_GlyIsOn_C);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


bool QmitkControlVisualizationPropertiesView::IsPlaneRotated()
{
  mitk::Image* currentImage = dynamic_cast<mitk::Image* >( m_NodeUsedForOdfVisualization->GetData() );
  if( currentImage == nullptr )
  {
    MITK_ERROR << " Casting problems. Returning false";
    return false;
  }

  mitk::Vector3D imageNormal0 = currentImage->GetSlicedGeometry()->GetAxisVector(0);
  mitk::Vector3D imageNormal1 = currentImage->GetSlicedGeometry()->GetAxisVector(1);
  mitk::Vector3D imageNormal2 = currentImage->GetSlicedGeometry()->GetAxisVector(2);
  imageNormal0.Normalize();
  imageNormal1.Normalize();
  imageNormal2.Normalize();

  auto renderWindowPart = this->GetRenderWindowPart();

  double eps = 0.000001;
  // for all 2D renderwindows of the render window part check alignment
  {
    mitk::PlaneGeometry::ConstPointer displayPlane
        = dynamic_cast<const mitk::PlaneGeometry*>
        ( renderWindowPart->GetQmitkRenderWindow("axial")->GetRenderer()->GetCurrentWorldPlaneGeometry() );

    if (displayPlane.IsNull()) { return false; }

    mitk::Vector3D normal = displayPlane->GetNormal();
    normal.Normalize();
    int test = 0;
    if( fabs(fabs(dot_product(normal.GetVnlVector(),imageNormal0.GetVnlVector()))-1) > eps ) { test++; }

    if( fabs(fabs(dot_product(normal.GetVnlVector(),imageNormal1.GetVnlVector()))-1) > eps ) { test++; }

    if( fabs(fabs(dot_product(normal.GetVnlVector(),imageNormal2.GetVnlVector()))-1) > eps ) { test++; }

    if (test==3) { return true; }
  }
  {
    mitk::PlaneGeometry::ConstPointer displayPlane
        = dynamic_cast<const mitk::PlaneGeometry*>
        ( renderWindowPart->GetQmitkRenderWindow("sagittal")->GetRenderer()->GetCurrentWorldPlaneGeometry() );

    if (displayPlane.IsNull()) { return false; }

    mitk::Vector3D normal = displayPlane->GetNormal();
    normal.Normalize();
    int test = 0;
    if( fabs(fabs(dot_product(normal.GetVnlVector(),imageNormal0.GetVnlVector()))-1) > eps ) { test++; }

    if( fabs(fabs(dot_product(normal.GetVnlVector(),imageNormal1.GetVnlVector()))-1) > eps ) { test++; }

    if( fabs(fabs(dot_product(normal.GetVnlVector(),imageNormal2.GetVnlVector()))-1) > eps ) { test++; }

    if (test==3) { return true; }
  }
  {
    mitk::PlaneGeometry::ConstPointer displayPlane
        = dynamic_cast<const mitk::PlaneGeometry*>
        ( renderWindowPart->GetQmitkRenderWindow("coronal")->GetRenderer()->GetCurrentWorldPlaneGeometry() );

    if (displayPlane.IsNull()) { return false; }

    mitk::Vector3D normal = displayPlane->GetNormal();
    normal.Normalize();
    int test = 0;
    if( fabs(fabs(dot_product(normal.GetVnlVector(),imageNormal0.GetVnlVector()))-1) > eps ) { test++; }

    if( fabs(fabs(dot_product(normal.GetVnlVector(),imageNormal1.GetVnlVector()))-1) > eps ) { test++; }

    if( fabs(fabs(dot_product(normal.GetVnlVector(),imageNormal2.GetVnlVector()))-1) > eps ) { test++; }

    if (test==3) { return true; }
  }

  return false;
}


void QmitkControlVisualizationPropertiesView::ShowMaxNumberChanged()
{
  int maxNr = m_Controls->m_ShowMaxNumber->value();
  if ( maxNr < 1 )
  {
    m_Controls->m_ShowMaxNumber->setValue( 1 );
    maxNr = 1;
  }

  if ( dynamic_cast<mitk::OdfImage*>(m_SelectedNode->GetData())
       || dynamic_cast<mitk::TensorImage*>(m_SelectedNode->GetData())
       || dynamic_cast<mitk::ShImage*>(m_SelectedNode->GetData()) )
  {
    m_SelectedNode->SetIntProperty("ShowMaxNumber", maxNr);
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void QmitkControlVisualizationPropertiesView::NormalizationDropdownChanged(int normDropdown)
{
  typedef mitk::OdfNormalizationMethodProperty PropType;
  PropType::Pointer normMeth = PropType::New();

  switch(normDropdown)
  {
  case 0:
    normMeth->SetNormalizationToMinMax();
    break;
  case 1:
    normMeth->SetNormalizationToMax();
    break;
  case 2:
    normMeth->SetNormalizationToNone();
    break;
  case 3:
    normMeth->SetNormalizationToGlobalMax();
    break;
  default:
    normMeth->SetNormalizationToMinMax();
  }

  if ( dynamic_cast<mitk::OdfImage*>(m_SelectedNode->GetData())
       || dynamic_cast<mitk::TensorImage*>(m_SelectedNode->GetData())
       || dynamic_cast<mitk::ShImage*>(m_SelectedNode->GetData()) )
  {
    m_SelectedNode->SetProperty("Normalization", normMeth.GetPointer());
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void QmitkControlVisualizationPropertiesView::ScalingFactorChanged(double scalingFactor)
{
  if ( dynamic_cast<mitk::OdfImage*>(m_SelectedNode->GetData())
       || dynamic_cast<mitk::TensorImage*>(m_SelectedNode->GetData())
       || dynamic_cast<mitk::ShImage*>(m_SelectedNode->GetData()) )
  {
    m_SelectedNode->SetFloatProperty("Scaling", scalingFactor);
  }

  if (auto renderWindowPart = this->GetRenderWindowPart())
  {
    renderWindowPart->RequestUpdate();
  }
}


void QmitkControlVisualizationPropertiesView::AdditionalScaling(int additionalScaling)
{

  typedef mitk::OdfScaleByProperty PropType;
  PropType::Pointer scaleBy = PropType::New();

  switch(additionalScaling)
  {
  case 0:
    scaleBy->SetScaleByNothing();
    break;
  case 1:
    scaleBy->SetScaleByGFA();
    //m_Controls->params_frame->setVisible(true);
    break;
  default:
    scaleBy->SetScaleByNothing();
  }

  if ( dynamic_cast<mitk::OdfImage*>(m_SelectedNode->GetData())
       || dynamic_cast<mitk::TensorImage*>(m_SelectedNode->GetData())
       || dynamic_cast<mitk::ShImage*>(m_SelectedNode->GetData()) )
  {
    m_SelectedNode->SetProperty("ScaleBy", scaleBy.GetPointer());
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkControlVisualizationPropertiesView::Fiber2DfadingEFX()
{
  if (m_SelectedNode && dynamic_cast<mitk::FiberBundle*>(m_SelectedNode->GetData()) )
  {
    bool currentMode;
    m_SelectedNode->GetBoolProperty("Fiber2DfadeEFX", currentMode);
    m_SelectedNode->SetProperty("Fiber2DfadeEFX", mitk::BoolProperty::New(!currentMode));
    dynamic_cast<mitk::FiberBundle*>(m_SelectedNode->GetData())->RequestUpdate2D();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}


void QmitkControlVisualizationPropertiesView::FiberSlicingThickness2D()
{
  if (m_SelectedNode && dynamic_cast<mitk::FiberBundle*>(m_SelectedNode->GetData()))
  {
    float fibThickness = m_Controls->m_FiberThicknessSlider->value() * 0.1;
    float currentThickness = 0;
    m_SelectedNode->GetFloatProperty("Fiber2DSliceThickness", currentThickness);
    if ( fabs(fibThickness-currentThickness) < 0.001 )
    {
      return;
    }

    m_SelectedNode->SetProperty("Fiber2DSliceThickness", mitk::FloatProperty::New(fibThickness));
    dynamic_cast<mitk::FiberBundle*>(m_SelectedNode->GetData())->RequestUpdate2D();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}


void QmitkControlVisualizationPropertiesView::FiberSlicingUpdateLabel(int value)
{
  QString label = "Range %1 mm";
  label = label.arg(value * 0.1);
  m_Controls->label_range->setText(label);
  FiberSlicingThickness2D();
}

void QmitkControlVisualizationPropertiesView::SetCustomColor(const itk::EventObject& /*e*/)
{
  if(m_SelectedNode && dynamic_cast<mitk::FiberBundle*>(m_SelectedNode->GetData()))
  {
    float color[3];
    m_SelectedNode->GetColor(color);
    mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(m_SelectedNode->GetData());
    fib->SetFiberColors(color[0]*255, color[1]*255, color[2]*255);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  else if (m_SelectedNode && dynamic_cast<mitk::PeakImage*>(m_SelectedNode->GetData()))
  {
    float color[3];
    m_SelectedNode->GetColor(color);
    mitk::PeakImage::Pointer img = dynamic_cast<mitk::PeakImage*>(m_SelectedNode->GetData());
    img->SetCustomColor(color[0]*255, color[1]*255, color[2]*255);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}


void QmitkControlVisualizationPropertiesView::ResetColoring()
{
  if(m_SelectedNode && dynamic_cast<mitk::FiberBundle*>(m_SelectedNode->GetData()))
  {
    mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(m_SelectedNode->GetData());
    fib->ColorFibersByOrientation();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  else if(m_SelectedNode && dynamic_cast<mitk::PeakImage*>(m_SelectedNode->GetData()))
  {
    mitk::PeakImage::Pointer fib = dynamic_cast<mitk::PeakImage*>(m_SelectedNode->GetData());
    fib->ColorByOrientation();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}


void QmitkControlVisualizationPropertiesView::PlanarFigureFocus()
{
  if(m_SelectedNode)
  {
    mitk::PlanarFigure* _PlanarFigure = 0;
    _PlanarFigure = dynamic_cast<mitk::PlanarFigure*> (m_SelectedNode->GetData());

    if (_PlanarFigure && _PlanarFigure->GetPlaneGeometry())
    {

      QmitkRenderWindow* selectedRenderWindow = 0;
      bool PlanarFigureInitializedWindow = false;

      auto renderWindowPart = this->GetRenderWindowPart(mitk::WorkbenchUtil::IRenderWindowPartStrategy::OPEN);

      QmitkRenderWindow* axialRenderWindow =
          renderWindowPart->GetQmitkRenderWindow("axial");

      if (m_SelectedNode->GetBoolProperty("PlanarFigureInitializedWindow",
                                          PlanarFigureInitializedWindow, axialRenderWindow->GetRenderer()))
      {
        selectedRenderWindow = axialRenderWindow;
      }

      QmitkRenderWindow* sagittalRenderWindow =
          renderWindowPart->GetQmitkRenderWindow("sagittal");

      if (!selectedRenderWindow && m_SelectedNode->GetBoolProperty(
            "PlanarFigureInitializedWindow", PlanarFigureInitializedWindow,
            sagittalRenderWindow->GetRenderer()))
      {
        selectedRenderWindow = sagittalRenderWindow;
      }

      QmitkRenderWindow* coronalRenderWindow =
          renderWindowPart->GetQmitkRenderWindow("coronal");

      if (!selectedRenderWindow && m_SelectedNode->GetBoolProperty(
            "PlanarFigureInitializedWindow", PlanarFigureInitializedWindow,
            coronalRenderWindow->GetRenderer()))
      {
        selectedRenderWindow = coronalRenderWindow;
      }

      QmitkRenderWindow* _3DRenderWindow =
          renderWindowPart->GetQmitkRenderWindow("3d");

      if (!selectedRenderWindow && m_SelectedNode->GetBoolProperty(
            "PlanarFigureInitializedWindow", PlanarFigureInitializedWindow,
            _3DRenderWindow->GetRenderer()))
      {
        selectedRenderWindow = _3DRenderWindow;
      }

      const mitk::PlaneGeometry* _PlaneGeometry = _PlanarFigure->GetPlaneGeometry();

      mitk::VnlVector normal = _PlaneGeometry->GetNormalVnl();

      mitk::PlaneGeometry::ConstPointer worldGeometry1 =
          axialRenderWindow->GetRenderer()->GetCurrentWorldPlaneGeometry();
      mitk::PlaneGeometry::ConstPointer _Plane1 =
          dynamic_cast<const mitk::PlaneGeometry*>( worldGeometry1.GetPointer() );
      mitk::VnlVector normal1 = _Plane1->GetNormalVnl();

      mitk::PlaneGeometry::ConstPointer worldGeometry2 =
          sagittalRenderWindow->GetRenderer()->GetCurrentWorldPlaneGeometry();
      mitk::PlaneGeometry::ConstPointer _Plane2 =
          dynamic_cast<const mitk::PlaneGeometry*>( worldGeometry2.GetPointer() );
      mitk::VnlVector normal2 = _Plane2->GetNormalVnl();

      mitk::PlaneGeometry::ConstPointer worldGeometry3 =
          coronalRenderWindow->GetRenderer()->GetCurrentWorldPlaneGeometry();
      mitk::PlaneGeometry::ConstPointer _Plane3 =
          dynamic_cast<const mitk::PlaneGeometry*>( worldGeometry3.GetPointer() );
      mitk::VnlVector normal3 = _Plane3->GetNormalVnl();

      normal[0]  = fabs(normal[0]);  normal[1]  = fabs(normal[1]);  normal[2]  = fabs(normal[2]);
      normal1[0] = fabs(normal1[0]); normal1[1] = fabs(normal1[1]); normal1[2] = fabs(normal1[2]);
      normal2[0] = fabs(normal2[0]); normal2[1] = fabs(normal2[1]); normal2[2] = fabs(normal2[2]);
      normal3[0] = fabs(normal3[0]); normal3[1] = fabs(normal3[1]); normal3[2] = fabs(normal3[2]);

      double ang1 = angle(normal, normal1);
      double ang2 = angle(normal, normal2);
      double ang3 = angle(normal, normal3);

      if(ang1 < ang2 && ang1 < ang3)
      {
        selectedRenderWindow = axialRenderWindow;
      }
      else
      {
        if(ang2 < ang3)
        {
          selectedRenderWindow = sagittalRenderWindow;
        }
        else
        {
          selectedRenderWindow = coronalRenderWindow;
        }
      }

      // make node visible
      if (selectedRenderWindow)
      {
        const mitk::Point3D& centerP = _PlaneGeometry->GetOrigin();
        selectedRenderWindow->GetSliceNavigationController()->ReorientSlices(
              centerP, _PlaneGeometry->GetNormal());
      }
    }

    // set interactor for new node (if not already set)
    mitk::PlanarFigureInteractor::Pointer figureInteractor
        = dynamic_cast<mitk::PlanarFigureInteractor*>(m_SelectedNode->GetDataInteractor().GetPointer());

    if(figureInteractor.IsNull())
    {
      figureInteractor = mitk::PlanarFigureInteractor::New();
      us::Module* planarFigureModule = us::ModuleRegistry::GetModule( "MitkPlanarFigure" );
      figureInteractor->LoadStateMachine("PlanarFigureInteraction.xml", planarFigureModule );
      figureInteractor->SetEventConfig( "PlanarFigureConfig.xml", planarFigureModule );
      figureInteractor->SetDataNode( m_SelectedNode );
    }

    m_SelectedNode->SetProperty("planarfigure.iseditable",mitk::BoolProperty::New(true));
  }
}


void QmitkControlVisualizationPropertiesView::SetInteractor()
{
  // BUG 19179
  //    typedef std::vector<mitk::DataNode*> Container;
  //    Container _NodeSet = this->GetDataManagerSelection();
  //    mitk::DataNode* node = 0;
  //    mitk::FiberBundle* bundle = 0;
  //    mitk::FiberBundleInteractor::Pointer bundleInteractor = 0;

  //    // finally add all nodes to the model
  //    for(Container::const_iterator it=_NodeSet.begin(); it!=_NodeSet.end()
  //        ; it++)
  //    {
  //        node = const_cast<mitk::DataNode*>(*it);
  //        bundle = dynamic_cast<mitk::FiberBundle*>(node->GetData());

  //        if(bundle)
  //        {
  //            bundleInteractor = dynamic_cast<mitk::FiberBundleInteractor*>(node->GetInteractor());

  //            if(bundleInteractor.IsNotNull())
  //                mitk::GlobalInteraction::GetInstance()->RemoveInteractor(bundleInteractor);

  //            if(!m_Controls->m_Crosshair->isChecked())
  //            {
  //                m_Controls->m_Crosshair->setChecked(false);
  //                this->GetActiveStdMultiWidget()->GetRenderWindow4()->setCursor(Qt::ArrowCursor);
  //                m_CurrentPickingNode = 0;
  //            }
  //            else
  //            {
  //                m_Controls->m_Crosshair->setChecked(true);
  //                bundleInteractor = mitk::FiberBundleInteractor::New("FiberBundleInteractor", node);
  //                mitk::GlobalInteraction::GetInstance()->AddInteractor(bundleInteractor);
  //                this->GetActiveStdMultiWidget()->GetRenderWindow4()->setCursor(Qt::CrossCursor);
  //                m_CurrentPickingNode = node;
  //            }

  //        }
  //    }
}


void QmitkControlVisualizationPropertiesView::TubeRadiusChanged()
{
  if(m_SelectedNode && dynamic_cast<mitk::FiberBundle*>(m_SelectedNode->GetData()))
  {
    float newRadius = m_Controls->m_TubeWidth->value();
    m_SelectedNode->SetFloatProperty("shape.tuberadius", newRadius);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkControlVisualizationPropertiesView::RibbonWidthChanged()
{
  if(m_SelectedNode && dynamic_cast<mitk::FiberBundle*>(m_SelectedNode->GetData()))
  {
    float newWidth = m_Controls->m_RibbonWidth->value();
    m_SelectedNode->SetFloatProperty("shape.ribbonwidth", newWidth);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkControlVisualizationPropertiesView::LineWidthChanged()
{
  if(m_SelectedNode && dynamic_cast<mitk::PeakImage*>(m_SelectedNode->GetData()))
  {
    auto newWidth = m_Controls->m_LineWidth->value();
    float currentWidth = 0;
    m_SelectedNode->SetFloatProperty("shape.linewidth", currentWidth);
    if (currentWidth==newWidth)
      return;
    m_SelectedNode->SetFloatProperty("shape.linewidth", newWidth);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}


void QmitkControlVisualizationPropertiesView::Welcome()
{
  berry::PlatformUI::GetWorkbench()->GetIntroManager()
      ->ShowIntro(GetSite()->GetWorkbenchWindow(), false);
}

void QmitkControlVisualizationPropertiesView::OnColourisationModeChanged()
{
  if( m_SelectedNode && m_NodeUsedForOdfVisualization.IsNotNull() )
  {
    m_SelectedNode->SetProperty( "DiffusionCore.Rendering.OdfVtkMapper.ColourisationModeBit", mitk::BoolProperty::New( m_Controls->m_OdfColorBox->currentIndex() ) );
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  else
  {
    MITK_DEBUG << "QmitkControlVisualizationPropertiesView::OnColourisationModeChanged() was called but m_NodeUsedForOdfVisualization was Null.";
  }
}
