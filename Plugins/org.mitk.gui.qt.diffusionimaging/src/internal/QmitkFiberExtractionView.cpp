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
#include "QmitkFiberExtractionView.h"
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
#include "usModuleRegistry.h"

// ITK
#include <itkResampleImageFilter.h>
#include <itkGaussianInterpolateImageFunction.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkTractsToFiberEndingsImageFilter.h>
#include <itkTractDensityImageFilter.h>
#include <itkImageRegion.h>
#include <itkTractsToRgbaImageFilter.h>

#include <math.h>


const std::string QmitkFiberExtractionView::VIEW_ID = "org.mitk.views.fiberextraction";
const std::string id_DataManager = "org.mitk.views.datamanager";
using namespace mitk;

QmitkFiberExtractionView::QmitkFiberExtractionView()
    : QmitkFunctionality()
    , m_Controls( 0 )
    , m_MultiWidget( NULL )
    , m_CircleCounter(0)
    , m_PolygonCounter(0)
    , m_UpsamplingFactor(1)
{

}

// Destructor
QmitkFiberExtractionView::~QmitkFiberExtractionView()
{

}

void QmitkFiberExtractionView::CreateQtPartControl( QWidget *parent )
{
    // build up qt view, unless already done
    if ( !m_Controls )
    {
        // create GUI widgets from the Qt Designer's .ui file
        m_Controls = new Ui::QmitkFiberExtractionViewControls;
        m_Controls->setupUi( parent );
        m_Controls->doExtractFibersButton->setDisabled(true);
        m_Controls->PFCompoANDButton->setDisabled(true);
        m_Controls->PFCompoORButton->setDisabled(true);
        m_Controls->PFCompoNOTButton->setDisabled(true);
        m_Controls->m_PlanarFigureButtonsFrame->setEnabled(false);
        m_Controls->m_RectangleButton->setVisible(false);

        connect( m_Controls->m_CircleButton, SIGNAL( clicked() ), this, SLOT( OnDrawCircle() ) );
        connect( m_Controls->m_PolygonButton, SIGNAL( clicked() ), this, SLOT( OnDrawPolygon() ) );
        connect(m_Controls->PFCompoANDButton, SIGNAL(clicked()), this, SLOT(GenerateAndComposite()) );
        connect(m_Controls->PFCompoORButton, SIGNAL(clicked()), this, SLOT(GenerateOrComposite()) );
        connect(m_Controls->PFCompoNOTButton, SIGNAL(clicked()), this, SLOT(GenerateNotComposite()) );
        connect(m_Controls->m_JoinBundles, SIGNAL(clicked()), this, SLOT(JoinBundles()) );
        connect(m_Controls->m_SubstractBundles, SIGNAL(clicked()), this, SLOT(SubstractBundles()) );
        connect(m_Controls->m_GenerateRoiImage, SIGNAL(clicked()), this, SLOT(GenerateRoiImage()) );

        connect(m_Controls->m_Extract3dButton_2, SIGNAL(clicked()), this, SLOT(ExtractNotPassingMask()));
        connect(m_Controls->m_Extract3dButton, SIGNAL(clicked()), this, SLOT(ExtractPassingMask()));
        connect( m_Controls->m_ExtractMask, SIGNAL(clicked()), this, SLOT(ExtractEndingInMask()) );
        connect( m_Controls->doExtractFibersButton, SIGNAL(clicked()), this, SLOT(DoFiberExtraction()) );

        connect( m_Controls->m_RemoveOutsideMaskButton, SIGNAL(clicked()), this, SLOT(DoRemoveOutsideMask()));
        connect( m_Controls->m_RemoveInsideMaskButton, SIGNAL(clicked()), this, SLOT(DoRemoveInsideMask()));
    }
}

void QmitkFiberExtractionView::DoRemoveInsideMask()
{
    if (m_MaskImageNode.IsNull())
        return;

    mitk::Image::Pointer mitkMask = dynamic_cast<mitk::Image*>(m_MaskImageNode->GetData());
    for (unsigned int i=0; i<m_SelectedFB.size(); i++)
    {
        mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>(m_SelectedFB.at(i)->GetData());
        QString name(m_SelectedFB.at(i)->GetName().c_str());

        itkUCharImageType::Pointer mask = itkUCharImageType::New();
        mitk::CastToItkImage(mitkMask, mask);
        mitk::FiberBundleX::Pointer newFib = fib->RemoveFibersOutside(mask, true);
        if (newFib->GetNumFibers()<=0)
        {
            QMessageBox::information(NULL, "No output generated:", "The resulting fiber bundle contains no fibers.");
            continue;
        }
        DataNode::Pointer newNode = DataNode::New();
        newNode->SetData(newFib);
        name += "_Cut";
        newNode->SetName(name.toStdString());
        GetDefaultDataStorage()->Add(newNode);
        m_SelectedFB.at(i)->SetVisibility(false);
    }
}

void QmitkFiberExtractionView::DoRemoveOutsideMask()
{
    if (m_MaskImageNode.IsNull())
        return;

    mitk::Image::Pointer mitkMask = dynamic_cast<mitk::Image*>(m_MaskImageNode->GetData());
    for (unsigned int i=0; i<m_SelectedFB.size(); i++)
    {
        mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>(m_SelectedFB.at(i)->GetData());
        QString name(m_SelectedFB.at(i)->GetName().c_str());

        itkUCharImageType::Pointer mask = itkUCharImageType::New();
        mitk::CastToItkImage(mitkMask, mask);
        mitk::FiberBundleX::Pointer newFib = fib->RemoveFibersOutside(mask);
        if (newFib->GetNumFibers()<=0)
        {
            QMessageBox::information(NULL, "No output generated:", "The resulting fiber bundle contains no fibers.");
            continue;
        }
        DataNode::Pointer newNode = DataNode::New();
        newNode->SetData(newFib);
        name += "_Cut";
        newNode->SetName(name.toStdString());
        GetDefaultDataStorage()->Add(newNode);
        m_SelectedFB.at(i)->SetVisibility(false);
    }
}

