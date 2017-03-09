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
#include <mitkFiberBundle.h>
#include <mitkImageCast.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateDimension.h>

// VTK
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkSmartPointer.h>
#include <vtkPolyLine.h>
#include <vtkCellData.h>

#include <omp.h>


const std::string QmitkStreamlineTrackingView::VIEW_ID = "org.mitk.views.streamlinetracking";
const std::string id_DataManager = "org.mitk.views.datamanager";
using namespace berry;

QmitkStreamlineTrackingView::QmitkStreamlineTrackingView()
    : QmitkFunctionality()
    , m_Controls( 0 )
    , m_MultiWidget( NULL )
    , m_MaskImage( NULL )
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
        m_Controls->m_FaImageBox->SetDataStorage(this->GetDataStorage());

        mitk::TNodePredicateDataType<mitk::Image>::Pointer isImagePredicate = mitk::TNodePredicateDataType<mitk::Image>::New();

        mitk::NodePredicateProperty::Pointer isBinaryPredicate = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
        mitk::NodePredicateNot::Pointer isNotBinaryPredicate = mitk::NodePredicateNot::New( isBinaryPredicate );
        mitk::NodePredicateAnd::Pointer isNotABinaryImagePredicate = mitk::NodePredicateAnd::New( isImagePredicate, isNotBinaryPredicate );
        mitk::NodePredicateDimension::Pointer dimensionPredicate = mitk::NodePredicateDimension::New(3);

        m_Controls->m_FaImageBox->SetPredicate( mitk::NodePredicateAnd::New(isNotABinaryImagePredicate, dimensionPredicate) );
        m_Controls->m_FaImageBox->SetZeroEntryText("--");

        connect( m_Controls->commandLinkButton, SIGNAL(clicked()), this, SLOT(DoFiberTracking()) );
        connect( m_Controls->m_SeedsPerVoxelSlider, SIGNAL(valueChanged(int)), this, SLOT(OnSeedsPerVoxelChanged(int)) );
        connect( m_Controls->m_MinTractLengthSlider, SIGNAL(valueChanged(int)), this, SLOT(OnMinTractLengthChanged(int)) );
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

void QmitkStreamlineTrackingView::OnSeedsPerVoxelChanged(int value)
{
    m_Controls->m_SeedsPerVoxelLabel->setText(QString("Seeds per Voxel: ")+QString::number(value));
}

