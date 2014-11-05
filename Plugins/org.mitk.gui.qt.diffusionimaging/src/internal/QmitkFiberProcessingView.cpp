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

// Qmitk
#include "QmitkFiberProcessingView.h"
#include <QmitkStdMultiWidget.h>

// Qt
#include <QMessageBox>

// MITK
#include <mitkNodePredicateProperty.h>
#include <mitkImageCast.h>
#include <mitkPointSet.h>
#include <mitkPlanarCircle.h>
#include <mitkPlanarPolygon.h>
#include <mitkPlanarRectangle.h>
#include <mitkPlanarFigureInteractor.h>
#include <mitkGlobalInteraction.h>
#include <mitkImageAccessByItk.h>
#include <mitkDataNodeObject.h>
#include <mitkDiffusionImage.h>
#include <mitkTensorImage.h>

// ITK
#include <itkResampleImageFilter.h>
#include <itkGaussianInterpolateImageFunction.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkTractsToFiberEndingsImageFilter.h>
#include <itkTractDensityImageFilter.h>
#include <itkImageRegion.h>
#include <itkTractsToRgbaImageFilter.h>
#include <itkTractsToVectorImageFilter.h>

#include <math.h>
#include <boost/lexical_cast.hpp>

const std::string QmitkFiberProcessingView::VIEW_ID = "org.mitk.views.fiberprocessing";
const std::string id_DataManager = "org.mitk.views.datamanager";
using namespace mitk;

QmitkFiberProcessingView::QmitkFiberProcessingView()
    : QmitkFunctionality()
    , m_Controls( 0 )
    , m_MultiWidget( NULL )
    , m_UpsamplingFactor(5)
{

}

// Destructor
QmitkFiberProcessingView::~QmitkFiberProcessingView()
{

}

void QmitkFiberProcessingView::CreateQtPartControl( QWidget *parent )
{
    // build up qt view, unless already done
    if ( !m_Controls )
    {
        // create GUI widgets from the Qt Designer's .ui file
        m_Controls = new Ui::QmitkFiberProcessingViewControls;
        m_Controls->setupUi( parent );

        connect( m_Controls->m_ProcessFiberBundleButton, SIGNAL(clicked()), this, SLOT(ProcessSelectedBundles()) );
        connect( m_Controls->m_ResampleFibersButton, SIGNAL(clicked()), this, SLOT(ResampleSelectedBundles()) );
        connect(m_Controls->m_FaColorFibersButton, SIGNAL(clicked()), this, SLOT(DoImageColorCoding()));
        connect( m_Controls->m_PruneFibersButton, SIGNAL(clicked()), this, SLOT(PruneBundle()) );
        connect( m_Controls->m_CurvatureThresholdButton, SIGNAL(clicked()), this, SLOT(ApplyCurvatureThreshold()) );
        connect( m_Controls->m_MirrorFibersButton, SIGNAL(clicked()), this, SLOT(MirrorFibers()) );
        connect( m_Controls->m_CompressFibersButton, SIGNAL(clicked()), this, SLOT(CompressSelectedBundles()) );
        connect( m_Controls->m_ExtractFiberPeaks, SIGNAL(clicked()), this, SLOT(CalculateFiberDirections()) );
    }
}

void QmitkFiberProcessingView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
    m_MultiWidget = &stdMultiWidget;
}


void QmitkFiberProcessingView::StdMultiWidgetNotAvailable()
{
    m_MultiWidget = NULL;
}