void QmitkFiberExtractionView::ExtractEndingInMask()
{
    if (m_MaskImageNode.IsNull())
        return;

    mitk::Image::Pointer mitkMask = dynamic_cast<mitk::Image*>(m_MaskImageNode->GetData());
    for (unsigned int i=0; i<m_SelectedFB.size(); i++)
    {
        mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>(m_SelectedFB.at(i)->GetData());
        QString name(m_SelectedFB.at(i)->GetName().c_str());

        itkUCharImageType::Pointer mask = itkUCharImageType::New();
        mitk::CastToItkImage(mitkMask, mask);
        mitk::FiberBundleX::Pointer newFib = fib->ExtractFiberSubset(mask, false);
        if (newFib->GetNumFibers()<=0)
        {
            QMessageBox::information(NULL, "No output generated:", "The resulting fiber bundle contains no fibers.");
            continue;
        }

        DataNode::Pointer newNode = DataNode::New();
        newNode->SetData(newFib);
        name += "_ending-in-mask";
        newNode->SetName(name.toStdString());
        GetDefaultDataStorage()->Add(newNode);
        m_SelectedFB.at(i)->SetVisibility(false);
    }
}

void QmitkFiberExtractionView::ExtractNotPassingMask()
{
    if (m_MaskImageNode.IsNull())
        return;

    mitk::Image::Pointer mitkMask = dynamic_cast<mitk::Image*>(m_MaskImageNode->GetData());
    for (unsigned int i=0; i<m_SelectedFB.size(); i++)
    {
        mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>(m_SelectedFB.at(i)->GetData());
        QString name(m_SelectedFB.at(i)->GetName().c_str());

        itkUCharImageType::Pointer mask = itkUCharImageType::New();
        mitk::CastToItkImage<itkUCharImageType>(mitkMask, mask);
        mitk::FiberBundleX::Pointer newFib = fib->ExtractFiberSubset(mask, true, true);
        if (newFib->GetNumFibers()<=0)
        {
            QMessageBox::information(NULL, "No output generated:", "The resulting fiber bundle contains no fibers.");
            continue;
        }
        DataNode::Pointer newNode = DataNode::New();
        newNode->SetData(newFib);
        name += "_not-passing-mask";
        newNode->SetName(name.toStdString());
        GetDefaultDataStorage()->Add(newNode);
        m_SelectedFB.at(i)->SetVisibility(false);
    }
}

void QmitkFiberExtractionView::ExtractPassingMask()
{
    if (m_MaskImageNode.IsNull())
        return;

    mitk::Image::Pointer mitkMask = dynamic_cast<mitk::Image*>(m_MaskImageNode->GetData());
    for (unsigned int i=0; i<m_SelectedFB.size(); i++)
    {
        mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>(m_SelectedFB.at(i)->GetData());
        QString name(m_SelectedFB.at(i)->GetName().c_str());

        itkUCharImageType::Pointer mask = itkUCharImageType::New();
        mitk::CastToItkImage(mitkMask, mask);
        mitk::FiberBundleX::Pointer newFib = fib->ExtractFiberSubset(mask, true);
        if (newFib->GetNumFibers()<=0)
        {
            QMessageBox::information(NULL, "No output generated:", "The resulting fiber bundle contains no fibers.");
            continue;
        }
        DataNode::Pointer newNode = DataNode::New();
        newNode->SetData(newFib);
        name += "_passing-mask";
        newNode->SetName(name.toStdString());
        GetDefaultDataStorage()->Add(newNode);
        m_SelectedFB.at(i)->SetVisibility(false);
    }
}

void QmitkFiberExtractionView::GenerateRoiImage()
{
    if (m_SelectedPF.empty())
        return;

    mitk::BaseGeometry::Pointer geometry;
    if (!m_SelectedFB.empty())
    {
        mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>(m_SelectedFB.front()->GetData());
        geometry = fib->GetGeometry();
    }
    else if (m_SelectedImage)
        geometry = m_SelectedImage->GetGeometry();
    else
        return;

    itk::Vector<double,3> spacing = geometry->GetSpacing();
    spacing /= m_UpsamplingFactor;

    mitk::Point3D newOrigin = geometry->GetOrigin();
    mitk::Geometry3D::BoundsArrayType bounds = geometry->GetBounds();
    newOrigin[0] += bounds.GetElement(0);
    newOrigin[1] += bounds.GetElement(2);
    newOrigin[2] += bounds.GetElement(4);

    itk::Matrix<double, 3, 3> direction;
    itk::ImageRegion<3> imageRegion;
    for (int i=0; i<3; i++)
        for (int j=0; j<3; j++)
            direction[j][i] = geometry->GetMatrixColumn(i)[j]/spacing[j];
    imageRegion.SetSize(0, geometry->GetExtent(0)*m_UpsamplingFactor);
    imageRegion.SetSize(1, geometry->GetExtent(1)*m_UpsamplingFactor);
    imageRegion.SetSize(2, geometry->GetExtent(2)*m_UpsamplingFactor);

    m_PlanarFigureImage = itkUCharImageType::New();
    m_PlanarFigureImage->SetSpacing( spacing );   // Set the image spacing
    m_PlanarFigureImage->SetOrigin( newOrigin );     // Set the image origin
    m_PlanarFigureImage->SetDirection( direction );  // Set the image direction
    m_PlanarFigureImage->SetRegions( imageRegion );
    m_PlanarFigureImage->Allocate();
    m_PlanarFigureImage->FillBuffer( 0 );

    Image::Pointer tmpImage = Image::New();
    tmpImage->InitializeByItk(m_PlanarFigureImage.GetPointer());
    tmpImage->SetVolume(m_PlanarFigureImage->GetBufferPointer());

    std::string name = m_SelectedPF.at(0)->GetName();
    WritePfToImage(m_SelectedPF.at(0), tmpImage);
    for (unsigned int i=1; i<m_SelectedPF.size(); i++)
    {
        name += "+";
        name += m_SelectedPF.at(i)->GetName();
        WritePfToImage(m_SelectedPF.at(i), tmpImage);
    }

    DataNode::Pointer node = DataNode::New();
    tmpImage = Image::New();
    tmpImage->InitializeByItk(m_PlanarFigureImage.GetPointer());
    tmpImage->SetVolume(m_PlanarFigureImage->GetBufferPointer());
    node->SetData(tmpImage);
    node->SetName(name);
    this->GetDefaultDataStorage()->Add(node);
}

