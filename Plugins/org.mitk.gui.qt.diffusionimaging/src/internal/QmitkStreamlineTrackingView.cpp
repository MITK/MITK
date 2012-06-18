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
    }
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
    m_Controls->m_TensorImageLabel->setText("-");
    m_Controls->m_RoiImageLabel->setText("-");

    if(nodes.empty())
        return;

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
                if (isBinary)
                {
                    m_SeedRoi = dynamic_cast<mitk::Image*>(node->GetData());
                    m_Controls->m_RoiImageLabel->setText(node->GetName().c_str());
                }
            }
        }
    }

    if(m_TensorImage.IsNotNull())
        m_Controls->commandLinkButton->setEnabled(true);
    else
        m_Controls->commandLinkButton->setEnabled(false);
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

    if (m_SeedRoi.IsNotNull())
    {
        CastType2::Pointer caster2 = CastType2::New();
        caster2->SetInput(m_SeedRoi);
        caster2->Update();
        ItkUCharImageType::Pointer mask = caster2->GetOutput();
        filter->SetMaskImage(mask);
    }

    filter->Update();

    vtkSmartPointer<vtkPolyData> fiberBundle = filter->GetFiberPolyData();
    if ( fiberBundle->GetNumberOfLines()==0 )
        return;
    mitk::FiberBundleX::Pointer fib = mitk::FiberBundleX::New(fiberBundle);

    fib->RemoveShortFibers(m_Controls->m_MinTractLengthSlider->value());

    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(fib);

    QString name(m_TensorImageNode->GetName().c_str());
    name += "_FiberBundle";
    node->SetName(name.toStdString());

    node->SetVisibility(true);

    GetDataStorage()->Add(node);
}


