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

#include "mitkTbssImage.h"
#include "mitkPlanarFigure.h"
#include "mitkFiberBundle.h"
#include "QmitkDataStorageComboBox.h"
#include "QmitkStdMultiWidget.h"
#include "mitkFiberBundleInteractor.h"
#include "mitkPlanarFigureInteractor.h"
#include <mitkQBallImage.h>
#include <mitkTensorImage.h>
#include <mitkImage.h>
#include <mitkDiffusionPropertyHelper.h>
#include <mitkConnectomicsNetwork.h>
#include "mitkGlobalInteraction.h"
#include "usModuleRegistry.h"

#include "mitkPlaneGeometry.h"

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

#define ROUND(a) ((a)>0 ? (int)((a)+0.5) : -(int)(0.5-(a)))

const std::string QmitkControlVisualizationPropertiesView::VIEW_ID = "org.mitk.views.controlvisualizationpropertiesview";

using namespace berry;

QmitkControlVisualizationPropertiesView::QmitkControlVisualizationPropertiesView()
    : QmitkFunctionality(),
      m_Controls(NULL),
      m_MultiWidget(NULL),
      m_NodeUsedForOdfVisualization(NULL),
      m_IconTexOFF(new QIcon(":/QmitkDiffusionImaging/texIntOFFIcon.png")),
      m_IconTexON(new QIcon(":/QmitkDiffusionImaging/texIntONIcon.png")),
      m_IconGlyOFF_T(new QIcon(":/QmitkDiffusionImaging/glyphsoff_T.png")),
      m_IconGlyON_T(new QIcon(":/QmitkDiffusionImaging/glyphson_T.png")),
      m_IconGlyOFF_C(new QIcon(":/QmitkDiffusionImaging/glyphsoff_C.png")),
      m_IconGlyON_C(new QIcon(":/QmitkDiffusionImaging/glyphson_C.png")),
      m_IconGlyOFF_S(new QIcon(":/QmitkDiffusionImaging/glyphsoff_S.png")),
      m_IconGlyON_S(new QIcon(":/QmitkDiffusionImaging/glyphson_S.png")),
      m_CurrentSelection(0),
      m_CurrentPickingNode(0),
      m_GlyIsOn_S(false),
      m_GlyIsOn_C(false),
      m_GlyIsOn_T(false),
      m_FiberBundleObserverTag(0),
      m_FiberBundleObserveOpacityTag(0),
      m_Color(NULL)
{
    currentThickSlicesMode = 1;
    m_MyMenu = NULL;
    int numThread = itk::MultiThreader::GetGlobalMaximumNumberOfThreads();
    if (numThread > 12)
        numThread = 12;
    itk::MultiThreader::SetGlobalDefaultNumberOfThreads(numThread);
}

QmitkControlVisualizationPropertiesView::~QmitkControlVisualizationPropertiesView()
{
    if(m_SlicesRotationObserverTag1 )
    {
        mitk::SlicesCoordinator::Pointer coordinator = m_MultiWidget->GetSlicesRotator();
        if( coordinator.IsNotNull() )
            coordinator->RemoveObserver(m_SlicesRotationObserverTag1);
    }
    if( m_SlicesRotationObserverTag2)
    {
        mitk::SlicesCoordinator::Pointer coordinator = m_MultiWidget->GetSlicesRotator();
        if( coordinator.IsNotNull() )
            coordinator->RemoveObserver(m_SlicesRotationObserverTag1);
    }

    this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->RemovePostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelListener.data());
}