void QmitkFiberExtractionView::WritePfToImage(mitk::DataNode::Pointer node, mitk::Image* image)
{
    if (dynamic_cast<mitk::PlanarFigure*>(node->GetData()))
    {
        m_PlanarFigure = dynamic_cast<mitk::PlanarFigure*>(node->GetData());
        AccessFixedDimensionByItk_2(
                    image,
                    InternalReorientImagePlane, 3,
                    m_PlanarFigure->GetGeometry(), -1);

        AccessFixedDimensionByItk_2(
                    m_InternalImage,
                    InternalCalculateMaskFromPlanarFigure,
                    3, 2, node->GetName() );
    }
    else if (dynamic_cast<mitk::PlanarFigureComposite*>(node->GetData()))
    {
        mitk::PlanarFigureComposite* pfc = dynamic_cast<mitk::PlanarFigureComposite*>(node->GetData());
        for (int j=0; j<pfc->getNumberOfChildren(); j++)
        {
            WritePfToImage(pfc->getDataNodeAt(j), image);
        }
    }
}

template < typename TPixel, unsigned int VImageDimension >
void QmitkFiberExtractionView::InternalReorientImagePlane( const itk::Image< TPixel, VImageDimension > *image, mitk::BaseGeometry* planegeo3D, int additionalIndex )
{
    typedef itk::Image< TPixel, VImageDimension > ImageType;
    typedef itk::Image< float, VImageDimension > FloatImageType;

    typedef itk::ResampleImageFilter<ImageType, FloatImageType, double> ResamplerType;
    typename ResamplerType::Pointer resampler = ResamplerType::New();

    mitk::PlaneGeometry* planegeo = dynamic_cast<mitk::PlaneGeometry*>(planegeo3D);

    float upsamp = m_UpsamplingFactor;
    float gausssigma = 0.5;

    // Spacing
    typename ResamplerType::SpacingType spacing = planegeo->GetSpacing();
    spacing[0] = image->GetSpacing()[0] / upsamp;
    spacing[1] = image->GetSpacing()[1] / upsamp;
    spacing[2] = image->GetSpacing()[2];
    resampler->SetOutputSpacing( spacing );

    // Size
    typename ResamplerType::SizeType size;
    size[0] = planegeo->GetExtentInMM(0) / spacing[0];
    size[1] = planegeo->GetExtentInMM(1) / spacing[1];
    size[2] = 1;
    resampler->SetSize( size );

    // Origin
    typename mitk::Point3D orig = planegeo->GetOrigin();
    typename mitk::Point3D corrorig;
    planegeo3D->WorldToIndex(orig,corrorig);
    corrorig[0] += 0.5/upsamp;
    corrorig[1] += 0.5/upsamp;
    corrorig[2] += 0;
    planegeo3D->IndexToWorld(corrorig,corrorig);
    resampler->SetOutputOrigin(corrorig );

    // Direction
    typename ResamplerType::DirectionType direction;
    typename mitk::AffineTransform3D::MatrixType matrix = planegeo->GetIndexToWorldTransform()->GetMatrix();
    for(int c=0; c<matrix.ColumnDimensions; c++)
    {
        double sum = 0;
        for(int r=0; r<matrix.RowDimensions; r++)
            sum += matrix(r,c)*matrix(r,c);
        for(int r=0; r<matrix.RowDimensions; r++)
            direction(r,c) = matrix(r,c)/sqrt(sum);
    }
    resampler->SetOutputDirection( direction );

    // Gaussian interpolation
    if(gausssigma != 0)
    {
        double sigma[3];
        for( unsigned int d = 0; d < 3; d++ )
            sigma[d] = gausssigma * image->GetSpacing()[d];
        double alpha = 2.0;
        typedef itk::GaussianInterpolateImageFunction<ImageType, double> GaussianInterpolatorType;
        typename GaussianInterpolatorType::Pointer interpolator = GaussianInterpolatorType::New();
        interpolator->SetInputImage( image );
        interpolator->SetParameters( sigma, alpha );
        resampler->SetInterpolator( interpolator );
    }
    else
    {
        typedef typename itk::LinearInterpolateImageFunction<ImageType, double> InterpolatorType;
        typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
        interpolator->SetInputImage( image );
        resampler->SetInterpolator( interpolator );
    }

    resampler->SetInput( image );
    resampler->SetDefaultPixelValue(0);
    resampler->Update();

    if(additionalIndex < 0)
    {
        this->m_InternalImage = mitk::Image::New();
        this->m_InternalImage->InitializeByItk( resampler->GetOutput() );
        this->m_InternalImage->SetVolume( resampler->GetOutput()->GetBufferPointer() );
    }
}