void QmitkFiberProcessingView::CalculateFiberDirections()
{
    typedef itk::Image<unsigned char, 3>                                            ItkUcharImgType;
    typedef itk::Image< itk::Vector< float, 3>, 3 >                                 ItkDirectionImage3DType;
    typedef itk::VectorContainer< unsigned int, ItkDirectionImage3DType::Pointer >  ItkDirectionImageContainerType;

    // load fiber bundle
    mitk::FiberBundleX::Pointer inputTractogram = dynamic_cast<mitk::FiberBundleX*>(m_SelectedFB.back()->GetData());

    itk::TractsToVectorImageFilter<float>::Pointer fOdfFilter = itk::TractsToVectorImageFilter<float>::New();
    if (m_SelectedImage.IsNotNull())
    {
        ItkUcharImgType::Pointer itkMaskImage = ItkUcharImgType::New();
        mitk::CastToItkImage<ItkUcharImgType>(m_SelectedImage, itkMaskImage);
        fOdfFilter->SetMaskImage(itkMaskImage);
    }

    // extract directions from fiber bundle
    fOdfFilter->SetFiberBundle(inputTractogram);
    fOdfFilter->SetAngularThreshold(cos(m_Controls->m_AngularThreshold->value()*M_PI/180));
    fOdfFilter->SetNormalizeVectors(m_Controls->m_NormalizeDirectionsBox->isChecked());
    fOdfFilter->SetUseWorkingCopy(true);
    fOdfFilter->SetCreateDirectionImages(m_Controls->m_DirectionImagesBox->isChecked());
    fOdfFilter->SetSizeThreshold(m_Controls->m_PeakThreshold->value());
    fOdfFilter->SetMaxNumDirections(m_Controls->m_MaxNumDirections->value());
    fOdfFilter->Update();

    QString name = m_SelectedFB.back()->GetName().c_str();

    if (m_Controls->m_VectorFieldBox->isChecked())
    {
        float minSpacing = 1;
        if (m_SelectedImage.IsNotNull())
        {
            mitk::Vector3D outImageSpacing = m_SelectedImage->GetGeometry()->GetSpacing();

            if(outImageSpacing[0]<outImageSpacing[1] && outImageSpacing[0]<outImageSpacing[2])
                minSpacing = outImageSpacing[0];
            else if (outImageSpacing[1] < outImageSpacing[2])
                minSpacing = outImageSpacing[1];
            else
                minSpacing = outImageSpacing[2];
        }

        mitk::FiberBundleX::Pointer directions = fOdfFilter->GetOutputFiberBundle();
        mitk::DataNode::Pointer node = mitk::DataNode::New();
        node->SetData(directions);
        node->SetName((name+"_vectorfield").toStdString().c_str());
        node->SetProperty("Fiber2DSliceThickness", mitk::FloatProperty::New(minSpacing));
        node->SetProperty("Fiber2DfadeEFX", mitk::BoolProperty::New(false));
        node->SetProperty("color", mitk::ColorProperty::New(1.0f, 1.0f, 1.0f));

        GetDefaultDataStorage()->Add(node, m_SelectedFB.back());
    }

    if (m_Controls->m_NumDirectionsBox->isChecked())
    {
        mitk::Image::Pointer mitkImage = mitk::Image::New();
        mitkImage->InitializeByItk( fOdfFilter->GetNumDirectionsImage().GetPointer() );
        mitkImage->SetVolume( fOdfFilter->GetNumDirectionsImage()->GetBufferPointer() );

        mitk::DataNode::Pointer node = mitk::DataNode::New();
        node->SetData(mitkImage);
        node->SetName((name+"_numdirections").toStdString().c_str());
        GetDefaultDataStorage()->Add(node, m_SelectedFB.back());
    }

    if (m_Controls->m_DirectionImagesBox->isChecked())
    {
        ItkDirectionImageContainerType::Pointer directionImageContainer = fOdfFilter->GetDirectionImageContainer();
        for (unsigned int i=0; i<directionImageContainer->Size(); i++)
        {
            itk::TractsToVectorImageFilter<float>::ItkDirectionImageType::Pointer itkImg = directionImageContainer->GetElement(i);

            if (itkImg.IsNull())
                return;

            mitk::Image::Pointer mitkImage = mitk::Image::New();
            mitkImage->InitializeByItk( itkImg.GetPointer() );
            mitkImage->SetVolume( itkImg->GetBufferPointer() );

            mitk::DataNode::Pointer node = mitk::DataNode::New();
            node->SetData(mitkImage);
            node->SetName( (name+"_direction_"+boost::lexical_cast<std::string>(i).c_str()).toStdString().c_str());
            node->SetVisibility(false);
            GetDefaultDataStorage()->Add(node, m_SelectedFB.back());
        }
    }
}

void QmitkFiberProcessingView::UpdateGui()
{
    m_Controls->m_CompressFibersButton->setEnabled(!m_SelectedFB.empty());
    m_Controls->m_ProcessFiberBundleButton->setEnabled(!m_SelectedFB.empty());
    m_Controls->m_ResampleFibersButton->setEnabled(!m_SelectedFB.empty());
    m_Controls->m_FaColorFibersButton->setEnabled(false);
    m_Controls->m_PruneFibersButton->setEnabled(!m_SelectedFB.empty());
    m_Controls->m_CurvatureThresholdButton->setEnabled(!m_SelectedFB.empty());
    m_Controls->m_ExtractFiberPeaks->setEnabled(!m_SelectedFB.empty());

    // are fiber bundles selected?
    if ( m_SelectedFB.empty() )
    {
        if (m_SelectedSurfaces.size()>0 )
            m_Controls->m_MirrorFibersButton->setEnabled(true);
        else
            m_Controls->m_MirrorFibersButton->setEnabled(false);
    }
    else
    {
        m_Controls->m_MirrorFibersButton->setEnabled(true);
        if (m_SelectedImage.IsNotNull())
            m_Controls->m_FaColorFibersButton->setEnabled(true);
    }
}

