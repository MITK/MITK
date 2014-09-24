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
#include "QmitkStochasticFiberTrackingView.h"
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


const std::string QmitkStochasticFiberTrackingView::VIEW_ID = "org.mitk.views.stochasticfibertracking";
const std::string id_DataManager = "org.mitk.views.datamanager";
using namespace berry;

QmitkStochasticFiberTrackingView::QmitkStochasticFiberTrackingView()
    : QmitkFunctionality()
    , m_Controls( 0 )
    , m_MultiWidget( NULL )
    , m_SeedRoi( NULL )
    , m_DiffusionImage( NULL )
{
}

// Destructor
QmitkStochasticFiberTrackingView::~QmitkStochasticFiberTrackingView()
{

}

void QmitkStochasticFiberTrackingView::CreateQtPartControl( QWidget *parent )
{
    if ( !m_Controls )
    {
        // create GUI widgets from the Qt Designer's .ui file
        m_Controls = new Ui::QmitkStochasticFiberTrackingViewControls;
        m_Controls->setupUi( parent );

        connect( m_Controls->commandLinkButton, SIGNAL(clicked()), this, SLOT(DoFiberTracking()) );
        connect( m_Controls->m_SeedsPerVoxelSlider, SIGNAL(valueChanged(int)), this, SLOT(OnSeedsPerVoxelChanged(int)) );
        connect( m_Controls->m_MaxCacheSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(OnMaxCacheSizeChanged(int)) );
        connect( m_Controls->m_MaxTractLengthSlider, SIGNAL(valueChanged(int)), this, SLOT(OnMaxTractLengthChanged(int)) );
    }
}

void QmitkStochasticFiberTrackingView::OnSeedsPerVoxelChanged(int value)
{
    m_Controls->m_SeedsPerVoxelLabel->setText(QString("Seeds per Voxel: ")+QString::number(value));
}

void QmitkStochasticFiberTrackingView::OnMaxTractLengthChanged(int value)
{
    m_Controls->m_MaxTractLengthLabel->setText(QString("Max. Tract Length: ")+QString::number(value));
}

void QmitkStochasticFiberTrackingView::OnMaxCacheSizeChanged(int value)
{
    m_Controls->m_MaxCacheSizeLabel->setText(QString("Max. Cache Size: ")+QString::number(value)+"GB");
}

void QmitkStochasticFiberTrackingView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
    m_MultiWidget = &stdMultiWidget;
}


void QmitkStochasticFiberTrackingView::StdMultiWidgetNotAvailable()
{
    m_MultiWidget = NULL;
}

