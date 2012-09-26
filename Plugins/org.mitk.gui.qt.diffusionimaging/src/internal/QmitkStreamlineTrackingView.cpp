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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIStructuredSelection.h>

// Qmitk
#include "QmitkStreamlineTrackingView.h"
#include "QmitkStdMultiWidget.h"

// Qt
#include <QMessageBox>

// MITK
#include <mitkImageToItk.h>
#include <mitkFiberBundleX.h>
#include <mitkImageCast.h>

// VTK
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkSmartPointer.h>
#include <vtkPolyLine.h>
#include <vtkCellData.h>


const std::string QmitkStreamlineTrackingView::VIEW_ID = "org.mitk.views.streamlinetracking";
const std::string id_DataManager = "org.mitk.views.datamanager";
using namespace berry;

QmitkStreamlineTrackingView::QmitkStreamlineTrackingView()
    : QmitkFunctionality()
    , m_Controls( 0 )
    , m_MultiWidget( NULL )
    , m_TensorImage( NULL )
    , m_SeedRoi( NULL )
{
}

// Destructor
QmitkStreamlineTrackingView::~QmitkStreamlineTrackingView()
{

}

void QmitkStreamlineTrackingView::CreateQtPartControl( QWidget *parent )
{
    if ( !m_Controls )
    {
        // create GUI widgets from the Qt Designer's .ui file
        m_Controls = new Ui::QmitkStreamlineTrackingViewControls;
        m_Controls->setupUi( parent );

        connect( m_Controls->commandLinkButton, SIGNAL(clicked()), this, SLOT(DoFiberTracking()) );
        connect( m_Controls->m_SeedsPerVoxelSlider, SIGNAL(valueChanged(int)), this, SLOT(OnSeedsPerVoxelChanged(int)) );
        connect( m_Controls->m_MinTractLengthSlider, SIGNAL(valueChanged(int)), this, SLOT(OnMinTractLengthChanged(int)) );
        connect( m_Controls->m_FaThresholdSlider, SIGNAL(valueChanged(int)), this, SLOT(OnFaThresholdChanged(int)) );
        connect( m_Controls->m_AngularThresholdSlider, SIGNAL(valueChanged(int)), this, SLOT(OnAngularThresholdChanged(int)) );
        connect( m_Controls->m_StepsizeSlider, SIGNAL(valueChanged(int)), this, SLOT(OnStepsizeChanged(int)) );
        connect( m_Controls->m_fSlider, SIGNAL(valueChanged(int)), this, SLOT(OnfChanged(int)) );
        connect( m_Controls->m_gSlider, SIGNAL(valueChanged(int)), this, SLOT(OngChanged(int)) );
    }
}

void QmitkStreamlineTrackingView::OnfChanged(int value)
{
    m_Controls->m_fLabel->setText(QString("f: ")+QString::number((float)value/100));
}

void QmitkStreamlineTrackingView::OngChanged(int value)
{
    m_Controls->m_gLabel->setText(QString("g: ")+QString::number((float)value/100));
}

void QmitkStreamlineTrackingView::OnAngularThresholdChanged(int value)
{
    if (value<0)
        m_Controls->m_AngularThresholdLabel->setText(QString("Min. Curvature Radius: auto"));
    else
        m_Controls->m_AngularThresholdLabel->setText(QString("Min. Curvature Radius: ")+QString::number((float)value/10)+QString("mm"));
}

void QmitkStreamlineTrackingView::OnSeedsPerVoxelChanged(int value)
{
    m_Controls->m_SeedsPerVoxelLabel->setText(QString("Seeds per Voxel: ")+QString::number(value));
}

void QmitkStreamlineTrackingView::OnMinTractLengthChanged(int value)
{
    m_Controls->m_MinTractLengthLabel->setText(QString("Min. Tract Length: ")+QString::number(value)+QString("mm"));
}

void QmitkStreamlineTrackingView::OnFaThresholdChanged(int value)
{
    m_Controls->m_FaThresholdLabel->setText(QString("FA Threshold: ")+QString::number((float)value/100));
}

void QmitkStreamlineTrackingView::OnStepsizeChanged(int value)
{
    if (value==0)
        m_Controls->m_StepsizeLabel->setText(QString("Stepsize: auto"));
    else
        m_Controls->m_StepsizeLabel->setText(QString("Stepsize: ")+QString::number((float)value/10)+QString("mm"));
}

void QmitkStreamlineTrackingView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
    m_MultiWidget = &stdMultiWidget;
}