template < typename TPixel, unsigned int VImageDimension >
void QmitkFiberExtractionView::InternalCalculateMaskFromPlanarFigure( itk::Image< TPixel, VImageDimension > *image, unsigned int axis, std::string )
{
    typedef itk::Image< TPixel, VImageDimension > ImageType;
    typedef itk::CastImageFilter< ImageType, itkUCharImageType > CastFilterType;

    // Generate mask image as new image with same header as input image and
    // initialize with "1".
    itkUCharImageType::Pointer newMaskImage = itkUCharImageType::New();
    newMaskImage->SetSpacing( image->GetSpacing() );   // Set the image spacing
    newMaskImage->SetOrigin( image->GetOrigin() );     // Set the image origin
    newMaskImage->SetDirection( image->GetDirection() );  // Set the image direction
    newMaskImage->SetRegions( image->GetLargestPossibleRegion() );
    newMaskImage->Allocate();
    newMaskImage->FillBuffer( 1 );

    // Generate VTK polygon from (closed) PlanarFigure polyline
    // (The polyline points are shifted by -0.5 in z-direction to make sure
    // that the extrusion filter, which afterwards elevates all points by +0.5
    // in z-direction, creates a 3D object which is cut by the the plane z=0)
    const PlaneGeometry *planarFigurePlaneGeometry = m_PlanarFigure->GetPlaneGeometry();
    const PlanarFigure::PolyLineType planarFigurePolyline = m_PlanarFigure->GetPolyLine( 0 );
    const BaseGeometry *imageGeometry3D = m_InternalImage->GetGeometry( 0 );

    vtkPolyData *polyline = vtkPolyData::New();
    polyline->Allocate( 1, 1 );

    // Determine x- and y-dimensions depending on principal axis
    int i0, i1;
    switch ( axis )
    {
    case 0:
        i0 = 1;
        i1 = 2;
        break;
    case 1:
        i0 = 0;
        i1 = 2;
        break;
    case 2:
    default:
        i0 = 0;
        i1 = 1;
        break;
    }

    // Create VTK polydata object of polyline contour
    vtkPoints *points = vtkPoints::New();
    PlanarFigure::PolyLineType::const_iterator it;
    unsigned int numberOfPoints = 0;

    for ( it = planarFigurePolyline.begin(); it != planarFigurePolyline.end(); ++it )
    {
        Point3D point3D;

        // Convert 2D point back to the local index coordinates of the selected image
        Point2D point2D = *it;
        planarFigurePlaneGeometry->WorldToIndex(point2D, point2D);
        point2D[0] -= 0.5/m_UpsamplingFactor;
        point2D[1] -= 0.5/m_UpsamplingFactor;
        planarFigurePlaneGeometry->IndexToWorld(point2D, point2D);
        planarFigurePlaneGeometry->Map( point2D, point3D );

        // Polygons (partially) outside of the image bounds can not be processed further due to a bug in vtkPolyDataToImageStencil
        if ( !imageGeometry3D->IsInside( point3D ) )
        {
            float bounds[2] = {0,0};
            bounds[0] =
                    this->m_InternalImage->GetLargestPossibleRegion().GetSize().GetElement(i0);
            bounds[1] =
                    this->m_InternalImage->GetLargestPossibleRegion().GetSize().GetElement(i1);

            imageGeometry3D->WorldToIndex( point3D, point3D );

            if (point3D[i0]<0)
                point3D[i0] = 0.0;
            else if (point3D[i0]>bounds[0])
                point3D[i0] = bounds[0]-0.001;

            if (point3D[i1]<0)
                point3D[i1] = 0.0;
            else if (point3D[i1]>bounds[1])
                point3D[i1] = bounds[1]-0.001;

            points->InsertNextPoint( point3D[i0], point3D[i1], -0.5 );
            numberOfPoints++;
        }
        else
        {
            imageGeometry3D->WorldToIndex( point3D, point3D );
            // Add point to polyline array
            points->InsertNextPoint( point3D[i0], point3D[i1], -0.5 );
            numberOfPoints++;
        }
    }
    polyline->SetPoints( points );
    points->Delete();

    vtkIdType *ptIds = new vtkIdType[numberOfPoints];
    for ( vtkIdType i = 0; i < numberOfPoints; ++i )
        ptIds[i] = i;
    polyline->InsertNextCell( VTK_POLY_LINE, numberOfPoints, ptIds );

    // Extrude the generated contour polygon
    vtkLinearExtrusionFilter *extrudeFilter = vtkLinearExtrusionFilter::New();
    extrudeFilter->SetInputData( polyline );
    extrudeFilter->SetScaleFactor( 1 );
    extrudeFilter->SetExtrusionTypeToNormalExtrusion();
    extrudeFilter->SetVector( 0.0, 0.0, 1.0 );

    // Make a stencil from the extruded polygon
    vtkPolyDataToImageStencil *polyDataToImageStencil = vtkPolyDataToImageStencil::New();
    polyDataToImageStencil->SetInputConnection( extrudeFilter->GetOutputPort() );

    // Export from ITK to VTK (to use a VTK filter)
    typedef itk::VTKImageImport< itkUCharImageType > ImageImportType;
    typedef itk::VTKImageExport< itkUCharImageType > ImageExportType;

    typename ImageExportType::Pointer itkExporter = ImageExportType::New();
    itkExporter->SetInput( newMaskImage );

    vtkImageImport *vtkImporter = vtkImageImport::New();
    this->ConnectPipelines( itkExporter, vtkImporter );
    vtkImporter->Update();

    // Apply the generated image stencil to the input image
    vtkImageStencil *imageStencilFilter = vtkImageStencil::New();
    imageStencilFilter->SetInputConnection( vtkImporter->GetOutputPort() );
    imageStencilFilter->SetStencilConnection(polyDataToImageStencil->GetOutputPort() );
    imageStencilFilter->ReverseStencilOff();
    imageStencilFilter->SetBackgroundValue( 0 );
    imageStencilFilter->Update();

    // Export from VTK back to ITK
    vtkImageExport *vtkExporter = vtkImageExport::New();
    vtkExporter->SetInputConnection( imageStencilFilter->GetOutputPort() );
    vtkExporter->Update();

    typename ImageImportType::Pointer itkImporter = ImageImportType::New();
    this->ConnectPipelines( vtkExporter, itkImporter );
    itkImporter->Update();

    // calculate cropping bounding box
    m_InternalImageMask3D = itkImporter->GetOutput();
    m_InternalImageMask3D->SetDirection(image->GetDirection());

    itk::ImageRegionConstIterator<itkUCharImageType>
            itmask(m_InternalImageMask3D, m_InternalImageMask3D->GetLargestPossibleRegion());
    itk::ImageRegionIterator<ImageType>
            itimage(image, image->GetLargestPossibleRegion());

    itmask.GoToBegin();
    itimage.GoToBegin();

    typename ImageType::SizeType lowersize = {{9999999999,9999999999,9999999999}};
    typename ImageType::SizeType uppersize = {{0,0,0}};
    while( !itmask.IsAtEnd() )
    {
        if(itmask.Get() == 0)
            itimage.Set(0);
        else
        {
            typename ImageType::IndexType index = itimage.GetIndex();
            typename ImageType::SizeType signedindex;
            signedindex[0] = index[0];
            signedindex[1] = index[1];
            signedindex[2] = index[2];

            lowersize[0] = signedindex[0] < lowersize[0] ? signedindex[0] : lowersize[0];
            lowersize[1] = signedindex[1] < lowersize[1] ? signedindex[1] : lowersize[1];
            lowersize[2] = signedindex[2] < lowersize[2] ? signedindex[2] : lowersize[2];

            uppersize[0] = signedindex[0] > uppersize[0] ? signedindex[0] : uppersize[0];
            uppersize[1] = signedindex[1] > uppersize[1] ? signedindex[1] : uppersize[1];
            uppersize[2] = signedindex[2] > uppersize[2] ? signedindex[2] : uppersize[2];
        }

        ++itmask;
        ++itimage;
    }

    typename ImageType::IndexType index;
    index[0] = lowersize[0];
    index[1] = lowersize[1];
    index[2] = lowersize[2];

    typename ImageType::SizeType size;
    size[0] = uppersize[0] - lowersize[0] + 1;
    size[1] = uppersize[1] - lowersize[1] + 1;
    size[2] = uppersize[2] - lowersize[2] + 1;

    itk::ImageRegion<3> cropRegion = itk::ImageRegion<3>(index, size);

    // crop internal mask
    typedef itk::RegionOfInterestImageFilter< itkUCharImageType, itkUCharImageType > ROIMaskFilterType;
    typename ROIMaskFilterType::Pointer roi2 = ROIMaskFilterType::New();
    roi2->SetRegionOfInterest(cropRegion);
    roi2->SetInput(m_InternalImageMask3D);
    roi2->Update();
    m_InternalImageMask3D = roi2->GetOutput();

    Image::Pointer tmpImage = Image::New();
    tmpImage->InitializeByItk(m_InternalImageMask3D.GetPointer());
    tmpImage->SetVolume(m_InternalImageMask3D->GetBufferPointer());

    Image::Pointer tmpImage2 = Image::New();
    tmpImage2->InitializeByItk(m_PlanarFigureImage.GetPointer());
    const BaseGeometry *pfImageGeometry3D = tmpImage2->GetGeometry( 0 );

    const BaseGeometry *intImageGeometry3D = tmpImage->GetGeometry( 0 );

    typedef itk::ImageRegionIteratorWithIndex<itkUCharImageType> IteratorType;
    IteratorType imageIterator (m_InternalImageMask3D, m_InternalImageMask3D->GetRequestedRegion());
    imageIterator.GoToBegin();
    while ( !imageIterator.IsAtEnd() )
    {
        unsigned char val = imageIterator.Value();
        if (val>0)
        {
            itk::Index<3> index = imageIterator.GetIndex();
            Point3D point;
            point[0] = index[0];
            point[1] = index[1];
            point[2] = index[2];

            intImageGeometry3D->IndexToWorld(point, point);
            pfImageGeometry3D->WorldToIndex(point, point);

            point[i0] += 0.5;
            point[i1] += 0.5;

            index[0] = point[0];
            index[1] = point[1];
            index[2] = point[2];

            if (pfImageGeometry3D->IsIndexInside(index))
                m_PlanarFigureImage->SetPixel(index, 1);
        }
        ++imageIterator;
    }

    // Clean up VTK objects
    polyline->Delete();
    extrudeFilter->Delete();
    polyDataToImageStencil->Delete();
    vtkImporter->Delete();
    imageStencilFilter->Delete();
    //vtkExporter->Delete(); // TODO: crashes when outcommented; memory leak??
    delete[] ptIds;
}