void QmitkControlVisualizationPropertiesView::OnThickSlicesModeSelected( QAction* action )
{
    currentThickSlicesMode = action->data().toInt();

    switch(currentThickSlicesMode)
    {
    default:
    case 1:
        this->m_Controls->m_TSMenu->setText("MIP");
        break;
    case 2:
        this->m_Controls->m_TSMenu->setText("SUM");
        break;
    case 3:
        this->m_Controls->m_TSMenu->setText("WEIGH");
        break;
    }

    mitk::DataNode* n;
    n = this->m_MultiWidget->GetWidgetPlane1(); if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( currentThickSlicesMode ) );
    n = this->m_MultiWidget->GetWidgetPlane2(); if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( currentThickSlicesMode ) );
    n = this->m_MultiWidget->GetWidgetPlane3(); if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( currentThickSlicesMode ) );

    mitk::BaseRenderer::Pointer renderer =
            this->GetActiveStdMultiWidget()->GetRenderWindow1()->GetRenderer();
    if(renderer.IsNotNull())
    {
        renderer->SendUpdateSlice();
    }
    renderer = this->GetActiveStdMultiWidget()->GetRenderWindow2()->GetRenderer();
    if(renderer.IsNotNull())
    {
        renderer->SendUpdateSlice();
    }
    renderer = this->GetActiveStdMultiWidget()->GetRenderWindow3()->GetRenderer();
    if(renderer.IsNotNull())
    {
        renderer->SendUpdateSlice();
    }
    renderer->GetRenderingManager()->RequestUpdateAll();
}

void QmitkControlVisualizationPropertiesView::OnTSNumChanged(int num)
{
    if(num==0)
    {
        mitk::DataNode* n;
        n = this->m_MultiWidget->GetWidgetPlane1();
        if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( 0 ) );
        if(n) n->SetProperty( "reslice.thickslices.num", mitk::IntProperty::New( num ) );
        if(n) n->SetProperty( "reslice.thickslices.showarea", mitk::BoolProperty::New( false ) );

        n = this->m_MultiWidget->GetWidgetPlane2();
        if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( 0 ) );
        if(n) n->SetProperty( "reslice.thickslices.num", mitk::IntProperty::New( num ) );
        if(n) n->SetProperty( "reslice.thickslices.showarea", mitk::BoolProperty::New( false ) );

        n = this->m_MultiWidget->GetWidgetPlane3();
        if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( 0 ) );
        if(n) n->SetProperty( "reslice.thickslices.num", mitk::IntProperty::New( num ) );
        if(n) n->SetProperty( "reslice.thickslices.showarea", mitk::BoolProperty::New( false ) );
    }
    else
    {
        mitk::DataNode* n;
        n = this->m_MultiWidget->GetWidgetPlane1();
        if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( currentThickSlicesMode ) );
        if(n) n->SetProperty( "reslice.thickslices.num", mitk::IntProperty::New( num ) );
        if(n) n->SetProperty( "reslice.thickslices.showarea", mitk::BoolProperty::New( (num>0) ) );

        n = this->m_MultiWidget->GetWidgetPlane2();
        if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( currentThickSlicesMode ) );
        if(n) n->SetProperty( "reslice.thickslices.num", mitk::IntProperty::New( num ) );
        if(n) n->SetProperty( "reslice.thickslices.showarea", mitk::BoolProperty::New( (num>0) ) );

        n = this->m_MultiWidget->GetWidgetPlane3();
        if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( currentThickSlicesMode ) );
        if(n) n->SetProperty( "reslice.thickslices.num", mitk::IntProperty::New( num ) );
        if(n) n->SetProperty( "reslice.thickslices.showarea", mitk::BoolProperty::New( (num>0) ) );
    }

    m_TSLabel->setText(QString::number(num*2+1));

    mitk::BaseRenderer::Pointer renderer = this->GetActiveStdMultiWidget()->GetRenderWindow1()->GetRenderer();
    if(renderer.IsNotNull())
        renderer->SendUpdateSlice();

    renderer = this->GetActiveStdMultiWidget()->GetRenderWindow2()->GetRenderer();
    if(renderer.IsNotNull())
        renderer->SendUpdateSlice();

    renderer = this->GetActiveStdMultiWidget()->GetRenderWindow3()->GetRenderer();
    if(renderer.IsNotNull())
        renderer->SendUpdateSlice();

    renderer->GetRenderingManager()->RequestUpdateAll(mitk::RenderingManager::REQUEST_UPDATE_2DWINDOWS);
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