void QmitkStreamlineTrackingView::StdMultiWidgetNotAvailable()
{
    m_MultiWidget = NULL;
}

void QmitkStreamlineTrackingView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
    m_TensorImageNode = NULL;
    m_TensorImage = NULL;
    m_SeedRoi = NULL;
    m_MaskImage = NULL;
    m_Controls->m_TensorImageLabel->setText("<font color='red'>mandatory</font>");
    m_Controls->m_RoiImageLabel->setText("<font color='grey'>optional</font>");
    m_Controls->m_MaskImageLabel->setText("<font color='grey'>optional</font>");

    for( std::vector<mitk::DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it )
    {
        mitk::DataNode::Pointer node = *it;

        if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
        {
            if( dynamic_cast<mitk::TensorImage*>(node->GetData()) )
            {
                m_TensorImageNode = node;
                m_TensorImage = dynamic_cast<mitk::TensorImage*>(node->GetData());
                m_Controls->m_TensorImageLabel->setText(node->GetName().c_str());
            }
            else
            {
                bool isBinary = false;
                node->GetPropertyValue<bool>("binary", isBinary);
                if (isBinary && m_SeedRoi.IsNull())
                {
                    m_SeedRoi = dynamic_cast<mitk::Image*>(node->GetData());
                    m_Controls->m_RoiImageLabel->setText(node->GetName().c_str());
                }
                else if (isBinary)
                {
                    m_MaskImage = dynamic_cast<mitk::Image*>(node->GetData());
                    m_Controls->m_MaskImageLabel->setText(node->GetName().c_str());
                }
            }
        }
    }

    if(m_TensorImageNode.IsNotNull())
    {
        m_Controls->m_InputData->setTitle("Input Data");
        m_Controls->commandLinkButton->setEnabled(true);
    }
    else
    {
        m_Controls->m_InputData->setTitle("Please Select Input Data");
        m_Controls->commandLinkButton->setEnabled(false);
    }
}



void QmitkStreamlineTrackingView::DoFiberTracking()
{
    if (m_TensorImage.IsNull())
        return;

    typedef itk::Image< itk::DiffusionTensor3D<float>, 3> TensorImageType;
    typedef mitk::ImageToItk<TensorImageType> CastType;
    typedef mitk::ImageToItk<ItkUCharImageType> CastType2;

    CastType::Pointer caster = CastType::New();
    caster->SetInput(m_TensorImage);
    caster->Update();
    TensorImageType::Pointer image = caster->GetOutput();

    typedef itk::StreamlineTrackingFilter< float > FilterType;
    FilterType::Pointer filter = FilterType::New();
    filter->SetInput(image);
    filter->SetSeedsPerVoxel(m_Controls->m_SeedsPerVoxelSlider->value());
    filter->SetFaThreshold((float)m_Controls->m_FaThresholdSlider->value()/100);
    filter->SetMinCurvatureRadius((float)m_Controls->m_AngularThresholdSlider->value()/10);
    filter->SetStepSize((float)m_Controls->m_StepsizeSlider->value()/10);
    filter->SetF((float)m_Controls->m_fSlider->value()/100);
    filter->SetG((float)m_Controls->m_gSlider->value()/100);
    filter->SetInterpolate(m_Controls->m_InterpolationBox->isChecked());
    filter->SetMinTractLength(m_Controls->m_MinTractLengthSlider->value());
    //filter->SetNumberOfThreads(1);

    if (m_SeedRoi.IsNotNull())
    {
        ItkUCharImageType::Pointer mask = ItkUCharImageType::New();
        mitk::CastToItkImage<ItkUCharImageType>(m_SeedRoi, mask);
        filter->SetSeedImage(mask);
    }

    if (m_MaskImage.IsNotNull())
    {
        ItkUCharImageType::Pointer mask = ItkUCharImageType::New();
        mitk::CastToItkImage<ItkUCharImageType>(m_MaskImage, mask);
        filter->SetMaskImage(mask);
    }

    filter->Update();

    vtkSmartPointer<vtkPolyData> fiberBundle = filter->GetFiberPolyData();
    if ( fiberBundle->GetNumberOfLines()==0 )
        return;
    mitk::FiberBundleX::Pointer fib = mitk::FiberBundleX::New(fiberBundle);

    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(fib);
    QString name(m_TensorImageNode->GetName().c_str());
    name += "_Streamline";
    node->SetName(name.toStdString());
    node->SetVisibility(true);
    GetDataStorage()->Add(node);
}