void QmitkFiberProcessingView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
    //reset existing Vectors containing FiberBundles and PlanarFigures from a previous selection
    m_SelectedFB.clear();
    m_SelectedSurfaces.clear();
    m_SelectedImage = NULL;

    for( std::vector<mitk::DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it )
    {
        mitk::DataNode::Pointer node = *it;
        if ( dynamic_cast<mitk::FiberBundleX*>(node->GetData()) )
        {
            m_SelectedFB.push_back(node);
        }
        else if (dynamic_cast<mitk::Image*>(node->GetData()))
            m_SelectedImage = dynamic_cast<mitk::Image*>(node->GetData());
        else if (dynamic_cast<mitk::Surface*>(node->GetData()))
        {
            m_SelectedSurfaces.push_back(dynamic_cast<mitk::Surface*>(node->GetData()));
        }
    }
    UpdateGui();
    GenerateStats();
}

void QmitkFiberProcessingView::Activated()
{

}

void QmitkFiberProcessingView::PruneBundle()
{
    int minLength = this->m_Controls->m_PruneFibersSpinBox->value();
    int maxLength = this->m_Controls->m_MaxPruneFibersSpinBox->value();
    for (int i=0; i<m_SelectedFB.size(); i++)
    {
        mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>(m_SelectedFB.at(i)->GetData());
        if (!fib->RemoveShortFibers(minLength))
            QMessageBox::information(NULL, "No output generated:", "The resulting fiber bundle contains no fibers.");
        else if (!fib->RemoveLongFibers(maxLength))
            QMessageBox::information(NULL, "No output generated:", "The resulting fiber bundle contains no fibers.");
    }
    GenerateStats();
    RenderingManager::GetInstance()->RequestUpdateAll();
}