#ifndef DIFFUSION_IMAGING_EXTENDED
        int size = m_Controls->m_AdditionalScaling->count();
        for(int t=0; t<size; t++)
        {
            if(m_Controls->m_AdditionalScaling->itemText(t).toStdString() == "Scale by ASR")
            {
                m_Controls->m_AdditionalScaling->removeItem(t);
            }
        }
#endif

        m_Controls->m_ScalingFrame->setVisible(false);
        m_Controls->m_NormalizationFrame->setVisible(false);
    }
}

void QmitkControlVisualizationPropertiesView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
    m_MultiWidget = &stdMultiWidget;

    if (m_MultiWidget)
    {
        mitk::SlicesCoordinator* coordinator = m_MultiWidget->GetSlicesRotator();
        if (coordinator)
        {
            itk::ReceptorMemberCommand<QmitkControlVisualizationPropertiesView>::Pointer command2 = itk::ReceptorMemberCommand<QmitkControlVisualizationPropertiesView>::New();
            command2->SetCallbackFunction( this, &QmitkControlVisualizationPropertiesView::SliceRotation );
            m_SlicesRotationObserverTag1 = coordinator->AddObserver( mitk::SliceRotationEvent(), command2 );
        }

        coordinator = m_MultiWidget->GetSlicesSwiveller();
        if (coordinator)
        {
            itk::ReceptorMemberCommand<QmitkControlVisualizationPropertiesView>::Pointer command2 = itk::ReceptorMemberCommand<QmitkControlVisualizationPropertiesView>::New();
            command2->SetCallbackFunction( this, &QmitkControlVisualizationPropertiesView::SliceRotation );
            m_SlicesRotationObserverTag2 = coordinator->AddObserver( mitk::SliceRotationEvent(), command2 );
        }
    }
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

void QmitkControlVisualizationPropertiesView::StdMultiWidgetNotAvailable()
{
    m_MultiWidget = NULL;
}

void QmitkControlVisualizationPropertiesView::NodeRemoved(const mitk::DataNode* /*node*/)
{
}

#include <mitkMessage.h>
void QmitkControlVisualizationPropertiesView::CreateConnections()
{
    if ( m_Controls )
    {
        connect( (QObject*)(m_Controls->m_VisibleOdfsON_T), SIGNAL(clicked()), this, SLOT(VisibleOdfsON_T()) );
        connect( (QObject*)(m_Controls->m_VisibleOdfsON_S), SIGNAL(clicked()), this, SLOT(VisibleOdfsON_S()) );
        connect( (QObject*)(m_Controls->m_VisibleOdfsON_C), SIGNAL(clicked()), this, SLOT(VisibleOdfsON_C()) );
        connect( (QObject*)(m_Controls->m_ShowMaxNumber), SIGNAL(editingFinished()), this, SLOT(ShowMaxNumberChanged()) );
        connect( (QObject*)(m_Controls->m_NormalizationDropdown), SIGNAL(currentIndexChanged(int)), this, SLOT(NormalizationDropdownChanged(int)) );
        connect( (QObject*)(m_Controls->m_ScalingFactor), SIGNAL(valueChanged(double)), this, SLOT(ScalingFactorChanged(double)) );
        connect( (QObject*)(m_Controls->m_AdditionalScaling), SIGNAL(currentIndexChanged(int)), this, SLOT(AdditionalScaling(int)) );
        connect( (QObject*)(m_Controls->m_ScalingCheckbox), SIGNAL(clicked()), this, SLOT(ScalingCheckbox()) );
        connect((QObject*) m_Controls->m_ResetColoring, SIGNAL(clicked()), (QObject*) this, SLOT(BundleRepresentationResetColoring()));
        connect((QObject*) m_Controls->m_FiberFading2D, SIGNAL(clicked()), (QObject*) this, SLOT( Fiber2DfadingEFX() ) );
        connect((QObject*) m_Controls->m_FiberThicknessSlider, SIGNAL(sliderReleased()), (QObject*) this, SLOT( FiberSlicingThickness2D() ) );
        connect((QObject*) m_Controls->m_FiberThicknessSlider, SIGNAL(valueChanged(int)), (QObject*) this, SLOT( FiberSlicingUpdateLabel(int) ));
        connect((QObject*) m_Controls->m_Crosshair, SIGNAL(clicked()), (QObject*) this, SLOT(SetInteractor()));
        connect((QObject*) m_Controls->m_LineWidth, SIGNAL(editingFinished()), (QObject*) this, SLOT(LineWidthChanged()));
        connect((QObject*) m_Controls->m_TubeWidth, SIGNAL(editingFinished()), (QObject*) this, SLOT(TubeRadiusChanged()));
    }
}