void QmitkFiberExtractionView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
    m_MultiWidget = &stdMultiWidget;
}

void QmitkFiberExtractionView::StdMultiWidgetNotAvailable()
{
    m_MultiWidget = NULL;
}

void QmitkFiberExtractionView::UpdateGui()
{
    m_Controls->m_FibLabel->setText("<font color='red'>mandatory</font>");
    m_Controls->m_PfLabel->setText("<font color='grey'>needed for extraction</font>");
    m_Controls->m_InputData->setTitle("Please Select Input Data");

    m_Controls->m_Extract3dButton_2->setEnabled(false);
    m_Controls->m_Extract3dButton->setEnabled(false);
    m_Controls->m_ExtractMask->setEnabled(false);
    m_Controls->m_RemoveOutsideMaskButton->setEnabled(false);
    m_Controls->m_RemoveInsideMaskButton->setEnabled(false);

    m_Controls->doExtractFibersButton->setEnabled(false);
    m_Controls->PFCompoANDButton->setEnabled(false);
    m_Controls->PFCompoORButton->setEnabled(false);
    m_Controls->PFCompoNOTButton->setEnabled(false);
    m_Controls->m_GenerateRoiImage->setEnabled(false);

    m_Controls->m_JoinBundles->setEnabled(false);
    m_Controls->m_SubstractBundles->setEnabled(false);
    m_Controls->doExtractFibersButton->setEnabled(false);
    m_Controls->m_PlanarFigureButtonsFrame->setEnabled(false);

    // are fiber bundles selected?
    if ( !m_SelectedFB.empty() )
    {
        m_Controls->m_FibLabel->setText(QString(m_SelectedFB.at(0)->GetName().c_str()));
        m_Controls->m_PlanarFigureButtonsFrame->setEnabled(true);

        // one bundle and one planar figure needed to extract fibers
        if (!m_SelectedPF.empty())
        {
            m_Controls->m_InputData->setTitle("Input Data");
            m_Controls->m_PfLabel->setText(QString(m_SelectedPF.at(0)->GetName().c_str()));
            m_Controls->doExtractFibersButton->setEnabled(true);
        }

        // more than two bundles needed to join/subtract
        if (m_SelectedFB.size() > 1)
        {
            m_Controls->m_FibLabel->setText("multiple bundles selected");

            m_Controls->m_JoinBundles->setEnabled(true);
            m_Controls->m_SubstractBundles->setEnabled(true);
        }

        if (m_MaskImageNode.IsNotNull())
        {
            m_Controls->m_Extract3dButton_2->setEnabled(true);
            m_Controls->m_Extract3dButton->setEnabled(true);
            m_Controls->m_ExtractMask->setEnabled(true);
            m_Controls->m_RemoveOutsideMaskButton->setEnabled(true);
            m_Controls->m_RemoveInsideMaskButton->setEnabled(true);
        }
    }

    // are planar figures selected?
    if ( !m_SelectedPF.empty() )
    {
        if ( !m_SelectedFB.empty() || m_SelectedImage.IsNotNull())
            m_Controls->m_GenerateRoiImage->setEnabled(true);

        if (m_SelectedPF.size() > 1)
        {
            m_Controls->PFCompoANDButton->setEnabled(true);
            m_Controls->PFCompoORButton->setEnabled(true);
        }
        else
            m_Controls->PFCompoNOTButton->setEnabled(true);
    }
}

void QmitkFiberExtractionView::NodeRemoved(const mitk::DataNode* node)
{
    std::vector<mitk::DataNode*> nodes;
    OnSelectionChanged(nodes);
}

void QmitkFiberExtractionView::NodeAdded(const mitk::DataNode* node)
{
    std::vector<mitk::DataNode*> nodes;
    OnSelectionChanged(nodes);
}

