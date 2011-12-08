/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
 Version:   $Revision: 21975 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


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
#include <mitkSurface.h>
#include <mitkDiffusionImage.h>

// ITK
#include <itkResampleImageFilter.h>
#include <itkGaussianInterpolateImageFunction.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkTractsToFiberEndingsImageFilter.h>
#include <itkTractDensityImageFilter.h>
#include <itkImageRegion.h>
#include <itkTractsToRgbaImageFilter.h>


const std::string QmitkFiberProcessingView::VIEW_ID = "org.mitk.views.fiberprocessing";
const std::string id_DataManager = "org.mitk.views.datamanager";
using namespace mitk;

QmitkFiberProcessingView::QmitkFiberProcessingView()
: QmitkFunctionality()
, m_Controls( 0 )
, m_MultiWidget( NULL )
, m_EllipseCounter(0)
, m_PolygonCounter(0)
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
    m_Controls->doExtractFibersButton->setDisabled(true);
    m_Controls->PFCompoANDButton->setDisabled(true);
    m_Controls->PFCompoORButton->setDisabled(true);
    m_Controls->PFCompoNOTButton->setDisabled(true);
    m_Controls->m_CircleButton->setEnabled(false);
    m_Controls->m_PolygonButton->setEnabled(false);
    m_Controls->m_RectangleButton->setEnabled(false);
    m_Controls->m_RectangleButton->setVisible(false);

    connect( m_Controls->doExtractFibersButton, SIGNAL(clicked()), this, SLOT(DoFiberExtraction()) );
    connect( m_Controls->m_CircleButton, SIGNAL( clicked() ), this, SLOT( ActionDrawEllipseTriggered() ) );
    connect( m_Controls->m_PolygonButton, SIGNAL( clicked() ), this, SLOT( ActionDrawPolygonTriggered() ) );
    connect(m_Controls->PFCompoANDButton, SIGNAL(clicked()), this, SLOT(GenerateAndComposite()) );
    connect(m_Controls->PFCompoORButton, SIGNAL(clicked()), this, SLOT(GenerateOrComposite()) );
    connect(m_Controls->PFCompoNOTButton, SIGNAL(clicked()), this, SLOT(GenerateNotComposite()) );

    connect(m_Controls->m_JoinBundles, SIGNAL(clicked()), this, SLOT(JoinBundles()) );
    connect(m_Controls->m_SubstractBundles, SIGNAL(clicked()), this, SLOT(SubstractBundles()) );
    connect(m_Controls->m_GenerateRoiImage, SIGNAL(clicked()), this, SLOT(GenerateRoiImage()) );

    connect(m_Controls->m_Extract3dButton, SIGNAL(clicked()), this, SLOT(Extract3d()));
    connect( m_Controls->m_ProcessFiberBundleButton, SIGNAL(clicked()), this, SLOT(ProcessSelectedBundles()) );
  }
}

void QmitkFiberProcessingView::Extract3d()
{
  std::vector<mitk::DataNode*> nodes = this->GetDataManagerSelection();
  if (nodes.empty())
    return;

  mitk::FiberBundleX::Pointer fib = mitk::FiberBundleX::New();
  mitk::Surface::Pointer roi = mitk::Surface::New();
  bool fibB = false;
  bool roiB = false;
  for (int i=0; i<nodes.size(); i++)
  {
    if (dynamic_cast<mitk::FiberBundleX*>(nodes.at(i)->GetData()))
    {
      fib = dynamic_cast<mitk::FiberBundleX*>(nodes.at(i)->GetData());
      fibB = true;
    }
    else if (dynamic_cast<mitk::Surface*>(nodes.at(i)->GetData()))
    {
      roi = dynamic_cast<mitk::Surface*>(nodes.at(i)->GetData());
      roiB = true;
    }
  }
  if (!fibB)
    return;
  if (!roiB)
    return;

  vtkSmartPointer<vtkPolyData> polyRoi = roi->GetVtkPolyData();
  vtkSmartPointer<vtkPolyData> polyFib = fib->GetFiberPolyData();

  vtkSmartPointer<vtkSelectEnclosedPoints> selectEnclosedPoints = vtkSmartPointer<vtkSelectEnclosedPoints>::New();
  selectEnclosedPoints->SetInput(polyFib);
  selectEnclosedPoints->SetSurface(polyRoi);
  selectEnclosedPoints->Update();

  vtkSmartPointer<vtkPolyData> newPoly = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkCellArray> newCellArray = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkPoints>    newPoints = vtkSmartPointer<vtkPoints>::New();

  vtkSmartPointer<vtkPolyData> newPolyComplement = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkCellArray> newCellArrayComplement = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkPoints>    newPointsComplement = vtkSmartPointer<vtkPoints>::New();

  vtkSmartPointer<vtkCellArray> vLines = polyFib->GetLines();

  vLines->InitTraversal();
  int numberOfLines = vLines->GetNumberOfCells();
  // each line
  for (int j=0; j<numberOfLines; j++)
  {
    vtkIdType   numPoints(0);
    vtkIdType*  points(NULL);
    vLines->GetNextCell ( numPoints, points );
    bool isPassing = false;

    // each point of this line
    for (int k=0; k<numPoints; k++)
    {
      // is point inside polydata ?
      if (selectEnclosedPoints->IsInside(points[k]))
      {
        isPassing = true;
        // fill new polydata
        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
        for (int k=0; k<numPoints; k++)
        {
          double* point = polyFib->GetPoint(points[k]);
          vtkIdType pointId = newPoints->InsertNextPoint(point);
          container->GetPointIds()->InsertNextId(pointId);
        }
        newCellArray->InsertNextCell(container);
        break;
      }
    }
    if (!isPassing)
    {
      vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
      for (int k=0; k<numPoints; k++)
      {
        double* point = polyFib->GetPoint(points[k]);
        vtkIdType pointId = newPointsComplement->InsertNextPoint(point);
        container->GetPointIds()->InsertNextId(pointId);
      }
      newCellArrayComplement->InsertNextCell(container);
    }
  }

  newPoly->SetPoints(newPoints);
  newPoly->SetLines(newCellArray);
  mitk::FiberBundleX::Pointer fb = mitk::FiberBundleX::New(newPoly);
  DataNode::Pointer newNode = DataNode::New();
  newNode->SetData(fb);
  newNode->SetName("passing surface");
  GetDefaultDataStorage()->Add(newNode);

  newPolyComplement->SetPoints(newPointsComplement);
  newPolyComplement->SetLines(newCellArrayComplement);
  mitk::FiberBundleX::Pointer fbComplement = mitk::FiberBundleX::New(newPolyComplement);
  DataNode::Pointer newNodeComplement = DataNode::New();
  newNodeComplement->SetData(fbComplement);
  newNodeComplement->SetName("not passing surface");
  GetDefaultDataStorage()->Add(newNodeComplement);
}
void QmitkFiberProcessingView::GenerateRoiImage(){

  if (m_SelectedImage.IsNull() || m_SelectedPF.empty())
    return;

  mitk::Image* image = const_cast<mitk::Image*>(m_SelectedImage.GetPointer());

  UCharImageType::Pointer temp = UCharImageType::New();
  mitk::CastToItkImage<UCharImageType>(m_SelectedImage, temp);

  m_PlanarFigureImage = UCharImageType::New();
  m_PlanarFigureImage->SetSpacing( temp->GetSpacing() );   // Set the image spacing
  m_PlanarFigureImage->SetOrigin( temp->GetOrigin() );     // Set the image origin
  m_PlanarFigureImage->SetDirection( temp->GetDirection() );  // Set the image direction
  m_PlanarFigureImage->SetRegions( temp->GetLargestPossibleRegion() );
  m_PlanarFigureImage->Allocate();
  m_PlanarFigureImage->FillBuffer( 0 );

  for (int i=0; i<m_SelectedPF.size(); i++)
    CompositeExtraction(m_SelectedPF.at(i), image);

  DataNode::Pointer node = DataNode::New();
  Image::Pointer tmpImage = Image::New();
  tmpImage->InitializeByItk(m_PlanarFigureImage.GetPointer());
  tmpImage->SetVolume(m_PlanarFigureImage->GetBufferPointer());
  node->SetData(tmpImage);
  node->SetName("ROI Image");
  this->GetDefaultDataStorage()->Add(node);
}