void QmitkControlVisualizationPropertiesView::Activated()
{

}

void QmitkControlVisualizationPropertiesView::Deactivated()
{

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
        mitk::BValueMapProperty* bmapproperty = static_cast<mitk::BValueMapProperty*>(dimg->GetProperty(mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME.c_str()).GetPointer() );
        mitk::DiffusionPropertyHelper::BValueMapType map = bmapproperty->GetBValueMap();
        if( map[0].size() > 0)
            displayChannelPropertyValue = map[0].front();

        notConst->SetIntProperty("DisplayChannel", displayChannelPropertyValue );
    }
}

/* OnSelectionChanged is registered to SelectionService, therefore no need to
implement SelectionService Listener explicitly */
void QmitkControlVisualizationPropertiesView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
    m_Controls->m_BundleControlsFrame->setVisible(false);
    m_Controls->m_ImageControlsFrame->setVisible(false);

    if (nodes.size()>1) // only do stuff if one node is selected
        return;

    m_Controls->m_NumberGlyphsFrame->setVisible(false);
    m_Controls->m_GlyphFrame->setVisible(false);
    m_Controls->m_TSMenu->setVisible(false);

    m_SelectedNode = NULL;

    int numOdfImages = 0;
    for( std::vector<mitk::DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it )
    {
        mitk::DataNode::Pointer node = *it;
        if(node.IsNull())
            continue;

        mitk::BaseData* nodeData = node->GetData();
        if(nodeData == NULL)
            continue;

        m_SelectedNode = node;

        if (dynamic_cast<mitk::FiberBundle*>(nodeData))
        {
            // handle fiber bundle property observers
            if (m_Color.IsNotNull())
                m_Color->RemoveObserver(m_FiberBundleObserverTag);
            itk::ReceptorMemberCommand<QmitkControlVisualizationPropertiesView>::Pointer command = itk::ReceptorMemberCommand<QmitkControlVisualizationPropertiesView>::New();
            command->SetCallbackFunction( this, &QmitkControlVisualizationPropertiesView::SetFiberBundleCustomColor );
            m_Color = dynamic_cast<mitk::ColorProperty*>(node->GetProperty("color", NULL));
            if (m_Color.IsNotNull())
                m_FiberBundleObserverTag = m_Color->AddObserver( itk::ModifiedEvent(), command );

            if (m_Opacity.IsNotNull())
                m_Opacity->RemoveObserver(m_FiberBundleObserveOpacityTag);
            itk::ReceptorMemberCommand<QmitkControlVisualizationPropertiesView>::Pointer command2 = itk::ReceptorMemberCommand<QmitkControlVisualizationPropertiesView>::New();
            command2->SetCallbackFunction( this, &QmitkControlVisualizationPropertiesView::SetFiberBundleOpacity );
            m_Opacity = dynamic_cast<mitk::FloatProperty*>(node->GetProperty("opacity", NULL));
            if (m_Opacity.IsNotNull())
                m_FiberBundleObserveOpacityTag = m_Opacity->AddObserver( itk::ModifiedEvent(), command2 );

            m_Controls->m_BundleControlsFrame->setVisible(true);

            // ???
            if(m_CurrentPickingNode != 0 && node.GetPointer() != m_CurrentPickingNode)
                m_Controls->m_Crosshair->setEnabled(false);
            else
                m_Controls->m_Crosshair->setEnabled(true);

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
            m_Controls->m_FiberThicknessSlider->setFocus();
        }
        else if(dynamic_cast<mitk::QBallImage*>(nodeData) || dynamic_cast<mitk::TensorImage*>(nodeData))
        {
            m_Controls->m_ImageControlsFrame->setVisible(true);
            m_Controls->m_NumberGlyphsFrame->setVisible(true);
            m_Controls->m_GlyphFrame->setVisible(true);

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

            int val;
            node->GetIntProperty("ShowMaxNumber", val);
            m_Controls->m_ShowMaxNumber->setValue(val);

            m_Controls->m_NormalizationDropdown->setCurrentIndex(dynamic_cast<mitk::EnumerationProperty*>(node->GetProperty("Normalization"))->GetValueAsId());

            float fval;
            node->GetFloatProperty("Scaling",fval);
            m_Controls->m_ScalingFactor->setValue(fval);

            m_Controls->m_AdditionalScaling->setCurrentIndex(dynamic_cast<mitk::EnumerationProperty*>(node->GetProperty("ScaleBy"))->GetValueAsId());

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

    if( nodes.empty() )
        return;

    mitk::DataNode::Pointer node = nodes.at(0);

    if( node.IsNull() )
        return;

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

    for( std::vector<mitk::DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it )
    {
        mitk::Image* image = dynamic_cast<mitk::Image*>((*it)->GetData());
        if (image)
        {
            int size = std::max(image->GetDimension(0), std::max(image->GetDimension(1), image->GetDimension(2)));
            if (size>maxTS)
                maxTS=size;
        }
    }
    maxTS /= 2;

    int currentNum = 0;
    {
        mitk::IntProperty::Pointer m = dynamic_cast<mitk::IntProperty*>(node->GetProperty( "reslice.thickslices.num" ));
        if( m.IsNotNull() )
        {
            currentNum = m->GetValue();
            if(currentNum < 0) currentNum = 0;
            if(currentNum > maxTS) currentNum = maxTS;
        }
    }

    if(currentTSMode==0)
        currentNum=0;

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
    mipThickSlicesAction->setChecked(currentThickSlicesMode==1);
    mipThickSlicesAction->setData(1);
    myMenu->addAction( mipThickSlicesAction );

    QAction* sumThickSlicesAction = new QAction(myMenu);
    sumThickSlicesAction->setActionGroup(thickSliceModeActionGroup);
    sumThickSlicesAction->setText("SUM (sum intensity proj.)");
    sumThickSlicesAction->setCheckable(true);
    sumThickSlicesAction->setChecked(currentThickSlicesMode==2);
    sumThickSlicesAction->setData(2);
    myMenu->addAction( sumThickSlicesAction );

    QAction* weightedThickSlicesAction = new QAction(myMenu);
    weightedThickSlicesAction->setActionGroup(thickSliceModeActionGroup);
    weightedThickSlicesAction->setText("WEIGHTED (gaussian proj.)");
    weightedThickSlicesAction->setCheckable(true);
    weightedThickSlicesAction->setChecked(currentThickSlicesMode==3);
    weightedThickSlicesAction->setData(3);
    myMenu->addAction( weightedThickSlicesAction );

    connect( thickSliceModeActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(OnThickSlicesModeSelected(QAction*)) );
}

void QmitkControlVisualizationPropertiesView::VisibleOdfsON_S()
{
    m_GlyIsOn_S = m_Controls->m_VisibleOdfsON_S->isChecked();
    if (m_NodeUsedForOdfVisualization.IsNull())
    {
        MITK_WARN << "ODF visualization activated but m_NodeUsedForOdfVisualization is NULL";
        return;
    }
    m_NodeUsedForOdfVisualization->SetBoolProperty("VisibleOdfs_S", m_GlyIsOn_S);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkControlVisualizationPropertiesView::VisibleOdfsON_T()
{
    m_GlyIsOn_T = m_Controls->m_VisibleOdfsON_T->isChecked();
    if (m_NodeUsedForOdfVisualization.IsNull())
    {
        MITK_WARN << "ODF visualization activated but m_NodeUsedForOdfVisualization is NULL";
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
        MITK_WARN << "ODF visualization activated but m_NodeUsedForOdfVisualization is NULL";
        return;
    }
    m_NodeUsedForOdfVisualization->SetBoolProperty("VisibleOdfs_C", m_GlyIsOn_C);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

bool QmitkControlVisualizationPropertiesView::IsPlaneRotated()
{
    mitk::Image* currentImage = dynamic_cast<mitk::Image* >( m_NodeUsedForOdfVisualization->GetData() );
    if( currentImage == NULL )
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

    double eps = 0.000001;
    // for all 2D renderwindows of m_MultiWidget check alignment
    {
        mitk::PlaneGeometry::ConstPointer displayPlane = dynamic_cast<const mitk::PlaneGeometry*>( m_MultiWidget->GetRenderWindow1()->GetRenderer()->GetCurrentWorldGeometry2D() );
        if (displayPlane.IsNull()) return false;
        mitk::Vector3D normal       = displayPlane->GetNormal();
        normal.Normalize();
        int test = 0;
        if( fabs(fabs(dot_product(normal.GetVnlVector(),imageNormal0.GetVnlVector()))-1) > eps )
            test++;
        if( fabs(fabs(dot_product(normal.GetVnlVector(),imageNormal1.GetVnlVector()))-1) > eps )
            test++;
        if( fabs(fabs(dot_product(normal.GetVnlVector(),imageNormal2.GetVnlVector()))-1) > eps )
            test++;
        if (test==3)
            return true;
    }
    {
        mitk::PlaneGeometry::ConstPointer displayPlane = dynamic_cast<const mitk::PlaneGeometry*>( m_MultiWidget->GetRenderWindow2()->GetRenderer()->GetCurrentWorldGeometry2D() );
        if (displayPlane.IsNull()) return false;
        mitk::Vector3D normal       = displayPlane->GetNormal();
        normal.Normalize();
        int test = 0;
        if( fabs(fabs(dot_product(normal.GetVnlVector(),imageNormal0.GetVnlVector()))-1) > eps )
            test++;
        if( fabs(fabs(dot_product(normal.GetVnlVector(),imageNormal1.GetVnlVector()))-1) > eps )
            test++;
        if( fabs(fabs(dot_product(normal.GetVnlVector(),imageNormal2.GetVnlVector()))-1) > eps )
            test++;
        if (test==3)
            return true;
    }
    {
        mitk::PlaneGeometry::ConstPointer displayPlane = dynamic_cast<const mitk::PlaneGeometry*>( m_MultiWidget->GetRenderWindow3()->GetRenderer()->GetCurrentWorldGeometry2D() );
        if (displayPlane.IsNull()) return false;
        mitk::Vector3D normal       = displayPlane->GetNormal();
        normal.Normalize();
        int test = 0;
        if( fabs(fabs(dot_product(normal.GetVnlVector(),imageNormal0.GetVnlVector()))-1) > eps )
            test++;
        if( fabs(fabs(dot_product(normal.GetVnlVector(),imageNormal1.GetVnlVector()))-1) > eps )
            test++;
        if( fabs(fabs(dot_product(normal.GetVnlVector(),imageNormal2.GetVnlVector()))-1) > eps )
            test++;
        if (test==3)
            return true;
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

    if (dynamic_cast<mitk::QBallImage*>(m_SelectedNode->GetData()) || dynamic_cast<mitk::TensorImage*>(m_SelectedNode->GetData()))
        m_SelectedNode->SetIntProperty("ShowMaxNumber", maxNr);

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

    if (dynamic_cast<mitk::QBallImage*>(m_SelectedNode->GetData()) || dynamic_cast<mitk::TensorImage*>(m_SelectedNode->GetData()))
        m_SelectedNode->SetProperty("Normalization", normMeth.GetPointer());

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkControlVisualizationPropertiesView::ScalingFactorChanged(double scalingFactor)
{
    if (dynamic_cast<mitk::QBallImage*>(m_SelectedNode->GetData()) || dynamic_cast<mitk::TensorImage*>(m_SelectedNode->GetData()))
        m_SelectedNode->SetFloatProperty("Scaling", scalingFactor);

    if(m_MultiWidget)
        m_MultiWidget->RequestUpdate();
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
#ifdef DIFFUSION_IMAGING_EXTENDED
    case 2:
        scaleBy->SetScaleByPrincipalCurvature();
        // commented in for SPIE paper, Principle curvature scaling
        //m_Controls->params_frame->setVisible(true);
        break;
#endif
    default:
        scaleBy->SetScaleByNothing();
    }

    if (dynamic_cast<mitk::QBallImage*>(m_SelectedNode->GetData()) || dynamic_cast<mitk::TensorImage*>(m_SelectedNode->GetData()))
        m_SelectedNode->SetProperty("Normalization", scaleBy.GetPointer());

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkControlVisualizationPropertiesView::ScalingCheckbox()
{
    m_Controls->m_ScalingFrame->setVisible(
                m_Controls->m_ScalingCheckbox->isChecked());

    if(!m_Controls->m_ScalingCheckbox->isChecked())
    {
        m_Controls->m_AdditionalScaling->setCurrentIndex(0);
        m_Controls->m_ScalingFactor->setValue(1.0);
    }
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
        if (fabs(fibThickness-currentThickness)<0.001)
            return;
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

void QmitkControlVisualizationPropertiesView::SetFiberBundleOpacity(const itk::EventObject& /*e*/)
{
    if(m_SelectedNode)
    {
        mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(m_SelectedNode->GetData());
        fib->RequestUpdate();
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
}

void QmitkControlVisualizationPropertiesView::SetFiberBundleCustomColor(const itk::EventObject& /*e*/)
{
    if(m_SelectedNode && dynamic_cast<mitk::FiberBundle*>(m_SelectedNode->GetData()))
    {
        float color[3];
        m_SelectedNode->GetColor(color);
        mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(m_SelectedNode->GetData());
        fib->SetFiberColors(color[0]*255, color[1]*255, color[2]*255);
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
}

void QmitkControlVisualizationPropertiesView::BundleRepresentationResetColoring()
{
    if(m_SelectedNode && dynamic_cast<mitk::FiberBundle*>(m_SelectedNode->GetData()))
    {
        mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(m_SelectedNode->GetData());
        fib->ColorFibersByOrientation();
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

            QmitkRenderWindow* RenderWindow1 =
                    this->GetActiveStdMultiWidget()->GetRenderWindow1();

            if (m_SelectedNode->GetBoolProperty("PlanarFigureInitializedWindow",
                                                PlanarFigureInitializedWindow, RenderWindow1->GetRenderer()))
            {
                selectedRenderWindow = RenderWindow1;
            }

            QmitkRenderWindow* RenderWindow2 =
                    this->GetActiveStdMultiWidget()->GetRenderWindow2();

            if (!selectedRenderWindow && m_SelectedNode->GetBoolProperty(
                        "PlanarFigureInitializedWindow", PlanarFigureInitializedWindow,
                        RenderWindow2->GetRenderer()))
            {
                selectedRenderWindow = RenderWindow2;
            }

            QmitkRenderWindow* RenderWindow3 =
                    this->GetActiveStdMultiWidget()->GetRenderWindow3();

            if (!selectedRenderWindow && m_SelectedNode->GetBoolProperty(
                        "PlanarFigureInitializedWindow", PlanarFigureInitializedWindow,
                        RenderWindow3->GetRenderer()))
            {
                selectedRenderWindow = RenderWindow3;
            }

            QmitkRenderWindow* RenderWindow4 =
                    this->GetActiveStdMultiWidget()->GetRenderWindow4();

            if (!selectedRenderWindow && m_SelectedNode->GetBoolProperty(
                        "PlanarFigureInitializedWindow", PlanarFigureInitializedWindow,
                        RenderWindow4->GetRenderer()))
            {
                selectedRenderWindow = RenderWindow4;
            }

            const mitk::PlaneGeometry* _PlaneGeometry = _PlanarFigure->GetPlaneGeometry();

            mitk::VnlVector normal = _PlaneGeometry->GetNormalVnl();

            mitk::Geometry2D::ConstPointer worldGeometry1 =
                    RenderWindow1->GetRenderer()->GetCurrentWorldGeometry2D();
            mitk::PlaneGeometry::ConstPointer _Plane1 =
                    dynamic_cast<const mitk::PlaneGeometry*>( worldGeometry1.GetPointer() );
            mitk::VnlVector normal1 = _Plane1->GetNormalVnl();

            mitk::Geometry2D::ConstPointer worldGeometry2 =
                    RenderWindow2->GetRenderer()->GetCurrentWorldGeometry2D();
            mitk::PlaneGeometry::ConstPointer _Plane2 =
                    dynamic_cast<const mitk::PlaneGeometry*>( worldGeometry2.GetPointer() );
            mitk::VnlVector normal2 = _Plane2->GetNormalVnl();

            mitk::Geometry2D::ConstPointer worldGeometry3 =
                    RenderWindow3->GetRenderer()->GetCurrentWorldGeometry2D();
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
                selectedRenderWindow = RenderWindow1;
            }
            else
            {
                if(ang2 < ang3)
                {
                    selectedRenderWindow = RenderWindow2;
                }
                else
                {
                    selectedRenderWindow = RenderWindow3;
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
    typedef std::vector<mitk::DataNode*> Container;
    Container _NodeSet = this->GetDataManagerSelection();
    mitk::DataNode* node = 0;
    mitk::FiberBundle* bundle = 0;
    mitk::FiberBundleInteractor::Pointer bundleInteractor = 0;

    // finally add all nodes to the model
    for(Container::const_iterator it=_NodeSet.begin(); it!=_NodeSet.end()
        ; it++)
    {
        node = const_cast<mitk::DataNode*>(*it);
        bundle = dynamic_cast<mitk::FiberBundle*>(node->GetData());

        if(bundle)
        {
            bundleInteractor = dynamic_cast<mitk::FiberBundleInteractor*>(node->GetInteractor());

            if(bundleInteractor.IsNotNull())
                mitk::GlobalInteraction::GetInstance()->RemoveInteractor(bundleInteractor);

            if(!m_Controls->m_Crosshair->isChecked())
            {
                m_Controls->m_Crosshair->setChecked(false);
                this->GetActiveStdMultiWidget()->GetRenderWindow4()->setCursor(Qt::ArrowCursor);
                m_CurrentPickingNode = 0;
            }
            else
            {
                m_Controls->m_Crosshair->setChecked(true);
                bundleInteractor = mitk::FiberBundleInteractor::New("FiberBundleInteractor", node);
                mitk::GlobalInteraction::GetInstance()->AddInteractor(bundleInteractor);
                this->GetActiveStdMultiWidget()->GetRenderWindow4()->setCursor(Qt::CrossCursor);
                m_CurrentPickingNode = node;
            }

        }
    }
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

void QmitkControlVisualizationPropertiesView::LineWidthChanged()
{
    if(m_SelectedNode && dynamic_cast<mitk::FiberBundle*>(m_SelectedNode->GetData()))
    {
        int newWidth = m_Controls->m_LineWidth->value();
        int currentWidth = 0;
        m_SelectedNode->GetIntProperty("shape.linewidth", currentWidth);
        if (currentWidth==newWidth)
            return;
        m_SelectedNode->SetIntProperty("shape.linewidth", newWidth);
        dynamic_cast<mitk::FiberBundle*>(m_SelectedNode->GetData())->RequestUpdate();
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
}

void QmitkControlVisualizationPropertiesView::Welcome()
{
    berry::PlatformUI::GetWorkbench()->GetIntroManager()->ShowIntro(
                GetSite()->GetWorkbenchWindow(), false);
}