void QmitkFiberExtractionView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
    //reset existing Vectors containing FiberBundles and PlanarFigures from a previous selection
    m_SelectedFB.clear();
    m_SelectedPF.clear();
    m_SelectedSurfaces.clear();
    m_SelectedImage = NULL;
    m_MaskImageNode = NULL;

    for( std::vector<mitk::DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it )
    {
        mitk::DataNode::Pointer node = *it;
        if ( dynamic_cast<mitk::FiberBundleX*>(node->GetData()) )
            m_SelectedFB.push_back(node);
        else if (dynamic_cast<mitk::PlanarPolygon*>(node->GetData()) || dynamic_cast<mitk::PlanarFigureComposite*>(node->GetData()) || dynamic_cast<mitk::PlanarCircle*>(node->GetData()))
            m_SelectedPF.push_back(node);
        else if (dynamic_cast<mitk::Image*>(node->GetData()))
        {
            m_SelectedImage = dynamic_cast<mitk::Image*>(node->GetData());
            bool isBinary = false;
            node->GetPropertyValue<bool>("binary", isBinary);
            if (isBinary)
                m_MaskImageNode = node;
        }
        else if (dynamic_cast<mitk::Surface*>(node->GetData()))
            m_SelectedSurfaces.push_back(dynamic_cast<mitk::Surface*>(node->GetData()));
    }

    if (m_SelectedFB.empty())
    {
        int maxLayer = 0;
        itk::VectorContainer<unsigned int, mitk::DataNode::Pointer>::ConstPointer nodes = this->GetDefaultDataStorage()->GetAll();
        for (unsigned int i=0; i<nodes->Size(); i++)
            if (dynamic_cast<mitk::FiberBundleX*>(nodes->at(i)->GetData()))
            {
                mitk::DataStorage::SetOfObjects::ConstPointer sources = GetDataStorage()->GetSources(nodes->at(i));
                if (sources->Size()>0)
                    continue;

                int layer = 0;
                nodes->at(i)->GetPropertyValue("layer", layer);
                if (layer>=maxLayer)
                {
                    maxLayer = layer;
                    m_SelectedFB.clear();
                    m_SelectedFB.push_back(nodes->at(i));
                }
            }
    }

    if (m_SelectedPF.empty())
    {
        int maxLayer = 0;
        itk::VectorContainer<unsigned int, mitk::DataNode::Pointer>::ConstPointer nodes = this->GetDefaultDataStorage()->GetAll();
        for (unsigned int i=0; i<nodes->Size(); i++)
            if (dynamic_cast<mitk::PlanarPolygon*>(nodes->at(i)->GetData()) || dynamic_cast<mitk::PlanarFigureComposite*>(nodes->at(i)->GetData()) || dynamic_cast<mitk::PlanarCircle*>(nodes->at(i)->GetData()))
            {
                mitk::DataStorage::SetOfObjects::ConstPointer sources = GetDataStorage()->GetSources(nodes->at(i));
                if (sources->Size()>0)
                    continue;

                int layer = 0;
                nodes->at(i)->GetPropertyValue("layer", layer);
                if (layer>=maxLayer)
                {
                    maxLayer = layer;
                    m_SelectedPF.clear();
                    m_SelectedPF.push_back(nodes->at(i));
                }
            }
    }

    UpdateGui();
}

void QmitkFiberExtractionView::OnDrawPolygon()
{
    mitk::PlanarPolygon::Pointer figure = mitk::PlanarPolygon::New();
    figure->ClosedOn();
    this->AddFigureToDataStorage(figure, QString("Polygon%1").arg(++m_PolygonCounter));
}

void QmitkFiberExtractionView::OnDrawCircle()
{
    mitk::PlanarCircle::Pointer figure = mitk::PlanarCircle::New();
    this->AddFigureToDataStorage(figure, QString("Circle%1").arg(++m_CircleCounter));
}

void QmitkFiberExtractionView::Activated()
{

}

void QmitkFiberExtractionView::AddFigureToDataStorage(mitk::PlanarFigure* figure, const QString& name, const char *, mitk::BaseProperty* )
{
    // initialize figure's geometry with empty geometry
    mitk::PlaneGeometry::Pointer emptygeometry = mitk::PlaneGeometry::New();
    figure->SetPlaneGeometry( emptygeometry );

    //set desired data to DataNode where Planarfigure is stored
    mitk::DataNode::Pointer newNode = mitk::DataNode::New();
    newNode->SetName(name.toStdString());
    newNode->SetData(figure);
    newNode->SetBoolProperty("planarfigure.3drendering", true);

    mitk::PlanarFigureInteractor::Pointer figureInteractor = dynamic_cast<mitk::PlanarFigureInteractor*>(newNode->GetDataInteractor().GetPointer());
    if(figureInteractor.IsNull())
    {
        figureInteractor = mitk::PlanarFigureInteractor::New();
        us::Module* planarFigureModule = us::ModuleRegistry::GetModule( "MitkPlanarFigure" );
        figureInteractor->LoadStateMachine("PlanarFigureInteraction.xml", planarFigureModule );
        figureInteractor->SetEventConfig( "PlanarFigureConfig.xml", planarFigureModule );
        figureInteractor->SetDataNode(newNode);
    }

    // figure drawn on the topmost layer / image
    GetDataStorage()->Add(newNode );

    for(unsigned int i = 0; i < m_SelectedPF.size(); i++)
        m_SelectedPF[i]->SetSelected(false);

    newNode->SetSelected(true);
    m_SelectedPF.clear();
    m_SelectedPF.push_back(newNode);
    UpdateGui();
}

void QmitkFiberExtractionView::DoFiberExtraction()
{
    if ( m_SelectedFB.empty() || m_SelectedPF.empty() ){
        QMessageBox::information( NULL, "Warning", "No fibe bundle selected!");
        return;
    }

    std::vector<mitk::DataNode::Pointer> fiberBundles = m_SelectedFB;
    mitk::DataNode::Pointer planarFigure = m_SelectedPF.at(0);
    for (unsigned int i=0; i<fiberBundles.size(); i++)
    {
        mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>(fiberBundles.at(i)->GetData());
        mitk::BaseData::Pointer roi = planarFigure->GetData();

        mitk::FiberBundleX::Pointer extFB = fib->ExtractFiberSubset(roi);
        if (extFB->GetNumFibers()<=0)
        {
            QMessageBox::information(NULL, "No output generated:", "The resulting fiber bundle contains no fibers.");
            continue;
        }

        mitk::DataNode::Pointer node;
        node = mitk::DataNode::New();
        node->SetData(extFB);
        QString name(fiberBundles.at(i)->GetName().c_str());
        name += "_";
        name += planarFigure->GetName().c_str();
        node->SetName(name.toStdString());
        fiberBundles.at(i)->SetVisibility(false);
        GetDataStorage()->Add(node);
    }
}

void QmitkFiberExtractionView::GenerateAndComposite()
{
    mitk::PlanarFigureComposite::Pointer PFCAnd = mitk::PlanarFigureComposite::New();
    PFCAnd->setOperationType(mitk::PFCOMPOSITION_AND_OPERATION);
    for( std::vector<mitk::DataNode::Pointer>::iterator it = m_SelectedPF.begin(); it != m_SelectedPF.end(); ++it )
    {
        mitk::DataNode::Pointer nodePF = *it;
        PFCAnd->addPlanarFigure( nodePF->GetData() );
        PFCAnd->addDataNode( nodePF );
        PFCAnd->setDisplayName("AND");
    }
    AddCompositeToDatastorage(PFCAnd);
}