void QmitkFiberProcessingView::CompositeExtraction(mitk::DataNode::Pointer node, mitk::Image* image)
{
  if (dynamic_cast<mitk::PlanarFigure*>(node.GetPointer()->GetData()) && !dynamic_cast<mitk::PlanarFigureComposite*>(node.GetPointer()->GetData()))
  {
    m_PlanarFigure = dynamic_cast<mitk::PlanarFigure*>(node.GetPointer()->GetData());
    AccessFixedDimensionByItk_2(
        image,
        InternalReorientImagePlane, 3,
        m_PlanarFigure->GetGeometry(), -1);

//    itk::Image< unsigned char, 3 >::Pointer outimage = itk::Image< unsigned char, 3 >::New();

//    outimage->SetSpacing( m_PlanarFigure->GetGeometry()->GetSpacing()/m_UpsamplingFactor );   // Set the image spacing

//    mitk::Point3D origin = m_PlanarFigure->GetGeometry()->GetOrigin();
//    mitk::Point3D indexOrigin;
//    m_PlanarFigure->GetGeometry()->WorldToIndex(origin, indexOrigin);
//    indexOrigin[0] = indexOrigin[0] - .5 * (1.0-1.0/m_UpsamplingFactor);
//    indexOrigin[1] = indexOrigin[1] - .5 * (1.0-1.0/m_UpsamplingFactor);
//    indexOrigin[2] = indexOrigin[2] - .5 * (1.0-1.0/m_UpsamplingFactor);
//    mitk::Point3D newOrigin;
//    m_PlanarFigure->GetGeometry()->IndexToWorld(indexOrigin, newOrigin);

//    outimage->SetOrigin( newOrigin );     // Set the image origin
//    itk::Matrix<double, 3, 3> matrix;
//    for (int i=0; i<3; i++)
//      for (int j=0; j<3; j++)
//        matrix[j][i] = m_PlanarFigure->GetGeometry()->GetMatrixColumn(i)[j]/m_PlanarFigure->GetGeometry()->GetSpacing().GetElement(i);
//    outimage->SetDirection( matrix );  // Set the image direction

//    itk::ImageRegion<3> upsampledRegion;
//    upsampledRegion.SetSize(0, m_PlanarFigure->GetGeometry()->GetParametricExtentInMM(0)/m_PlanarFigure->GetGeometry()->GetSpacing()[0]);
//    upsampledRegion.SetSize(1, m_PlanarFigure->GetGeometry()->GetParametricExtentInMM(1)/m_PlanarFigure->GetGeometry()->GetSpacing()[1]);
//    upsampledRegion.SetSize(2, 1);

//    typename itk::Image< unsigned char, 3 >::RegionType::SizeType upsampledSize = upsampledRegion.GetSize();
//    for (unsigned int n = 0; n < 2; n++)
//    {
//      upsampledSize[n] = upsampledSize[n] * m_UpsamplingFactor;
//    }
//    upsampledRegion.SetSize( upsampledSize );
//    outimage->SetRegions( upsampledRegion );

//    outimage->Allocate();

//    this->m_InternalImage = mitk::Image::New();
//    this->m_InternalImage->InitializeByItk( outimage.GetPointer() );
//    this->m_InternalImage->SetVolume( outimage->GetBufferPointer() );

    AccessFixedDimensionByItk_2(
        m_InternalImage,
        InternalCalculateMaskFromPlanarFigure,
        3, 2, node->GetName() );
  }
}

template < typename TPixel, unsigned int VImageDimension >
    void QmitkFiberProcessingView::InternalReorientImagePlane(
        const itk::Image< TPixel, VImageDimension > *image, mitk::Geometry3D* planegeo3D, int additionalIndex )
{

  MITK_INFO << "InternalReorientImagePlane() start";

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
  size[0] = planegeo->GetParametricExtentInMM(0) / spacing[0];
  size[1] = planegeo->GetParametricExtentInMM(1) / spacing[1];
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
    {
      sum += matrix(r,c)*matrix(r,c);
    }
    for(int r=0; r<matrix.RowDimensions; r++)
    {
      direction(r,c) = matrix(r,c)/sqrt(sum);
    }
  }
  resampler->SetOutputDirection( direction );

  // Gaussian interpolation
  if(gausssigma != 0)
  {
    double sigma[3];
    for( unsigned int d = 0; d < 3; d++ )
    {
      sigma[d] = gausssigma * image->GetSpacing()[d];
    }
    double alpha = 2.0;

    typedef itk::GaussianInterpolateImageFunction<ImageType, double>
        GaussianInterpolatorType;

    typename GaussianInterpolatorType::Pointer interpolator
        = GaussianInterpolatorType::New();

    interpolator->SetInputImage( image );
    interpolator->SetParameters( sigma, alpha );

    resampler->SetInterpolator( interpolator );
  }
  else
  {
    //      typedef typename itk::BSplineInterpolateImageFunction<ImageType, double>
    //          InterpolatorType;
    typedef typename itk::LinearInterpolateImageFunction<ImageType, double> InterpolatorType;

    typename InterpolatorType::Pointer interpolator
        = InterpolatorType::New();

    interpolator->SetInputImage( image );

    resampler->SetInterpolator( interpolator );

  }

  // Other resampling options
  resampler->SetInput( image );
  resampler->SetDefaultPixelValue(0);

  MITK_INFO << "Resampling requested image plane ... ";
  resampler->Update();
  MITK_INFO << " ... done";

  if(additionalIndex < 0)
  {
    this->m_InternalImage = mitk::Image::New();
    this->m_InternalImage->InitializeByItk( resampler->GetOutput() );
    this->m_InternalImage->SetVolume( resampler->GetOutput()->GetBufferPointer() );
  }
}