void QmitkStreamlineTrackingView::OnMinTractLengthChanged(int value)
{
    m_Controls->m_MinTractLengthLabel->setText(QString("Min. Tract Length: ")+QString::number(value)+QString("mm"));
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
    m_InputImageNodes.clear();
    m_InputImages.clear();
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
            bool isBinary = false;
            node->GetPropertyValue<bool>("binary", isBinary);

            if (isBinary)
            {
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
            else if( dynamic_cast<mitk::TensorImage*>(node->GetData()) )
            {
                m_InputImageNodes.push_back(node);
                m_InputImages.push_back(dynamic_cast<mitk::Image*>(node->GetData()));
            }
            else
            {
                mitk::Image* img = dynamic_cast<mitk::Image*>(node->GetData());
                if (img!=nullptr)
                {
                    int dim = img->GetDimension();
                    unsigned int* dimensions = img->GetDimensions();
                    if (dim==4 && dimensions[3]%3==0)
                    {
                        m_InputImageNodes.push_back(node);
                        m_InputImages.push_back(dynamic_cast<mitk::Image*>(node->GetData()));
                    }
                }
            }
        }
    }

    if(!m_InputImageNodes.empty())
    {
        if (m_InputImageNodes.size()>1)
            m_Controls->m_TensorImageLabel->setText(m_InputImageNodes.size()+" images selected");
        else
            m_Controls->m_TensorImageLabel->setText(m_InputImageNodes.at(0)->GetName().c_str());
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
    if (m_InputImages.empty())
        return;

    mitk::TrackingDataHandler* trackingHandler;

    try {
        typedef mitk::ImageToItk< mitk::TrackingHandlerPeaks::PeakImgType > CasterType;
        CasterType::Pointer caster = CasterType::New();
        caster->SetInput(m_InputImages.at(0));
        caster->Update();
        mitk::TrackingHandlerPeaks::PeakImgType::Pointer itkImg = caster->GetOutput();

        trackingHandler = new mitk::TrackingHandlerPeaks();

        dynamic_cast<mitk::TrackingHandlerPeaks*>(trackingHandler)->SetPeakImage(itkImg);
        dynamic_cast<mitk::TrackingHandlerPeaks*>(trackingHandler)->SetPeakThreshold(m_Controls->m_ScalarThresholdBox->value());
    }
    catch(...)
    {
        typedef itk::Image< itk::DiffusionTensor3D<float>, 3> TensorImageType;
        typedef mitk::ImageToItk<TensorImageType> CasterType;

        trackingHandler = new mitk::TrackingHandlerTensor();
        for (int i=0; i<(int)m_InputImages.size(); i++)
        {
            TensorImageType::Pointer itkImg = TensorImageType::New();
            mitk::CastToItkImage(m_InputImages.at(i), itkImg);

            dynamic_cast<mitk::TrackingHandlerTensor*>(trackingHandler)->AddTensorImage(itkImg);
        }

        if (m_Controls->m_FaImageBox->GetSelectedNode().IsNotNull())
        {
            ItkFloatImageType::Pointer itkImg = ItkFloatImageType::New();
            mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_Controls->m_FaImageBox->GetSelectedNode()->GetData()), itkImg);

            dynamic_cast<mitk::TrackingHandlerTensor*>(trackingHandler)->SetFaImage(itkImg);
        }

        dynamic_cast<mitk::TrackingHandlerTensor*>(trackingHandler)->SetFaThreshold(m_Controls->m_ScalarThresholdBox->value());
        dynamic_cast<mitk::TrackingHandlerTensor*>(trackingHandler)->SetF((float)m_Controls->m_fSlider->value()/100);
        dynamic_cast<mitk::TrackingHandlerTensor*>(trackingHandler)->SetG((float)m_Controls->m_gSlider->value()/100);
    }

    trackingHandler->SetFlipX(m_Controls->m_FlipXBox->isChecked());
    trackingHandler->SetFlipY(m_Controls->m_FlipYBox->isChecked());
    trackingHandler->SetFlipZ(m_Controls->m_FlipZBox->isChecked());
    trackingHandler->SetInterpolate(m_Controls->m_InterpolationBox->isChecked());

    typedef itk::StreamlineTrackingFilter TrackerType;
    TrackerType::Pointer tracker = TrackerType::New();

    if (m_SeedRoi.IsNotNull())
    {
        ItkUCharImageType::Pointer mask = ItkUCharImageType::New();
        mitk::CastToItkImage(m_SeedRoi, mask);
        tracker->SetSeedImage(mask);
    }

    if (m_MaskImage.IsNotNull())
    {
        ItkUCharImageType::Pointer mask = ItkUCharImageType::New();
        mitk::CastToItkImage(m_MaskImage, mask);
        tracker->SetMaskImage(mask);
    }

    //tracker->SetStoppingRegions(stop);
    tracker->SetSeedsPerVoxel(m_Controls->m_SeedsPerVoxelSlider->value());
    tracker->SetStepSize(m_Controls->m_StepSizeBox->value());
    tracker->SetSamplingDistance(0.7);
    tracker->SetUseStopVotes(true);
    tracker->SetOnlyForwardSamples(true);
    tracker->SetAposterioriCurvCheck(false);
    //tracker->SetFourTTImage(tissue);
    tracker->SetNumberOfSamples(m_Controls->m_NumSamplesBox->value());
    tracker->SetSeedOnlyGm(false);
    tracker->SetTrackingHandler(trackingHandler);
    tracker->SetAngularThreshold(m_Controls->m_AngularThresholdBox->value());
    tracker->SetMinTractLength(m_Controls->m_MinTractLengthSlider->value());
    tracker->Update();

    vtkSmartPointer<vtkPolyData> fiberBundle = tracker->GetFiberPolyData();
    if ( fiberBundle->GetNumberOfLines()==0 )
    {
        QMessageBox warnBox;
        warnBox.setWindowTitle("Warning");
        warnBox.setText("No fiberbundle was generated!");
        warnBox.setDetailedText("No fibers were generated using the parameters: \n\n" + m_Controls->m_FaThresholdLabel->text() + "\n" + m_Controls->m_AngularThresholdLabel->text() + "\n" + m_Controls->m_fLabel->text() + "\n" + m_Controls->m_gLabel->text() + "\n" + m_Controls->m_StepsizeLabel->text() + "\n" + m_Controls->m_MinTractLengthLabel->text() + "\n" + m_Controls->m_SeedsPerVoxelLabel->text() + "\n\nPlease check your parametersettings.");
        warnBox.setIcon(QMessageBox::Warning);
        warnBox.exec();
        return;
    }
    mitk::FiberBundle::Pointer fib = mitk::FiberBundle::New(fiberBundle);
    fib->SetReferenceGeometry(dynamic_cast<mitk::Image*>(m_InputImageNodes.at(0)->GetData())->GetGeometry());
    if (m_Controls->m_ResampleFibersBox->isChecked())
        fib->Compress(m_Controls->m_FiberErrorBox->value());
    fib->ColorFibersByOrientation();

    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(fib);
    QString name("FiberBundle_");
    name += m_InputImageNodes.at(0)->GetName().c_str();
    name += "_Streamline";
    node->SetName(name.toStdString());
    node->SetVisibility(true);
    GetDataStorage()->Add(node, m_InputImageNodes.at(0));

    delete trackingHandler;
}