void QmitkFiberExtractionView::GenerateOrComposite()
{
    mitk::PlanarFigureComposite::Pointer PFCOr = mitk::PlanarFigureComposite::New();
    PFCOr->setOperationType(mitk::PFCOMPOSITION_OR_OPERATION);
    for( std::vector<mitk::DataNode::Pointer>::iterator it = m_SelectedPF.begin(); it != m_SelectedPF.end(); ++it )
    {
        mitk::DataNode::Pointer nodePF = *it;
        PFCOr->addPlanarFigure( nodePF->GetData() );
        PFCOr->addDataNode( nodePF );
        PFCOr->setDisplayName("OR");
    }
    AddCompositeToDatastorage(PFCOr);
}

void QmitkFiberExtractionView::GenerateNotComposite()
{
    mitk::PlanarFigureComposite::Pointer PFCNot = mitk::PlanarFigureComposite::New();
    PFCNot->setOperationType(mitk::PFCOMPOSITION_NOT_OPERATION);
    for( std::vector<mitk::DataNode::Pointer>::iterator it = m_SelectedPF.begin(); it != m_SelectedPF.end(); ++it )
    {
        mitk::DataNode::Pointer nodePF = *it;
        PFCNot->addPlanarFigure( nodePF->GetData() );
        PFCNot->addDataNode( nodePF );
        PFCNot->setDisplayName("NOT");
    }
    AddCompositeToDatastorage(PFCNot);
}

/* CLEANUP NEEDED */
void QmitkFiberExtractionView::AddCompositeToDatastorage(mitk::PlanarFigureComposite::Pointer pfc, mitk::DataNode::Pointer parentNode )
{
    mitk::DataNode::Pointer newPFCNode;
    newPFCNode = mitk::DataNode::New();
    newPFCNode->SetName( pfc->getDisplayName() );
    newPFCNode->SetData(pfc);

    switch (pfc->getOperationType()) {
    case 0:
    {
        if (parentNode.IsNotNull())
            GetDataStorage()->Add(newPFCNode, parentNode);
        else
            GetDataStorage()->Add(newPFCNode);

        //iterate through its childs
        for(int i=0; i<pfc->getNumberOfChildren(); ++i)
        {
            mitk::BaseData::Pointer tmpPFchild = pfc->getChildAt(i);
            mitk::DataNode::Pointer savedPFchildNode = pfc->getDataNodeAt(i);

            mitk::PlanarFigureComposite::Pointer pfcompcast= dynamic_cast<mitk::PlanarFigureComposite*>(tmpPFchild.GetPointer());
            if ( pfcompcast.IsNotNull() )
            {
                // child is of type planar Figure composite
                // make new node of the child, cuz later the child has to be removed of its old position in datamanager
                // feed new dataNode with information of the savedDataNode, which is gonna be removed soon
                mitk::DataNode::Pointer newChildPFCNode;
                newChildPFCNode = mitk::DataNode::New();
                newChildPFCNode->SetData(tmpPFchild);
                newChildPFCNode->SetName( savedPFchildNode->GetName() );
                pfcompcast->setDisplayName(  savedPFchildNode->GetName()  ); //name might be changed in DataManager by user

                //update inside vector the dataNodePointer
                pfc->replaceDataNodeAt(i, newChildPFCNode);

                AddCompositeToDatastorage(pfcompcast, newPFCNode); //the current PFCNode becomes the childs parent

                // remove savedNode here, cuz otherwise its children will change their position in the dataNodeManager
                // without having its parent anymore
                GetDataStorage()->Remove(savedPFchildNode);
            }
            else
            {
                // child is not of type PlanarFigureComposite, so its one of the planarFigures
                // create new dataNode containing the data of the old dataNode, but position in dataManager will be
                // modified cuz we re setting a (new) parent.
                mitk::DataNode::Pointer newPFchildNode = mitk::DataNode::New();
                newPFchildNode->SetName(savedPFchildNode->GetName() );
                newPFchildNode->SetData(tmpPFchild);
                newPFchildNode->SetVisibility(true);
                newPFchildNode->SetBoolProperty("planarfigure.3drendering", true);

                // replace the dataNode in PFComp DataNodeVector
                pfc->replaceDataNodeAt(i, newPFchildNode);

                // remove old child position in dataStorage
                GetDataStorage()->Remove(savedPFchildNode);

                //add new child to datamanager with its new position as child of newPFCNode parent
                GetDataStorage()->Add(newPFchildNode, newPFCNode);
            }
        }
        break;
    }
    case 1:
    {
        if (!parentNode.IsNull())
            GetDataStorage()->Add(newPFCNode, parentNode);
        else
            GetDataStorage()->Add(newPFCNode);

        for(int i=0; i<pfc->getNumberOfChildren(); ++i)
        {
            mitk::BaseData::Pointer tmpPFchild = pfc->getChildAt(i);
            mitk::DataNode::Pointer savedPFchildNode = pfc->getDataNodeAt(i);
            mitk::PlanarFigureComposite::Pointer pfcompcast= dynamic_cast<mitk::PlanarFigureComposite*>(tmpPFchild.GetPointer());
            if ( !pfcompcast.IsNull() )
            {
                // child is of type planar Figure composite
                // make new node of the child, cuz later the child has to be removed of its old position in datamanager
                // feed new dataNode with information of the savedDataNode, which is gonna be removed soon
                mitk::DataNode::Pointer newChildPFCNode;
                newChildPFCNode = mitk::DataNode::New();
                newChildPFCNode->SetData(tmpPFchild);
                newChildPFCNode->SetName( savedPFchildNode->GetName() );
                pfcompcast->setDisplayName(  savedPFchildNode->GetName()  ); //name might be changed in DataManager by user

                //update inside vector the dataNodePointer
                pfc->replaceDataNodeAt(i, newChildPFCNode);

                AddCompositeToDatastorage(pfcompcast, newPFCNode); //the current PFCNode becomes the childs parent

                // remove old child position in dataStorage
                GetDataStorage()->Remove(savedPFchildNode);
            }
            else
            {
                // child is not of type PlanarFigureComposite, so its one of the planarFigures
                // create new dataNode containing the data of the old dataNode, but position in dataManager will be
                // modified cuz we re setting a (new) parent.
                mitk::DataNode::Pointer newPFchildNode = mitk::DataNode::New();
                newPFchildNode->SetName(savedPFchildNode->GetName() );
                newPFchildNode->SetData(tmpPFchild);
                newPFchildNode->SetVisibility(true);
                newPFchildNode->SetBoolProperty("planarfigure.3drendering", true);

                // replace the dataNode in PFComp DataNodeVector
                pfc->replaceDataNodeAt(i, newPFchildNode);

                // remove old child position in dataStorage
                GetDataStorage()->Remove(savedPFchildNode);

                //add new child to datamanager with its new position as child of newPFCNode parent
                GetDataStorage()->Add(newPFchildNode, newPFCNode);
            }
        }
        break;
    }
    case 2:
    {
        if (!parentNode.IsNull())
            GetDataStorage()->Add(newPFCNode, parentNode);
        else
            GetDataStorage()->Add(newPFCNode);

        //iterate through its childs
        for(int i=0; i<pfc->getNumberOfChildren(); ++i)
        {
            mitk::BaseData::Pointer tmpPFchild = pfc->getChildAt(i);
            mitk::DataNode::Pointer savedPFchildNode = pfc->getDataNodeAt(i);

            mitk::PlanarFigureComposite::Pointer pfcompcast= dynamic_cast<mitk::PlanarFigureComposite*>(tmpPFchild.GetPointer());
            if ( !pfcompcast.IsNull() )
            { // child is of type planar Figure composite
                // makeRemoveBundle new node of the child, cuz later the child has to be removed of its old position in datamanager
                // feed new dataNode with information of the savedDataNode, which is gonna be removed soon
                mitk::DataNode::Pointer newChildPFCNode;
                newChildPFCNode = mitk::DataNode::New();
                newChildPFCNode->SetData(tmpPFchild);
                newChildPFCNode->SetName( savedPFchildNode->GetName() );
                pfcompcast->setDisplayName( savedPFchildNode->GetName() ); //name might be changed in DataManager by user

                //update inside vector the dataNodePointer
                pfc->replaceDataNodeAt(i, newChildPFCNode);

                AddCompositeToDatastorage(pfcompcast, newPFCNode); //the current PFCNode becomes the childs parent

                // remove old child position in dataStorage
                GetDataStorage()->Remove(savedPFchildNode);
            }
            else
            {
                // child is not of type PlanarFigureComposite, so its one of the planarFigures
                // create new dataNode containing the data of the old dataNode, but position in dataManager will be
                // modified cuz we re setting a (new) parent.
                mitk::DataNode::Pointer newPFchildNode = mitk::DataNode::New();
                newPFchildNode->SetName(savedPFchildNode->GetName() );
                newPFchildNode->SetData(tmpPFchild);
                newPFchildNode->SetVisibility(true);
                newPFchildNode->SetBoolProperty("planarfigure.3drendering", true);

                // replace the dataNode in PFComp DataNodeVector
                pfc->replaceDataNodeAt(i, newPFchildNode);

                // remove old child position in dataStorage
                GetDataStorage()->Remove(savedPFchildNode);

                //add new child to datamanager with its new position as child of newPFCNode parent
                GetDataStorage()->Add(newPFchildNode, newPFCNode);
            }
        }
        break;
    }
    default:
        MITK_DEBUG << "we have an UNDEFINED composition... ERROR" ;
        break;
    }

    for(unsigned int i = 0; i < m_SelectedPF.size(); i++)
        m_SelectedPF[i]->SetSelected(false);

    newPFCNode->SetSelected(true);
    m_SelectedPF.clear();
    m_SelectedPF.push_back(newPFCNode);
    UpdateGui();
}