template < typename TPixel, unsigned int VImageDimension >
    void QmitkFiberProcessingView::InternalCalculateMaskFromPlanarFigure(
        itk::Image< TPixel, VImageDimension > *image, unsigned int axis, std::string nodeName )
{

  MITK_INFO << "InternalCalculateMaskFromPlanarFigure() start";

  typedef itk::Image< TPixel, VImageDimension > ImageType;
  typedef itk::CastImageFilter< ImageType, UCharImageType > CastFilterType;

  // Generate mask image as new image with same header as input image and
  // initialize with "1".
  UCharImageType::Pointer newMaskImage = UCharImageType::New();
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
  const Geometry2D *planarFigureGeometry2D = m_PlanarFigure->GetGeometry2D();
  const PlanarFigure::PolyLineType planarFigurePolyline = m_PlanarFigure->GetPolyLine( 0 );
  const Geometry3D *imageGeometry3D = m_InternalImage->GetGeometry( 0 );

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
  std::vector<vtkIdType> indices;

  unsigned int numberOfPoints = 0;

  for ( it = planarFigurePolyline.begin();
  it != planarFigurePolyline.end();
  ++it )
  {
    Point3D point3D;

    // Convert 2D point back to the local index coordinates of the selected
    // image
    Point2D point2D = it->Point;
    planarFigureGeometry2D->WorldToIndex(point2D, point2D);
    point2D[0] -= 0.5/m_UpsamplingFactor;
    point2D[1] -= 0.5/m_UpsamplingFactor;
    planarFigureGeometry2D->IndexToWorld(point2D, point2D);
    planarFigureGeometry2D->Map( point2D, point3D );

    // Polygons (partially) outside of the image bounds can not be processed
    // further due to a bug in vtkPolyDataToImageStencil
    if ( !imageGeometry3D->IsInside( point3D ) )
    {
      float bounds[2] = {0,0};
      bounds[0] =
          this->m_InternalImage->GetLargestPossibleRegion().GetSize().GetElement(i0);
      bounds[1] =
          this->m_InternalImage->GetLargestPossibleRegion().GetSize().GetElement(i1);

      imageGeometry3D->WorldToIndex( point3D, point3D );

//      if (point3D[i0]<0)
//        point3D[i0] = 0.5;
//      else if (point3D[i0]>bounds[0])
//        point3D[i0] = bounds[0]-0.5;

//      if (point3D[i1]<0)
//        point3D[i1] = 0.5;
//      else if (point3D[i1]>bounds[1])
//        point3D[i1] = bounds[1]-0.5;

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
  {
    ptIds[i] = i;
  }
  polyline->InsertNextCell( VTK_POLY_LINE, numberOfPoints, ptIds );


  // Extrude the generated contour polygon
  vtkLinearExtrusionFilter *extrudeFilter = vtkLinearExtrusionFilter::New();
  extrudeFilter->SetInput( polyline );
  extrudeFilter->SetScaleFactor( 1 );
  extrudeFilter->SetExtrusionTypeToNormalExtrusion();
  extrudeFilter->SetVector( 0.0, 0.0, 1.0 );

  // Make a stencil from the extruded polygon
  vtkPolyDataToImageStencil *polyDataToImageStencil = vtkPolyDataToImageStencil::New();
  polyDataToImageStencil->SetInput( extrudeFilter->GetOutput() );



  // Export from ITK to VTK (to use a VTK filter)
  typedef itk::VTKImageImport< UCharImageType > ImageImportType;
  typedef itk::VTKImageExport< UCharImageType > ImageExportType;

  typename ImageExportType::Pointer itkExporter = ImageExportType::New();
  itkExporter->SetInput( newMaskImage );

  vtkImageImport *vtkImporter = vtkImageImport::New();
  this->ConnectPipelines( itkExporter, vtkImporter );
  vtkImporter->Update();


  // Apply the generated image stencil to the input image
  vtkImageStencil *imageStencilFilter = vtkImageStencil::New();
  imageStencilFilter->SetInputConnection( vtkImporter->GetOutputPort() );
  imageStencilFilter->SetStencil( polyDataToImageStencil->GetOutput() );
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

  itk::ImageRegionConstIterator<UCharImageType>
      itmask(m_InternalImageMask3D, m_InternalImageMask3D->GetLargestPossibleRegion());
  itk::ImageRegionIterator<ImageType>
      itimage(image, image->GetLargestPossibleRegion());

  itmask = itmask.Begin();
  itimage = itimage.Begin();

  typename ImageType::SizeType lowersize = {{9999999999,9999999999,9999999999}};
  typename ImageType::SizeType uppersize = {{0,0,0}};
  while( !itmask.IsAtEnd() )
  {
    if(itmask.Get() == 0)
    {
      itimage.Set(0);
    }
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
  typedef itk::RegionOfInterestImageFilter< UCharImageType, UCharImageType > ROIMaskFilterType;
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
  const Geometry3D *pfImageGeometry3D = tmpImage2->GetGeometry( 0 );

  const Geometry3D *intImageGeometry3D = tmpImage->GetGeometry( 0 );

  typedef itk::ImageRegionIteratorWithIndex<UCharImageType> IteratorType;
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

void QmitkFiberProcessingView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}


void QmitkFiberProcessingView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

/* OnSelectionChanged is registered to SelectionService, therefore no need to
 implement SelectionService Listener explicitly */

void QmitkFiberProcessingView::UpdateGui()
{
  // are fiber bundles selected?
  if ( m_SelectedFB.empty() )
  {
    m_Controls->m_JoinBundles->setEnabled(false);
    m_Controls->m_SubstractBundles->setEnabled(false);
    m_Controls->m_ProcessFiberBundleButton->setEnabled(false);
    m_Controls->doExtractFibersButton->setEnabled(false);
  }
  else
  {
    m_Controls->m_ProcessFiberBundleButton->setEnabled(true);

    // one bundle and one planar figure needed to extract fibers
    if (!m_SelectedPF.empty())
      m_Controls->doExtractFibersButton->setEnabled(true);

    // more than two bundles needed to join/subtract
    if (m_SelectedFB.size() > 1)
    {
      m_Controls->m_JoinBundles->setEnabled(true);
      m_Controls->m_SubstractBundles->setEnabled(true);
    }
    else
    {
      m_Controls->m_JoinBundles->setEnabled(false);
      m_Controls->m_SubstractBundles->setEnabled(false);
    }
  }

  // are planar figures selected?
  if ( m_SelectedPF.empty() )
  {
    m_Controls->doExtractFibersButton->setEnabled(false);
    m_Controls->PFCompoANDButton->setEnabled(false);
    m_Controls->PFCompoORButton->setEnabled(false);
    m_Controls->PFCompoNOTButton->setEnabled(false);
    m_Controls->m_GenerateRoiImage->setEnabled(false);
  }
  else
  {
    if ( m_SelectedImage.IsNotNull() )
      m_Controls->m_GenerateRoiImage->setEnabled(true);
    else
      m_Controls->m_GenerateRoiImage->setEnabled(false);

    if (m_SelectedPF.size() > 1)
    {
      m_Controls->PFCompoANDButton->setEnabled(true);
      m_Controls->PFCompoORButton->setEnabled(true);
      m_Controls->PFCompoNOTButton->setEnabled(false);
    }
    else
    {
      m_Controls->PFCompoANDButton->setEnabled(false);
      m_Controls->PFCompoORButton->setEnabled(false);
      m_Controls->PFCompoNOTButton->setEnabled(true);
    }
  }
}

void QmitkFiberProcessingView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
  if ( !this->IsVisible() )
    return;

  //reset existing Vectors containing FiberBundles and PlanarFigures from a previous selection
  m_SelectedFB.clear();
  m_SelectedPF.clear();
  m_SelectedImage = NULL;

  for( std::vector<mitk::DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it )
  {
    mitk::DataNode::Pointer node = *it;
    if ( dynamic_cast<mitk::FiberBundleX*>(node->GetData()) )
      m_SelectedFB.push_back(node);
    else if (dynamic_cast<mitk::PlanarFigure*>(node->GetData()))
      m_SelectedPF.push_back(node);
    else if (dynamic_cast<mitk::Image*>(node->GetData()))
      m_SelectedImage = dynamic_cast<mitk::Image*>(node->GetData());
  }
  UpdateGui();
  GenerateStats();
}

void QmitkFiberProcessingView::ActionDrawPolygonTriggered()
{
  //  bool checked = m_Controls->m_PolygonButton->isChecked();
  //  if(!this->AssertDrawingIsPossible(checked))
  //    return;

  mitk::PlanarPolygon::Pointer figure = mitk::PlanarPolygon::New();
  figure->ClosedOn();
  this->AddFigureToDataStorage(figure, QString("Polygon%1").arg(++m_PolygonCounter));

  MITK_INFO << "PlanarPolygon created ...";

  mitk::DataStorage::SetOfObjects::ConstPointer _NodeSet = this->GetDefaultDataStorage()->GetAll();
  mitk::DataNode* node = 0;
  mitk::PlanarFigureInteractor::Pointer figureInteractor = 0;
  mitk::PlanarFigure* figureP = 0;

  for(mitk::DataStorage::SetOfObjects::ConstIterator it=_NodeSet->Begin(); it!=_NodeSet->End()
      ; it++)
  {
    node = const_cast<mitk::DataNode*>(it->Value().GetPointer());
    figureP = dynamic_cast<mitk::PlanarFigure*>(node->GetData());

    if(figureP)
    {
      figureInteractor = dynamic_cast<mitk::PlanarFigureInteractor*>(node->GetInteractor());

      if(figureInteractor.IsNull())
        figureInteractor = mitk::PlanarFigureInteractor::New("PlanarFigureInteractor", node);

      mitk::GlobalInteraction::GetInstance()->AddInteractor(figureInteractor);
    }
  }

}


void QmitkFiberProcessingView::ActionDrawEllipseTriggered()
{
  //bool checked = m_Controls->m_CircleButton->isChecked();
  //if(!this->AssertDrawingIsPossible(checked))
  //  return;

  mitk::PlanarCircle::Pointer figure = mitk::PlanarCircle::New();
  this->AddFigureToDataStorage(figure, QString("Circle%1").arg(++m_EllipseCounter));

  this->GetDataStorage()->Modified();
  MITK_INFO << "PlanarCircle created ...";

  //call

  mitk::DataStorage::SetOfObjects::ConstPointer _NodeSet = this->GetDefaultDataStorage()->GetAll();
  mitk::DataNode* node = 0;
  mitk::PlanarFigureInteractor::Pointer figureInteractor = 0;
  mitk::PlanarFigure* figureP = 0;

  for(mitk::DataStorage::SetOfObjects::ConstIterator it=_NodeSet->Begin(); it!=_NodeSet->End()
      ; it++)
  {
    node = const_cast<mitk::DataNode*>(it->Value().GetPointer());
    figureP = dynamic_cast<mitk::PlanarFigure*>(node->GetData());

    if(figureP)
    {
      figureInteractor = dynamic_cast<mitk::PlanarFigureInteractor*>(node->GetInteractor());

      if(figureInteractor.IsNull())
        figureInteractor = mitk::PlanarFigureInteractor::New("PlanarFigureInteractor", node);

      mitk::GlobalInteraction::GetInstance()->AddInteractor(figureInteractor);
    }
  }



}

void QmitkFiberProcessingView::Activated()
{

  MITK_INFO << "FB OPerations ACTIVATED()";
  /*

   mitk::DataStorage::SetOfObjects::ConstPointer _NodeSet = this->GetDefaultDataStorage()->GetAll();
   mitk::DataNode* node = 0;
   mitk::PlanarFigureInteractor::Pointer figureInteractor = 0;
   mitk::PlanarFigure* figure = 0;

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

   */


}

void QmitkFiberProcessingView::AddFigureToDataStorage(mitk::PlanarFigure* figure, const QString& name,
                                                            const char *propertyKey, mitk::BaseProperty *property )
{
  // initialize figure's geometry with empty geometry
  mitk::PlaneGeometry::Pointer emptygeometry = mitk::PlaneGeometry::New();
  figure->SetGeometry2D( emptygeometry );

  //set desired data to DataNode where Planarfigure is stored
  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
  newNode->SetName(name.toStdString());
  newNode->SetData(figure);

  newNode->AddProperty( "planarfigure.default.line.color", mitk::ColorProperty::New(1.0,0.0,0.0));
  newNode->AddProperty( "planarfigure.line.width", mitk::FloatProperty::New(2.0));
  newNode->AddProperty( "planarfigure.drawshadow", mitk::BoolProperty::New(true));


  newNode->AddProperty( "selected", mitk::BoolProperty::New(true) );
  newNode->AddProperty( "planarfigure.ishovering", mitk::BoolProperty::New(true) );
  newNode->AddProperty( "planarfigure.drawoutline", mitk::BoolProperty::New(true) );
  newNode->AddProperty( "planarfigure.drawquantities", mitk::BoolProperty::New(false) );
  newNode->AddProperty( "planarfigure.drawshadow", mitk::BoolProperty::New(true) );

  newNode->AddProperty( "planarfigure.line.width", mitk::FloatProperty::New(3.0) );
  newNode->AddProperty( "planarfigure.shadow.widthmodifier", mitk::FloatProperty::New(2.0) );
  newNode->AddProperty( "planarfigure.outline.width", mitk::FloatProperty::New(2.0) );
  newNode->AddProperty( "planarfigure.helperline.width", mitk::FloatProperty::New(2.0) );

//  PlanarFigureControlPointStyleProperty::Pointer styleProperty =
//    dynamic_cast< PlanarFigureControlPointStyleProperty* >( node->GetProperty( "planarfigure.controlpointshape" ) );
//  if ( styleProperty.IsNotNull() )
//  {
//    m_ControlPointShape = styleProperty->GetShape();
//  }

  newNode->AddProperty( "planarfigure.default.line.color", mitk::ColorProperty::New(1.0,1.0,1.0) );
  newNode->AddProperty( "planarfigure.default.line.opacity", mitk::FloatProperty::New(2.0) );
  newNode->AddProperty( "planarfigure.default.outline.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
  newNode->AddProperty( "planarfigure.default.outline.opacity", mitk::FloatProperty::New(2.0) );
  newNode->AddProperty( "planarfigure.default.helperline.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
  newNode->AddProperty( "planarfigure.default.helperline.opacity", mitk::FloatProperty::New(2.0) );
  newNode->AddProperty( "planarfigure.default.markerline.color", mitk::ColorProperty::New(0.0,0.0,0.0)  );
  newNode->AddProperty( "planarfigure.default.markerline.opacity", mitk::FloatProperty::New(2.0) );
  newNode->AddProperty( "planarfigure.default.marker.color", mitk::ColorProperty::New(1.0,1.0,1.0)  );
  newNode->AddProperty( "planarfigure.default.marker.opacity",mitk::FloatProperty::New(2.0) );

  newNode->AddProperty( "planarfigure.hover.line.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
  newNode->AddProperty( "planarfigure.hover.line.opacity", mitk::FloatProperty::New(2.0) );
  newNode->AddProperty( "planarfigure.hover.outline.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
  newNode->AddProperty( "planarfigure.hover.outline.opacity", mitk::FloatProperty::New(2.0) );
  newNode->AddProperty( "planarfigure.hover.helperline.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
  newNode->AddProperty( "planarfigure.hover.helperline.opacity", mitk::FloatProperty::New(2.0) );
  newNode->AddProperty( "planarfigure.hover.markerline.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
  newNode->AddProperty( "planarfigure.hover.markerline.opacity", mitk::FloatProperty::New(2.0) );
  newNode->AddProperty( "planarfigure.hover.marker.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
  newNode->AddProperty( "planarfigure.hover.marker.opacity", mitk::FloatProperty::New(2.0) );

  newNode->AddProperty( "planarfigure.selected.line.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
  newNode->AddProperty( "planarfigure.selected.line.opacity",mitk::FloatProperty::New(2.0) );
  newNode->AddProperty( "planarfigure.selected.outline.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
  newNode->AddProperty( "planarfigure.selected.outline.opacity", mitk::FloatProperty::New(2.0));
  newNode->AddProperty( "planarfigure.selected.helperline.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
  newNode->AddProperty( "planarfigure.selected.helperline.opacity",mitk::FloatProperty::New(2.0) );
  newNode->AddProperty( "planarfigure.selected.markerline.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
  newNode->AddProperty( "planarfigure.selected.markerline.opacity", mitk::FloatProperty::New(2.0) );
  newNode->AddProperty( "planarfigure.selected.marker.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
  newNode->AddProperty( "planarfigure.selected.marker.opacity",mitk::FloatProperty::New(2.0));















  // Add custom property, if available
  //if ( (propertyKey != NULL) && (property != NULL) )
  //{
  //  newNode->AddProperty( propertyKey, property );
  //}

  //get current selected DataNode -which should be a FiberBundle- and set PlanarFigure as child

  //this->GetDataStorage()->GetNodes()



  // mitk::FiberBundle::Pointer selectedFBNode = m_SelectedFBNodes.at(0);

  // figure drawn on the topmost layer / image
  this->GetDataStorage()->Add(newNode );

  std::vector<mitk::DataNode*> selectedNodes = GetDataManagerSelection();
  for(unsigned int i = 0; i < selectedNodes.size(); i++)
  {
    selectedNodes[i]->SetSelected(false);
  }
  //selectedNodes = m_SelectedPlanarFigureNodes->GetNodes();
  /*for(unsigned int i = 0; i < selectedNodes.size(); i++)
   {
   selectedNodes[i]->SetSelected(false);
   }
   */
  newNode->SetSelected(true);

  //Select(newNode);
}

void QmitkFiberProcessingView::DoFiberExtraction()
{
  if ( m_SelectedFB.empty() ){
    QMessageBox::information( NULL, "Warning", "No fibe bundle selected!");
    MITK_WARN("QmitkFiberProcessingView") << "no fibe bundle selected";
    return;
  }

  for (int i=0; i<m_SelectedFB.size(); i++)
  {
    mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>(m_SelectedFB.at(i)->GetData());
    mitk::PlanarFigure::Pointer roi = dynamic_cast<mitk::PlanarFigure*> (m_SelectedPF.at(0)->GetData());

//    std::vector<int> extFBset = fib->extractFibersByPF(roi);
//    mitk::FiberBundleX::Pointer extFB = fib->extractFibersById(extFBset);

    mitk::DataNode::Pointer node;
    node = mitk::DataNode::New();
    //fbNode->SetData(extFB);
    QString name(m_SelectedFB.at(0)->GetName().c_str());
    name += "_extracted";
    node->SetName(name.toStdString());
    GetDataStorage()->Add(node);
  }
}

void QmitkFiberProcessingView::GenerateAndComposite()
{
  mitk::PlanarFigureComposite::Pointer PFCAnd = mitk::PlanarFigureComposite::New();

  mitk::PlaneGeometry* currentGeometry2D = dynamic_cast<mitk::PlaneGeometry*>( const_cast<mitk::Geometry2D*>(GetActiveStdMultiWidget()->GetRenderWindow1()->GetRenderer()->GetCurrentWorldGeometry2D()));
  PFCAnd->SetGeometry2D(currentGeometry2D);
  PFCAnd->setOperationType(mitk::PFCOMPOSITION_AND_OPERATION);

  for( std::vector<mitk::DataNode::Pointer>::iterator it = m_SelectedPF.begin();
      it != m_SelectedPF.end(); ++it )
  {
    mitk::DataNode::Pointer nodePF = *it;
    mitk::PlanarFigure::Pointer tmpPF =  dynamic_cast<mitk::PlanarFigure*>( nodePF->GetData() );
    PFCAnd->addPlanarFigure( tmpPF );
    PFCAnd->addDataNode( nodePF );
    PFCAnd->setDisplayName("AND_COMPO");
  }

  AddCompositeToDatastorage(PFCAnd, NULL);
}

void QmitkFiberProcessingView::GenerateOrComposite()
{
  mitk::PlanarFigureComposite::Pointer PFCOr = mitk::PlanarFigureComposite::New();
  mitk::PlaneGeometry* currentGeometry2D = dynamic_cast<mitk::PlaneGeometry*>( const_cast<mitk::Geometry2D*>(GetActiveStdMultiWidget()->GetRenderWindow1()->GetRenderer()->GetCurrentWorldGeometry2D()));
  PFCOr->SetGeometry2D(currentGeometry2D);
  PFCOr->setOperationType(mitk::PFCOMPOSITION_OR_OPERATION);

  for( std::vector<mitk::DataNode::Pointer>::iterator it = m_SelectedPF.begin();
      it != m_SelectedPF.end(); ++it )
  {
    mitk::DataNode::Pointer nodePF = *it;
    mitk::PlanarFigure::Pointer tmpPF =  dynamic_cast<mitk::PlanarFigure*>( nodePF->GetData() );
    PFCOr->addPlanarFigure( tmpPF );
    PFCOr->addDataNode( nodePF );
    PFCOr->setDisplayName("OR_COMPO");
  }

  AddCompositeToDatastorage(PFCOr, NULL);
}

void QmitkFiberProcessingView::GenerateNotComposite()
{
  mitk::PlanarFigureComposite::Pointer PFCNot = mitk::PlanarFigureComposite::New();
  mitk::PlaneGeometry* currentGeometry2D = dynamic_cast<mitk::PlaneGeometry*>( const_cast<mitk::Geometry2D*>(GetActiveStdMultiWidget()->GetRenderWindow1()->GetRenderer()->GetCurrentWorldGeometry2D()));
  PFCNot->SetGeometry2D(currentGeometry2D);
  PFCNot->setOperationType(mitk::PFCOMPOSITION_NOT_OPERATION);

  for( std::vector<mitk::DataNode::Pointer>::iterator it = m_SelectedPF.begin();
      it != m_SelectedPF.end(); ++it )
  {
    mitk::DataNode::Pointer nodePF = *it;
    mitk::PlanarFigure::Pointer tmpPF =  dynamic_cast<mitk::PlanarFigure*>( nodePF->GetData() );
    PFCNot->addPlanarFigure( tmpPF );
    PFCNot->addDataNode( nodePF );
    PFCNot->setDisplayName("NOT_COMPO");
  }

  AddCompositeToDatastorage(PFCNot, NULL);
}

/* CLEANUP NEEDED */
void QmitkFiberProcessingView::AddCompositeToDatastorage(mitk::PlanarFigureComposite::Pointer pfcomp, mitk::DataNode::Pointer parentDataNode )
{
  mitk::DataNode::Pointer newPFCNode;
  newPFCNode = mitk::DataNode::New();
  newPFCNode->SetName( pfcomp->getDisplayName() );
  newPFCNode->SetData(pfcomp);
  newPFCNode->SetVisibility(true);

  switch (pfcomp->getOperationType()) {
    case 0:
    {
      if (!parentDataNode.IsNull()) {
        GetDataStorage()->Add(newPFCNode, parentDataNode);

      } else {
        GetDataStorage()->Add(newPFCNode);
      }

      //iterate through its childs
      for(int i=0; i<pfcomp->getNumberOfChildren(); ++i)
      {
        mitk::PlanarFigure::Pointer tmpPFchild = pfcomp->getChildAt(i);
        mitk::DataNode::Pointer savedPFchildNode = pfcomp->getDataNodeAt(i);

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
          pfcomp->replaceDataNodeAt(i, newChildPFCNode);

          AddCompositeToDatastorage(pfcompcast, newPFCNode); //the current PFCNode becomes the childs parent

          // remove savedNode here, cuz otherwise its children will change their position in the dataNodeManager
          // without having its parent anymore
          //GetDataStorage()->Remove(savedPFchildNode);
          if ( GetDataStorage()->Exists(savedPFchildNode)) {
            MITK_INFO << savedPFchildNode->GetName() << " exists in DS...trying to remove it";

          }else{
            MITK_INFO << "[ERROR] does NOT exist, but can I read its Name? " << savedPFchildNode->GetName();
          }
          // remove old child position in dataStorage
          GetDataStorage()->Remove(savedPFchildNode);
          if ( GetDataStorage()->Exists(savedPFchildNode)) {
            MITK_INFO << savedPFchildNode->GetName() << " still exists";
          }
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

          // replace the dataNode in PFComp DataNodeVector
          pfcomp->replaceDataNodeAt(i, newPFchildNode);


          if ( GetDataStorage()->Exists(savedPFchildNode)) {
            MITK_INFO << savedPFchildNode->GetName() << " exists in DS...trying to remove it";

          }
          else
          {
            MITK_INFO << "[ERROR] does NOT exist, but can I read its Name? " << savedPFchildNode->GetName();
          }
          // remove old child position in dataStorage
          GetDataStorage()->Remove(savedPFchildNode);

          if ( GetDataStorage()->Exists(savedPFchildNode))
          {
            MITK_INFO << savedPFchildNode->GetName() << " still exists";
          }

          MITK_INFO << "adding " << newPFchildNode->GetName() << " to " << newPFCNode->GetName();
          //add new child to datamanager with its new position as child of newPFCNode parent
          GetDataStorage()->Add(newPFchildNode, newPFCNode);
        }
      }
      GetDataStorage()->Modified();
      break;
    }
    case 1:
    {
      if (!parentDataNode.IsNull()) {
        MITK_INFO << "adding " << newPFCNode->GetName() << " to " << parentDataNode->GetName() ;
        GetDataStorage()->Add(newPFCNode, parentDataNode);

      } else {
        MITK_INFO << "adding " << newPFCNode->GetName();
        GetDataStorage()->Add(newPFCNode);

      }

      for(int i=0; i<pfcomp->getNumberOfChildren(); ++i)
      {
        mitk::PlanarFigure::Pointer tmpPFchild = pfcomp->getChildAt(i);
        mitk::DataNode::Pointer savedPFchildNode = pfcomp->getDataNodeAt(i);

        mitk::PlanarFigureComposite::Pointer pfcompcast= dynamic_cast<mitk::PlanarFigureComposite*>(tmpPFchild.GetPointer());
        if ( !pfcompcast.IsNull() )
        { // child is of type planar Figure composite
          // make new node of the child, cuz later the child has to be removed of its old position in datamanager
          // feed new dataNode with information of the savedDataNode, which is gonna be removed soon
          mitk::DataNode::Pointer newChildPFCNode;
          newChildPFCNode = mitk::DataNode::New();
          newChildPFCNode->SetData(tmpPFchild);
          newChildPFCNode->SetName( savedPFchildNode->GetName() );
          pfcompcast->setDisplayName(  savedPFchildNode->GetName()  ); //name might be changed in DataManager by user

          //update inside vector the dataNodePointer
          pfcomp->replaceDataNodeAt(i, newChildPFCNode);

          AddCompositeToDatastorage(pfcompcast, newPFCNode); //the current PFCNode becomes the childs parent


          // remove savedNode here, cuz otherwise its children will change their position in the dataNodeManager
          // without having its parent anymore
          //GetDataStorage()->Remove(savedPFchildNode);

          if ( GetDataStorage()->Exists(savedPFchildNode)) {
            MITK_INFO << savedPFchildNode->GetName() << " exists in DS...trying to remove it";

          }else{
            MITK_INFO << "[ERROR] does NOT exist, but can I read its Name? " << savedPFchildNode->GetName();

          }
          // remove old child position in dataStorage
          GetDataStorage()->Remove(savedPFchildNode);


          if ( GetDataStorage()->Exists(savedPFchildNode)) {
            MITK_INFO << savedPFchildNode->GetName() << " still exists";
          }
        } else {

          // child is not of type PlanarFigureComposite, so its one of the planarFigures
          // create new dataNode containing the data of the old dataNode, but position in dataManager will be
          // modified cuz we re setting a (new) parent.
          mitk::DataNode::Pointer newPFchildNode = mitk::DataNode::New();
          newPFchildNode->SetName(savedPFchildNode->GetName() );
          newPFchildNode->SetData(tmpPFchild);
          newPFchildNode->SetVisibility(true);

          // replace the dataNode in PFComp DataNodeVector
          pfcomp->replaceDataNodeAt(i, newPFchildNode);


          if ( GetDataStorage()->Exists(savedPFchildNode)) {
            MITK_INFO << savedPFchildNode->GetName() << " exists in DS...trying to remove it";

          }else{
            MITK_INFO << "[ERROR] does NOT exist, but can I read its Name? " << savedPFchildNode->GetName();

          }
          // remove old child position in dataStorage
          GetDataStorage()->Remove(savedPFchildNode);


          if ( GetDataStorage()->Exists(savedPFchildNode)) {
            MITK_INFO << savedPFchildNode->GetName() << " still exists";
          }

          MITK_INFO << "adding " << newPFchildNode->GetName() << " to " << newPFCNode->GetName();
          //add new child to datamanager with its new position as child of newPFCNode parent
          GetDataStorage()->Add(newPFchildNode, newPFCNode);
        }
      }
      GetDataStorage()->Modified();
      break;
    }
    case 2:
    {
      if (!parentDataNode.IsNull()) {
        MITK_INFO << "adding " << newPFCNode->GetName() << " to " << parentDataNode->GetName() ;
        GetDataStorage()->Add(newPFCNode, parentDataNode);
      }
      else
      {
        MITK_INFO << "adding " << newPFCNode->GetName();
        GetDataStorage()->Add(newPFCNode);
      }


      //iterate through its childs

      for(int i=0; i<pfcomp->getNumberOfChildren(); ++i)
      {
        mitk::PlanarFigure::Pointer tmpPFchild = pfcomp->getChildAt(i);
        mitk::DataNode::Pointer savedPFchildNode = pfcomp->getDataNodeAt(i);

        mitk::PlanarFigureComposite::Pointer pfcompcast= dynamic_cast<mitk::PlanarFigureComposite*>(tmpPFchild.GetPointer());
        if ( !pfcompcast.IsNull() )
        { // child is of type planar Figure composite
          // makeRemoveBundle new node of the child, cuz later the child has to be removed of its old position in datamanager
          // feed new dataNode with information of the savedDataNode, which is gonna be removed soon
          mitk::DataNode::Pointer newChildPFCNode;
          newChildPFCNode = mitk::DataNode::New();
          newChildPFCNode->SetData(tmpPFchild);
          newChildPFCNode->SetName( savedPFchildNode->GetName() );
          pfcompcast->setDisplayName(  savedPFchildNode->GetName()  ); //name might be changed in DataManager by user

          //update inside vector the dataNodePointer
          pfcomp->replaceDataNodeAt(i, newChildPFCNode);

          AddCompositeToDatastorage(pfcompcast, newPFCNode); //the current PFCNode becomes the childs parent


          // remove savedNode here, cuz otherwise its children will change their position in the dataNodeManager
          // without having its parent anymore
          //GetDataStorage()->Remove(savedPFchildNode);

          if ( GetDataStorage()->Exists(savedPFchildNode)) {
            MITK_INFO << savedPFchildNode->GetName() << " exists in DS...trying to remove it";

          }else{
            MITK_INFO << "[ERROR] does NOT exist, but can I read its Name? " << savedPFchildNode->GetName();
          }
          // remove old child position in dataStorage
          GetDataStorage()->Remove(savedPFchildNode);


          if ( GetDataStorage()->Exists(savedPFchildNode)) {
            MITK_INFO << savedPFchildNode->GetName() << " still exists";
          }


        } else {

          // child is not of type PlanarFigureComposite, so its one of the planarFigures
          // create new dataNode containing the data of the old dataNode, but position in dataManager will be
          // modified cuz we re setting a (new) parent.
          mitk::DataNode::Pointer newPFchildNode = mitk::DataNode::New();
          newPFchildNode->SetName(savedPFchildNode->GetName() );
          newPFchildNode->SetData(tmpPFchild);
          newPFchildNode->SetVisibility(true);

          // replace the dataNode in PFComp DataNodeVector
          pfcomp->replaceDataNodeAt(i, newPFchildNode);


          if ( GetDataStorage()->Exists(savedPFchildNode)) {
            MITK_INFO << savedPFchildNode->GetName() << " exists in DS...trying to remove it";

          }else{
            MITK_INFO << "[ERROR] does NOT exist, but can I read its Name? " << savedPFchildNode->GetName();

          }
          // remove old child position in dataStorage
          GetDataStorage()->Remove(savedPFchildNode);


          if ( GetDataStorage()->Exists(savedPFchildNode)) {
            MITK_INFO << savedPFchildNode->GetName() << " still exists";
          }

          MITK_INFO << "adding " << newPFchildNode->GetName() << " to " << newPFCNode->GetName();
          //add new child to datamanager with its new position as child of newPFCNode parent
          GetDataStorage()->Add(newPFchildNode, newPFCNode);
        }
      }
      GetDataStorage()->Modified();
      break;
    }
    default:
      MITK_INFO << "we have an UNDEFINED composition... ERROR" ;
      break;
  }
}


void QmitkFiberProcessingView::JoinBundles()
{
  if ( m_SelectedFB.size()<2 ){
    QMessageBox::information( NULL, "Warning", "Select at least two fiber bundles!");
    MITK_WARN("QmitkFiberProcessingView") << "Select at least two fiber bundles!";
    return;
  }

  std::vector<mitk::DataNode::Pointer>::const_iterator it = m_SelectedFB.begin();
  mitk::FiberBundleX::Pointer newBundle = dynamic_cast<mitk::FiberBundleX*>((*it)->GetData());
  QString name("");
  name += QString((*it)->GetName().c_str());
  ++it;
  for (it; it!=m_SelectedFB.end(); ++it)
  {
    newBundle = *newBundle+dynamic_cast<mitk::FiberBundleX*>((*it)->GetData());
    name += "+"+QString((*it)->GetName().c_str());
  }

  mitk::DataNode::Pointer fbNode = mitk::DataNode::New();
  fbNode->SetData(newBundle);
  fbNode->SetName(name.toStdString());
  fbNode->SetVisibility(true);
  GetDataStorage()->Add(fbNode);
}

void QmitkFiberProcessingView::SubstractBundles()
{
  if ( m_SelectedFB.size()<2 ){
    QMessageBox::information( NULL, "Warning", "Select at least two fiber bundles!");
    MITK_WARN("QmitkFiberProcessingView") << "Select at least two fiber bundles!";
    return;
  }

  std::vector<mitk::DataNode::Pointer>::const_iterator it = m_SelectedFB.begin();
  mitk::FiberBundleX::Pointer newBundle = dynamic_cast<mitk::FiberBundleX*>((*it)->GetData());
  QString name("");
  name += QString((*it)->GetName().c_str());
  ++it;
  for (it; it!=m_SelectedFB.end(); ++it)
  {
    newBundle = *newBundle-dynamic_cast<mitk::FiberBundleX*>((*it)->GetData());
    name += "-"+QString((*it)->GetName().c_str());
  }

  mitk::DataNode::Pointer fbNode = mitk::DataNode::New();
  fbNode->SetData(newBundle);
  fbNode->SetName(name.toStdString());
  fbNode->SetVisibility(true);
  GetDataStorage()->Add(fbNode);
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
      vtkSmartPointer<vtkPolyData> fiberPolyData = fib->GetFiberPolyData();
      vtkSmartPointer<vtkCellArray> vLines = fiberPolyData->GetLines();
      vLines->InitTraversal();
      int numberOfLines = vLines->GetNumberOfCells();

      stats += "Number of fibers: "+ QString::number(numberOfLines) + "\n";

      float length = 0;
      std::vector<float> lengths;
      for (int i=0; i<numberOfLines; i++)
      {
        vtkIdType   numPoints(0);
        vtkIdType*  points(NULL);
        vLines->GetNextCell ( numPoints, points );

        float l=0;
        for (unsigned int j=0; j<numPoints-1; j++)
        {
          double p1[3] = {0,0,0};
          fiberPolyData->GetPoint(points[j], p1);
          double p2[3] = {0,0,0};
          fiberPolyData->GetPoint(points[j+1], p2);

          float a = p1[0]-p2[0];
          float b = p1[1]-p2[1];
          float c = p1[2]-p2[2];

          float dist = std::sqrt(a*a+b*b+c*c);
          length += dist;
          l += dist;
        }
        lengths.push_back(l);
      }

      std::sort(lengths.begin(), lengths.end());

      if (numberOfLines>0)
        length /= numberOfLines;

      float dev=0;
      int count = 0;
      vLines->InitTraversal();
      for (int i=0; i<numberOfLines; i++)
      {
        vtkIdType   numPoints(0);
        vtkIdType*  points(NULL);
        vLines->GetNextCell ( numPoints, points );

        float l=0;
        for (unsigned int j=0; j<numPoints-1; j++)
        {
          double p1[3] = {0,0,0};
          fiberPolyData->GetPoint(points[j], p1);
          double p2[3] = {0,0,0};
          fiberPolyData->GetPoint(points[j+1], p2);

          float a = p1[0]-p2[0];
          float b = p1[1]-p2[1];
          float c = p1[2]-p2[2];

          float dist = std::sqrt(a*a+b*b+c*c);
          l += dist;
        }
        dev += (length-l)*(length-l);
        count++;
      }

      if (numberOfLines>0)
        dev /= numberOfLines;

      stats += "Mean fiber length: "+ QString::number(length/10) + "cm\n";
      stats += "Median fiber length: "+ QString::number(lengths.at(lengths.size()/2)/10) + "cm\n";
      stats += "Standard deviation: "+ QString::number(dev/10) + "cm\n";
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
        newNode = GenerateTractDensityImage(fib, false);
        name += "_TDI";
        break;
      case 1:
        newNode = GenerateTractDensityImage(fib, true);
        name += "_envelope";
        break;
      case 2:
        newNode = GenerateColorHeatmap(fib);
        break;
      case 3:
        newNode = GenerateFiberEndingsImage(fib);
        name += "_fiber_endings";
        break;
      case 4:
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
  generator->SetInvertImage(m_Controls->m_InvertCheckbox->isChecked());
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
mitk::DataNode::Pointer QmitkFiberProcessingView::GenerateTractDensityImage(mitk::FiberBundleX::Pointer fib, bool binary)
{
  typedef float OutPixType;
  typedef itk::Image<OutPixType, 3> OutImageType;

  itk::TractDensityImageFilter< OutImageType >::Pointer generator = itk::TractDensityImageFilter< OutImageType >::New();
  generator->SetFiberBundle(fib);
  generator->SetBinaryOutput(binary);
  generator->SetInvertImage(m_Controls->m_InvertCheckbox->isChecked());
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