void QmitkFiberProcessingView::ApplyCurvatureThreshold()
{
    int mm = this->m_Controls->m_MinCurvatureRadiusBox->value();
    for (int i=0; i<m_SelectedFB.size(); i++)
    {
        mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>(m_SelectedFB.at(i)->GetData());
        if (!fib->ApplyCurvatureThreshold(mm, this->m_Controls->m_RemoveFiberDueToCurvatureCheckbox->isChecked()))
            QMessageBox::information(NULL, "No output generated:", "The resulting fiber bundle contains no fibers.");
    }
    GenerateStats();
    RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberProcessingView::GenerateStats()
{
    if ( m_SelectedFB.empty() )
        return;

    QString stats("");

    for( int i=0; i<m_SelectedFB.size(); i++ )
    {
        mitk::DataNode::Pointer node = m_SelectedFB[i];
        if (node.IsNotNull() && dynamic_cast<mitk::FiberBundleX*>(node->GetData()))
        {
            if (i>0)
                stats += "\n-----------------------------\n";
            stats += QString(node->GetName().c_str()) + "\n";
            mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>(node->GetData());
            stats += "Number of fibers: "+ QString::number(fib->GetNumFibers()) + "\n";
            stats += "Number of points: "+ QString::number(fib->GetNumberOfPoints()) + "\n";
            stats += "Min. length:         "+ QString::number(fib->GetMinFiberLength(),'f',1) + " mm\n";
            stats += "Max. length:         "+ QString::number(fib->GetMaxFiberLength(),'f',1) + " mm\n";
            stats += "Mean length:         "+ QString::number(fib->GetMeanFiberLength(),'f',1) + " mm\n";
            stats += "Median length:       "+ QString::number(fib->GetMedianFiberLength(),'f',1) + " mm\n";
            stats += "Standard deviation:  "+ QString::number(fib->GetLengthStDev(),'f',1) + " mm\n";
        }
    }
    this->m_Controls->m_StatsTextEdit->setText(stats);
}

void QmitkFiberProcessingView::ProcessSelectedBundles()
{
    if ( m_SelectedFB.empty() ){
        QMessageBox::information( NULL, "Warning", "No fibe bundle selected!");
        MITK_WARN("QmitkFiberProcessingView") << "no fibe bundle selected";
        return;
    }

    int generationMethod = m_Controls->m_GenerationBox->currentIndex();

    for( int i=0; i<m_SelectedFB.size(); i++ )
    {
        mitk::DataNode::Pointer node = m_SelectedFB[i];
        if (node.IsNotNull() && dynamic_cast<mitk::FiberBundleX*>(node->GetData()))
        {
            mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>(node->GetData());
            QString name(node->GetName().c_str());
            DataNode::Pointer newNode = NULL;
            switch(generationMethod){
            case 0:
                newNode = GenerateTractDensityImage(fib, false, true);
                name += "_TDI";
                break;
            case 1:
                newNode = GenerateTractDensityImage(fib, false, false);
                name += "_TDI";
                break;
            case 2:
                newNode = GenerateTractDensityImage(fib, true, false);
                name += "_envelope";
                break;
            case 3:
                newNode = GenerateColorHeatmap(fib);
                break;
            case 4:
                newNode = GenerateFiberEndingsImage(fib);
                name += "_fiber_endings";
                break;
            case 5:
                newNode = GenerateFiberEndingsPointSet(fib);
                name += "_fiber_endings";
                break;
            }
            if (newNode.IsNotNull())
            {
                newNode->SetName(name.toStdString());
                GetDataStorage()->Add(newNode);
            }
        }
    }
}

// generate pointset displaying the fiber endings
mitk::DataNode::Pointer QmitkFiberProcessingView::GenerateFiberEndingsPointSet(mitk::FiberBundleX::Pointer fib)
{
    mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
    vtkSmartPointer<vtkPolyData> fiberPolyData = fib->GetFiberPolyData();
    vtkSmartPointer<vtkCellArray> vLines = fiberPolyData->GetLines();
    vLines->InitTraversal();

    int count = 0;
    int numFibers = fib->GetNumFibers();
    for( int i=0; i<numFibers; i++ )
    {
        vtkIdType   numPoints(0);
        vtkIdType*  points(NULL);
        vLines->GetNextCell ( numPoints, points );

        if (numPoints>0)
        {
            double* point = fiberPolyData->GetPoint(points[0]);
            itk::Point<float,3> itkPoint;
            itkPoint[0] = point[0];
            itkPoint[1] = point[1];
            itkPoint[2] = point[2];
            pointSet->InsertPoint(count, itkPoint);
            count++;
        }
        if (numPoints>2)
        {
            double* point = fiberPolyData->GetPoint(points[numPoints-1]);
            itk::Point<float,3> itkPoint;
            itkPoint[0] = point[0];
            itkPoint[1] = point[1];
            itkPoint[2] = point[2];
            pointSet->InsertPoint(count, itkPoint);
            count++;
        }
    }

    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData( pointSet );
    return node;
}

// generate image displaying the fiber endings
mitk::DataNode::Pointer QmitkFiberProcessingView::GenerateFiberEndingsImage(mitk::FiberBundleX::Pointer fib)
{
    typedef unsigned char OutPixType;
    typedef itk::Image<OutPixType,3> OutImageType;

    typedef itk::TractsToFiberEndingsImageFilter< OutImageType > ImageGeneratorType;
    ImageGeneratorType::Pointer generator = ImageGeneratorType::New();
    generator->SetFiberBundle(fib);
    generator->SetUpsamplingFactor(m_Controls->m_UpsamplingSpinBox->value());
    if (m_SelectedImage.IsNotNull())
    {
        OutImageType::Pointer itkImage = OutImageType::New();
        CastToItkImage(m_SelectedImage, itkImage);
        generator->SetInputImage(itkImage);
        generator->SetUseImageGeometry(true);
    }
    generator->Update();

    // get output image
    OutImageType::Pointer outImg = generator->GetOutput();
    mitk::Image::Pointer img = mitk::Image::New();
    img->InitializeByItk(outImg.GetPointer());
    img->SetVolume(outImg->GetBufferPointer());

    // init data node
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(img);
    return node;
}

// generate rgba heatmap from fiber bundle
mitk::DataNode::Pointer QmitkFiberProcessingView::GenerateColorHeatmap(mitk::FiberBundleX::Pointer fib)
{
    typedef itk::RGBAPixel<unsigned char> OutPixType;
    typedef itk::Image<OutPixType, 3> OutImageType;
    typedef itk::TractsToRgbaImageFilter< OutImageType > ImageGeneratorType;
    ImageGeneratorType::Pointer generator = ImageGeneratorType::New();
    generator->SetFiberBundle(fib);
    generator->SetUpsamplingFactor(m_Controls->m_UpsamplingSpinBox->value());
    if (m_SelectedImage.IsNotNull())
    {
        itk::Image<unsigned char, 3>::Pointer itkImage = itk::Image<unsigned char, 3>::New();
        CastToItkImage(m_SelectedImage, itkImage);
        generator->SetInputImage(itkImage);
        generator->SetUseImageGeometry(true);
    }
    generator->Update();

    // get output image
    typedef itk::Image<OutPixType,3> OutType;
    OutType::Pointer outImg = generator->GetOutput();
    mitk::Image::Pointer img = mitk::Image::New();
    img->InitializeByItk(outImg.GetPointer());
    img->SetVolume(outImg->GetBufferPointer());

    // init data node
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(img);
    return node;
}

// generate tract density image from fiber bundle
mitk::DataNode::Pointer QmitkFiberProcessingView::GenerateTractDensityImage(mitk::FiberBundleX::Pointer fib, bool binary, bool absolute)
{
    typedef float OutPixType;
    typedef itk::Image<OutPixType, 3> OutImageType;

    itk::TractDensityImageFilter< OutImageType >::Pointer generator = itk::TractDensityImageFilter< OutImageType >::New();
    generator->SetFiberBundle(fib);
    generator->SetBinaryOutput(binary);
    generator->SetOutputAbsoluteValues(absolute);
    generator->SetUpsamplingFactor(m_Controls->m_UpsamplingSpinBox->value());
    if (m_SelectedImage.IsNotNull())
    {
        OutImageType::Pointer itkImage = OutImageType::New();
        CastToItkImage(m_SelectedImage, itkImage);
        generator->SetInputImage(itkImage);
        generator->SetUseImageGeometry(true);

    }
    generator->Update();

    // get output image
    typedef itk::Image<OutPixType,3> OutType;
    OutType::Pointer outImg = generator->GetOutput();
    mitk::Image::Pointer img = mitk::Image::New();
    img->InitializeByItk(outImg.GetPointer());
    img->SetVolume(outImg->GetBufferPointer());

    // init data node
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(img);
    return node;
}

void QmitkFiberProcessingView::ResampleSelectedBundles()
{
    double factor = this->m_Controls->m_ResampleFibersSpinBox->value();
    for (int i=0; i<m_SelectedFB.size(); i++)
    {
        mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>(m_SelectedFB.at(i)->GetData());
        fib->ResampleSpline(factor);
    }
    GenerateStats();
    RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberProcessingView::CompressSelectedBundles()
{
    double factor = this->m_Controls->m_FiberErrorSpinBox->value();
    for (int i=0; i<m_SelectedFB.size(); i++)
    {
        mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>(m_SelectedFB.at(i)->GetData());
        fib->Compress(factor);
    }
    GenerateStats();
    RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberProcessingView::MirrorFibers()
{
    unsigned int axis = this->m_Controls->m_AxisSelectionBox->currentIndex();
    for (int i=0; i<m_SelectedFB.size(); i++)
    {
        mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>(m_SelectedFB.at(i)->GetData());
        fib->MirrorFibers(axis);
    }
    if (m_SelectedFB.size()>0)
        GenerateStats();

    if (m_SelectedSurfaces.size()>0)
    {
        for (int i=0; i<m_SelectedSurfaces.size(); i++)
        {
            mitk::Surface::Pointer surf = m_SelectedSurfaces.at(i);
            vtkSmartPointer<vtkPolyData> poly = surf->GetVtkPolyData();
            vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();

            for (int i=0; i<poly->GetNumberOfPoints(); i++)
            {
                double* point = poly->GetPoint(i);
                point[axis] *= -1;
                vtkNewPoints->InsertNextPoint(point);
            }
            poly->SetPoints(vtkNewPoints);
            surf->CalculateBoundingBox();
        }
    }

    RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberProcessingView::DoImageColorCoding()
{
    if (m_SelectedImage.IsNull())
        return;

    for( int i=0; i<m_SelectedFB.size(); i++ )
    {
        mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>(m_SelectedFB.at(i)->GetData());
        fib->SetFAMap(m_SelectedImage);
        fib->SetColorCoding(mitk::FiberBundleX::COLORCODING_FA_BASED);
        fib->DoColorCodingFaBased();
    }

    if(m_MultiWidget)
        m_MultiWidget->RequestUpdate();
}