void QmitkFiberExtractionView::JoinBundles()
{
    if ( m_SelectedFB.size()<2 ){
        QMessageBox::information( NULL, "Warning", "Select at least two fiber bundles!");
        MITK_WARN("QmitkFiberExtractionView") << "Select at least two fiber bundles!";
        return;
    }

    mitk::FiberBundleX::Pointer newBundle = dynamic_cast<mitk::FiberBundleX*>(m_SelectedFB.at(0)->GetData());
    m_SelectedFB.at(0)->SetVisibility(false);
    QString name("");
    name += QString(m_SelectedFB.at(0)->GetName().c_str());
    for (unsigned int i=1; i<m_SelectedFB.size(); i++)
    {
        newBundle = newBundle->AddBundle(dynamic_cast<mitk::FiberBundleX*>(m_SelectedFB.at(i)->GetData()));
        name += "+"+QString(m_SelectedFB.at(i)->GetName().c_str());
        m_SelectedFB.at(i)->SetVisibility(false);
    }

    mitk::DataNode::Pointer fbNode = mitk::DataNode::New();
    fbNode->SetData(newBundle);
    fbNode->SetName(name.toStdString());
    fbNode->SetVisibility(true);
    GetDataStorage()->Add(fbNode);
}

void QmitkFiberExtractionView::SubstractBundles()
{
    if ( m_SelectedFB.size()<2 ){
        QMessageBox::information( NULL, "Warning", "Select at least two fiber bundles!");
        MITK_WARN("QmitkFiberExtractionView") << "Select at least two fiber bundles!";
        return;
    }

    mitk::FiberBundleX::Pointer newBundle = dynamic_cast<mitk::FiberBundleX*>(m_SelectedFB.at(0)->GetData());
    m_SelectedFB.at(0)->SetVisibility(false);
    QString name("");
    name += QString(m_SelectedFB.at(0)->GetName().c_str());
    for (unsigned int i=1; i<m_SelectedFB.size(); i++)
    {
        newBundle = newBundle->SubtractBundle(dynamic_cast<mitk::FiberBundleX*>(m_SelectedFB.at(i)->GetData()));
        if (newBundle.IsNull())
            break;
        name += "-"+QString(m_SelectedFB.at(i)->GetName().c_str());
        m_SelectedFB.at(i)->SetVisibility(false);
    }
    if (newBundle.IsNull())
    {
        QMessageBox::information(NULL, "No output generated:", "The resulting fiber bundle contains no fibers. Did you select the fiber bundles in the correct order? X-Y is not equal to Y-X!");
        return;
    }

    mitk::DataNode::Pointer fbNode = mitk::DataNode::New();
    fbNode->SetData(newBundle);
    fbNode->SetName(name.toStdString());
    fbNode->SetVisibility(true);
    GetDataStorage()->Add(fbNode);
}