void QmitkStochasticFiberTrackingView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
    m_DiffusionImageNode = NULL;
    m_DiffusionImage = NULL;
    m_SeedRoi = NULL;
    m_Controls->m_DiffusionImageLabel->setText("<font color='red'>mandatory</font>");
    m_Controls->m_RoiImageLabel->setText("<font color='red'>mandatory</font>");

    for( std::vector<mitk::DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it )
    {
        mitk::DataNode::Pointer node = *it;

        if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
        {
            if( dynamic_cast<mitk::DiffusionImage<short>*>(node->GetData()) )
            {
                m_DiffusionImageNode = node;
                m_DiffusionImage = dynamic_cast<mitk::DiffusionImage<short>*>(node->GetData());
                m_Controls->m_DiffusionImageLabel->setText(node->GetName().c_str());
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

    if(m_DiffusionImage.IsNotNull() && m_SeedRoi.IsNotNull())
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



void QmitkStochasticFiberTrackingView::DoFiberTracking()
{
    typedef itk::VectorImage< short int, 3 >    DWIVectorImageType;
    typedef itk::Image< float, 3 >              FloatImageType;
    typedef itk::Image< unsigned int, 3 >       CImageType;
    typedef itk::StochasticTractographyFilter< DWIVectorImageType, FloatImageType, CImageType > TrackingFilterType;
    typedef itk::DTITubeSpatialObject<3>        DTITubeType;
    typedef itk::DTITubeSpatialObjectPoint<3>   DTITubePointType;
    typedef itk::SceneSpatialObject<3>          SceneSpatialObjectType;

    /* get Gradients/Direction of dwi */
    itk::VectorContainer< unsigned int, vnl_vector_fixed<double,3> >::Pointer Pdir = m_DiffusionImage->GetDirections();

    /* bValueContainer, Container includes b-values according to corresponding gradient-direction*/
    TrackingFilterType::bValueContainerType::Pointer vecCont = TrackingFilterType::bValueContainerType::New();

    /* for each gradient set b-Value; for 0-gradient set b-value eq. 0 */
    for ( int i=0; i<(int)Pdir->size(); ++i)
    {
        vnl_vector_fixed<double,3> valsGrad = Pdir->at(i);
        if (valsGrad.get(0) == 0 && valsGrad.get(1) == 0 && valsGrad.get(2) == 0)
        {  //set 0-Gradient to bValue 0
            vecCont->InsertElement(i,0);
        }else{
            vecCont->InsertElement(i,m_DiffusionImage->GetReferenceBValue());
        }
    }

    /* define measurement frame (identity-matrix 3x3) */
    TrackingFilterType::MeasurementFrameType measurement_frame = m_DiffusionImage->GetMeasurementFrame();

    /* generate white matterImage (dummy?)*/
    FloatImageType::Pointer wmImage = FloatImageType::New();
    wmImage->SetSpacing( m_DiffusionImage->GetVectorImage()->GetSpacing() );
    wmImage->SetOrigin( m_DiffusionImage->GetVectorImage()->GetOrigin() );
    wmImage->SetDirection( m_DiffusionImage->GetVectorImage()->GetDirection() );
    wmImage->SetLargestPossibleRegion( m_DiffusionImage->GetVectorImage()->GetLargestPossibleRegion() );
    wmImage->SetBufferedRegion( wmImage->GetLargestPossibleRegion() );
    wmImage->SetRequestedRegion( wmImage->GetLargestPossibleRegion() );
    wmImage->Allocate();

    itk::ImageRegionIterator<FloatImageType> ot(wmImage, wmImage->GetLargestPossibleRegion() );
    while (!ot.IsAtEnd())
    {
        ot.Set(1);
        ++ot;
    }

    /* init TractographyFilter */
    TrackingFilterType::Pointer trackingFilter = TrackingFilterType::New();
    trackingFilter->SetPrimaryInput(m_DiffusionImage->GetVectorImage().GetPointer());
    trackingFilter->SetbValues(vecCont);
    trackingFilter->SetGradients(Pdir);
    trackingFilter->SetMeasurementFrame(measurement_frame);
    trackingFilter->SetWhiteMatterProbabilityImage(wmImage);
    trackingFilter->SetTotalTracts(m_Controls->m_SeedsPerVoxelSlider->value());
    trackingFilter->SetMaxLikelihoodCacheSize(m_Controls->m_MaxCacheSizeSlider->value()*1000);
    trackingFilter->SetMaxTractLength(m_Controls->m_MaxTractLengthSlider->value());

    //itk::Image< char, 3 >
    mitk::ImageToItk< itk::Image< unsigned char, 3 > >::Pointer binaryImageToItk1 = mitk::ImageToItk< itk::Image< unsigned char, 3 > >::New();
    binaryImageToItk1->SetInput( m_SeedRoi );
    binaryImageToItk1->Update();

    vtkSmartPointer<vtkPoints> vPoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vCellArray = vtkSmartPointer<vtkCellArray>::New();

    itk::ImageRegionConstIterator< BinaryImageType > it(binaryImageToItk1->GetOutput(), binaryImageToItk1->GetOutput()->GetRequestedRegion());
    it.GoToBegin();
    mitk::BaseGeometry* geom = m_DiffusionImage->GetGeometry();

    while(!it.IsAtEnd())
    {
        itk::ImageConstIterator<BinaryImageType>::PixelType tmpPxValue = it.Get();

        if(tmpPxValue != 0){
            mitk::Point3D point;
            itk::ImageRegionConstIterator< BinaryImageType >::IndexType seedIdx = it.GetIndex();
            trackingFilter->SetSeedIndex(seedIdx);
            trackingFilter->Update();


            /* get results from Filter */
            /* write each single tract into member container */
            TrackingFilterType::TractContainerType::Pointer container_tmp = trackingFilter->GetOutputTractContainer();
            TrackingFilterType::TractContainerType::Iterator elIt = container_tmp->Begin();
            TrackingFilterType::TractContainerType::Iterator end = container_tmp->End();
            bool addTract = true;

            while( elIt != end ){
                TrackingFilterType::TractContainerType::Element tract = elIt.Value();
                TrackingFilterType::TractContainerType::Element::ObjectType::VertexListType::ConstPointer vertexlist = tract->GetVertexList();

                vtkSmartPointer<vtkPolyLine> vPolyLine = vtkSmartPointer<vtkPolyLine>::New();
                for( int j=0; j<(int)vertexlist->Size(); j++)
                {
                    TrackingFilterType::TractContainerType::Element::ObjectType::VertexListType::Element vertex = vertexlist->GetElement(j);
                    mitk::Point3D index;
                    index[0] = (float)vertex[0];
                    index[1] = (float)vertex[1];
                    index[2] = (float)vertex[2];

                    if (geom->IsIndexInside(index))
                    {
                        geom->IndexToWorld(index, point);
                        vtkIdType id = vPoints->InsertNextPoint(point.GetDataPointer());
                        vPolyLine->GetPointIds()->InsertNextId(id);
                    }
                    else
                    {
                        addTract = false;
                        break;
                    }
                }

                if (addTract)
                    vCellArray->InsertNextCell(vPolyLine);

                ++elIt;
            }
        }
        ++it;
    }

    vtkSmartPointer<vtkPolyData> fiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    fiberPolyData->SetPoints(vPoints);
    fiberPolyData->SetLines(vCellArray);

    mitk::FiberBundleX::Pointer fib = mitk::FiberBundleX::New(fiberPolyData);
    fib->SetReferenceGeometry(dynamic_cast<mitk::Image*>(m_DiffusionImageNode->GetData())->GetGeometry());
    mitk::DataNode::Pointer fbNode = mitk::DataNode::New();
    fbNode->SetData(fib);
    QString name("FiberBundle_");
    name += m_DiffusionImageNode->GetName().c_str();
    name += "_Probabilistic";
    fbNode->SetName(name.toStdString());
    fbNode->SetVisibility(true);
    GetDataStorage()->Add(fbNode, m_DiffusionImageNode);
}


