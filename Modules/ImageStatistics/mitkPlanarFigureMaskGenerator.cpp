#include <mitkPlanarFigureMaskGenerator.h>
#include <mitkBaseGeometry.h>
#include <mitkITKImageImport.h>
#include "mitkImageAccessByItk.h"
#include <mitkExtractImageFilter.h>
#include <mitkConvert2Dto3DImageFilter.h>
#include <mitkImageTimeSelector.h>
#include <mitkIOUtil.h>

#include <itkCastImageFilter.h>
#include <itkVTKImageExport.h>
#include <itkVTKImageImport.h>
#include <itkImageDuplicator.h>
#include <itkExceptionObject.h>
#include <itkLineIterator.h>

#include <vtkPoints.h>
#include <vtkImageStencil.h>
#include <vtkImageImport.h>
#include <vtkImageExport.h>
#include <vtkLassoStencilSource.h>
#include <vtkSmartPointer.h>



namespace mitk
{

void PlanarFigureMaskGenerator::SetPlanarFigure(mitk::PlanarFigure::Pointer planarFigure)
{
    if ( planarFigure.IsNull() )
    {
      throw std::runtime_error( "Error: planar figure empty!" );
    }

    const PlaneGeometry *planarFigurePlaneGeometry = planarFigure->GetPlaneGeometry();
    if ( planarFigurePlaneGeometry == nullptr )
    {
      throw std::runtime_error( "Planar-Figure not yet initialized!" );
    }

    const auto *planarFigureGeometry =
      dynamic_cast< const PlaneGeometry * >( planarFigurePlaneGeometry );
    if ( planarFigureGeometry == nullptr )
    {
      throw std::runtime_error( "Non-planar planar figures not supported!" );
    }

    if (planarFigure != m_PlanarFigure)
    {
        this->Modified();
        m_PlanarFigure = planarFigure;
    }

}

mitk::Image::ConstPointer PlanarFigureMaskGenerator::GetReferenceImage()
{
    if (IsUpdateRequired())
    {
        this->CalculateMask();
    }
    return m_ReferenceImage;
}

template < typename TPixel, unsigned int VImageDimension >
void PlanarFigureMaskGenerator::InternalCalculateMaskFromPlanarFigure(
  const itk::Image< TPixel, VImageDimension > *image, unsigned int axis )
{
  typedef itk::Image< unsigned short, 2 > MaskImage2DType;

  typename MaskImage2DType::Pointer maskImage = MaskImage2DType::New();
  maskImage->SetOrigin(image->GetOrigin());
  maskImage->SetSpacing(image->GetSpacing());
  maskImage->SetLargestPossibleRegion(image->GetLargestPossibleRegion());
  maskImage->SetBufferedRegion(image->GetBufferedRegion());
  maskImage->SetDirection(image->GetDirection());
  maskImage->SetNumberOfComponentsPerPixel(image->GetNumberOfComponentsPerPixel());
  maskImage->Allocate();
  maskImage->FillBuffer(1);

  // all PolylinePoints of the PlanarFigure are stored in a vtkPoints object.
  // These points are used by the vtkLassoStencilSource to create
  // a vtkImageStencil.
  const mitk::PlaneGeometry *planarFigurePlaneGeometry = m_PlanarFigure->GetPlaneGeometry();
  const typename PlanarFigure::PolyLineType planarFigurePolyline = m_PlanarFigure->GetPolyLine( 0 );
  const mitk::BaseGeometry *imageGeometry3D = m_inputImage->GetGeometry( 0 );
  // If there is a second poly line in a closed planar figure, treat it as a hole.
  PlanarFigure::PolyLineType planarFigureHolePolyline;

  if (m_PlanarFigure->GetPolyLinesSize() == 2)
    planarFigureHolePolyline = m_PlanarFigure->GetPolyLine(1);


  // Determine x- and y-dimensions depending on principal axis
  // TODO use plane geometry normal to determine that automatically, then check whether the PF is aligned with one of the three principal axis
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

  // store the polyline contour as vtkPoints object
  bool outOfBounds = false;
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  typename PlanarFigure::PolyLineType::const_iterator it;
  for ( it = planarFigurePolyline.begin();
    it != planarFigurePolyline.end();
    ++it )
  {
    Point3D point3D;

    // Convert 2D point back to the local index coordinates of the selected
    // image
    // Fabian: From PlaneGeometry documentation:
    // Converts a 2D point given in mm (pt2d_mm) relative to the upper-left corner of the geometry into the corresponding world-coordinate (a 3D point in mm, pt3d_mm).
    // To convert a 2D point given in units (e.g., pixels in case of an image) into a 2D point given in mm (as required by this method), use IndexToWorld.
    planarFigurePlaneGeometry->Map( *it, point3D );

    // Polygons (partially) outside of the image bounds can not be processed
    // further due to a bug in vtkPolyDataToImageStencil
    if ( !imageGeometry3D->IsInside( point3D ) )
    {
      outOfBounds = true;
    }

    imageGeometry3D->WorldToIndex( point3D, point3D );

    points->InsertNextPoint( point3D[i0], point3D[i1], 0 );
  }

  vtkSmartPointer<vtkPoints> holePoints = nullptr;

  if (!planarFigureHolePolyline.empty())
  {
    holePoints = vtkSmartPointer<vtkPoints>::New();

    Point3D point3D;
    PlanarFigure::PolyLineType::const_iterator end = planarFigureHolePolyline.end();

    for (it = planarFigureHolePolyline.begin(); it != end; ++it)
    {
      // Fabian: same as above
      planarFigurePlaneGeometry->Map(*it, point3D);
      imageGeometry3D->WorldToIndex(point3D, point3D);
      holePoints->InsertNextPoint(point3D[i0], point3D[i1], 0);
    }
  }

  // mark a malformed 2D planar figure ( i.e. area = 0 ) as out of bounds
  // this can happen when all control points of a rectangle lie on the same line = two of the three extents are zero
  double bounds[6] = {0, 0, 0, 0, 0, 0};
  points->GetBounds( bounds );
  bool extent_x = (fabs(bounds[0] - bounds[1])) < mitk::eps;
  bool extent_y = (fabs(bounds[2] - bounds[3])) < mitk::eps;
  bool extent_z = (fabs(bounds[4] - bounds[5])) < mitk::eps;

  // throw an exception if a closed planar figure is deformed, i.e. has only one non-zero extent
  if ( m_PlanarFigure->IsClosed() &&
    ((extent_x && extent_y) || (extent_x && extent_z)  || (extent_y && extent_z)))
  {
    mitkThrow() << "Figure has a zero area and cannot be used for masking.";
  }

  if ( outOfBounds )
  {
    throw std::runtime_error( "Figure at least partially outside of image bounds!" );
  }

  // create a vtkLassoStencilSource and set the points of the Polygon
  vtkSmartPointer<vtkLassoStencilSource> lassoStencil = vtkSmartPointer<vtkLassoStencilSource>::New();
  lassoStencil->SetShapeToPolygon();
  lassoStencil->SetPoints( points );

  vtkSmartPointer<vtkLassoStencilSource> holeLassoStencil = nullptr;

  if (holePoints.GetPointer() != nullptr)
  {
    holeLassoStencil = vtkSmartPointer<vtkLassoStencilSource>::New();
    holeLassoStencil->SetShapeToPolygon();
    holeLassoStencil->SetPoints(holePoints);
  }

  // Export from ITK to VTK (to use a VTK filter)
  typedef itk::VTKImageImport< MaskImage2DType > ImageImportType;
  typedef itk::VTKImageExport< MaskImage2DType > ImageExportType;

  typename ImageExportType::Pointer itkExporter = ImageExportType::New();
  itkExporter->SetInput( maskImage );
//  itkExporter->SetInput( castFilter->GetOutput() );

  vtkSmartPointer<vtkImageImport> vtkImporter = vtkSmartPointer<vtkImageImport>::New();
  this->ConnectPipelines( itkExporter, vtkImporter );

  // Apply the generated image stencil to the input image
  vtkSmartPointer<vtkImageStencil> imageStencilFilter = vtkSmartPointer<vtkImageStencil>::New();
  imageStencilFilter->SetInputConnection( vtkImporter->GetOutputPort() );
  imageStencilFilter->SetStencilConnection(lassoStencil->GetOutputPort());
  imageStencilFilter->ReverseStencilOff();
  imageStencilFilter->SetBackgroundValue( 0 );
  imageStencilFilter->Update();

  vtkSmartPointer<vtkImageStencil> holeStencilFilter = nullptr;

  if (holeLassoStencil.GetPointer() != nullptr)
  {
    holeStencilFilter = vtkSmartPointer<vtkImageStencil>::New();
    holeStencilFilter->SetInputConnection(imageStencilFilter->GetOutputPort());
    holeStencilFilter->SetStencilConnection(holeLassoStencil->GetOutputPort());
    holeStencilFilter->ReverseStencilOn();
    holeStencilFilter->SetBackgroundValue(0);
    holeStencilFilter->Update();
  }

  // Export from VTK back to ITK
  vtkSmartPointer<vtkImageExport> vtkExporter = vtkSmartPointer<vtkImageExport>::New();
  vtkExporter->SetInputConnection( holeStencilFilter.GetPointer() == nullptr
    ? imageStencilFilter->GetOutputPort()
    : holeStencilFilter->GetOutputPort());
  vtkExporter->Update();

  typename ImageImportType::Pointer itkImporter = ImageImportType::New();
  this->ConnectPipelines( vtkExporter, itkImporter );
  itkImporter->Update();

  typedef itk::ImageDuplicator< ImageImportType::OutputImageType > DuplicatorType;
  DuplicatorType::Pointer duplicator = DuplicatorType::New();
  duplicator->SetInputImage( itkImporter->GetOutput() );
  duplicator->Update();

  // Store mask
  m_InternalITKImageMask2D = duplicator->GetOutput();
}

template < typename TPixel, unsigned int VImageDimension >
void PlanarFigureMaskGenerator::InternalCalculateMaskFromOpenPlanarFigure(
  const itk::Image< TPixel, VImageDimension > *image, unsigned int axis )
{
  typedef itk::Image< unsigned short, 2 >       MaskImage2DType;
  typedef itk::LineIterator< MaskImage2DType >  LineIteratorType;
  typedef MaskImage2DType::IndexType            IndexType2D;
  typedef std::vector< IndexType2D >            IndexVecType;

  typename MaskImage2DType::Pointer maskImage = MaskImage2DType::New();
  maskImage->SetOrigin(image->GetOrigin());
  maskImage->SetSpacing(image->GetSpacing());
  maskImage->SetLargestPossibleRegion(image->GetLargestPossibleRegion());
  maskImage->SetBufferedRegion(image->GetBufferedRegion());
  maskImage->SetDirection(image->GetDirection());
  maskImage->SetNumberOfComponentsPerPixel(image->GetNumberOfComponentsPerPixel());
  maskImage->Allocate();
  maskImage->FillBuffer(0);

  // all PolylinePoints of the PlanarFigure are stored in a vtkPoints object.
  const mitk::PlaneGeometry *planarFigurePlaneGeometry = m_PlanarFigure->GetPlaneGeometry();
  const typename PlanarFigure::PolyLineType planarFigurePolyline = m_PlanarFigure->GetPolyLine( 0 );
  const mitk::BaseGeometry *imageGeometry3D = m_inputImage->GetGeometry( 0 );

  // Determine x- and y-dimensions depending on principal axis
  // TODO use plane geometry normal to determine that automatically, then check whether the PF is aligned with one of the three principal axis
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

  int numPolyLines = m_PlanarFigure->GetPolyLinesSize();
  for ( int lineId = 0; lineId < numPolyLines; ++lineId )
  {
    // store the polyline contour as vtkPoints object
    bool outOfBounds = false;
    IndexVecType pointIndices;
    typename PlanarFigure::PolyLineType::const_iterator it;
    for ( it = planarFigurePolyline.begin();
      it != planarFigurePolyline.end();
      ++it )
    {
      Point3D point3D;

      planarFigurePlaneGeometry->Map( *it, point3D );

      if ( !imageGeometry3D->IsInside( point3D ) )
      {
        outOfBounds = true;
      }

      imageGeometry3D->WorldToIndex( point3D, point3D );

      IndexType2D index2D;
      index2D[0] = point3D[i0];
      index2D[1] = point3D[i1];

      pointIndices.push_back( index2D );
    }

    if ( outOfBounds )
    {
      throw std::runtime_error( "Figure at least partially outside of image bounds!" );
    }

    for ( IndexVecType::const_iterator it = pointIndices.begin(); it != pointIndices.end()-1; ++it )
    {
      IndexType2D ind1 = *it;
      IndexType2D ind2 = *(it+1);

      LineIteratorType lineIt( maskImage, ind1, ind2 );
      while ( !lineIt.IsAtEnd() )
      {
        lineIt.Set( 1 );
        ++lineIt;
      }
    }
  }

  // Store mask
  m_InternalITKImageMask2D = maskImage;
}

bool PlanarFigureMaskGenerator::GetPrincipalAxis(
  const BaseGeometry *geometry, Vector3D vector,
  unsigned int &axis )
{
  vector.Normalize();
  for ( unsigned int i = 0; i < 3; ++i )
  {
    Vector3D axisVector = geometry->GetAxisVector( i );
    axisVector.Normalize();

    if ( fabs( fabs( axisVector * vector ) - 1.0) < mitk::eps )
    {
      axis = i;
      return true;
    }
  }

  return false;
}

void PlanarFigureMaskGenerator::CalculateMask()
{
    if (m_inputImage.IsNull())
    {
        MITK_ERROR << "Image is not set.";
    }

    if (m_PlanarFigure.IsNull())
    {
        MITK_ERROR << "PlanarFigure is not set.";
    }

    if (m_TimeStep != 0)
    {
        MITK_WARN << "Multiple TimeSteps are not supported in PlanarFigureMaskGenerator (yet).";
    }

    const BaseGeometry *imageGeometry = m_inputImage->GetGeometry();
    if ( imageGeometry == nullptr )
    {
      throw std::runtime_error( "Image geometry invalid!" );
    }

    if (m_inputImage->GetTimeSteps() > 0)
    {
        mitk::ImageTimeSelector::Pointer imgTimeSel = mitk::ImageTimeSelector::New();
        imgTimeSel->SetInput(m_inputImage);
        imgTimeSel->SetTimeNr(m_TimeStep);
        imgTimeSel->UpdateLargestPossibleRegion();
        m_InternalTimeSliceImage = imgTimeSel->GetOutput();
    }
    else
    {
        m_InternalTimeSliceImage = m_inputImage;
    }

    m_InternalITKImageMask2D = nullptr;
    const PlaneGeometry *planarFigurePlaneGeometry = m_PlanarFigure->GetPlaneGeometry();
    const auto *planarFigureGeometry = dynamic_cast< const PlaneGeometry * >( planarFigurePlaneGeometry );
    //const BaseGeometry *imageGeometry = m_inputImage->GetGeometry();

    // Find principal direction of PlanarFigure in input image
    unsigned int axis;
    if ( !this->GetPrincipalAxis( imageGeometry,
      planarFigureGeometry->GetNormal(), axis ) )
    {
      throw std::runtime_error( "Non-aligned planar figures not supported!" );
    }
    m_PlanarFigureAxis = axis;

    // Find slice number corresponding to PlanarFigure in input image
    itk::Image< unsigned short, 3 >::IndexType index;
    imageGeometry->WorldToIndex( planarFigureGeometry->GetOrigin(), index );

    unsigned int slice = index[axis];
    m_PlanarFigureSlice = slice;

    // extract image slice which corresponds to the planarFigure and store it in m_InternalImageSlice
    mitk::Image::ConstPointer inputImageSlice = extract2DImageSlice(axis, slice);
    //mitk::IOUtil::Save(inputImageSlice, "/home/fabian/inputSliceImage.nrrd");
    // Compute mask from PlanarFigure
    // rastering for open planar figure:
    if ( !m_PlanarFigure->IsClosed() )
    {
      AccessFixedDimensionByItk_1(inputImageSlice,
        InternalCalculateMaskFromOpenPlanarFigure,
        2, axis)
    }
    else//for closed planar figure
    {
      AccessFixedDimensionByItk_1(inputImageSlice,
                                  InternalCalculateMaskFromPlanarFigure,
                                  2, axis)
    }

    //convert itk mask to mitk::Image::Pointer and return it
    mitk::Image::Pointer planarFigureMaskImage;
    planarFigureMaskImage = mitk::GrabItkImageMemory(m_InternalITKImageMask2D);
    //mitk::IOUtil::Save(planarFigureMaskImage, "/home/fabian/planarFigureMaskImage.nrrd");

    //Convert2Dto3DImageFilter::Pointer sliceTo3DImageConverter = Convert2Dto3DImageFilter::New();
    //sliceTo3DImageConverter->SetInput(planarFigureMaskImage);
    //sliceTo3DImageConverter->Update();
    //mitk::IOUtil::Save(sliceTo3DImageConverter->GetOutput(), "/home/fabian/3DsliceImage.nrrd");

    m_ReferenceImage = inputImageSlice;
    //mitk::IOUtil::Save(m_ReferenceImage, "/home/fabian/referenceImage.nrrd");
    m_InternalMask = planarFigureMaskImage;
}

void PlanarFigureMaskGenerator::SetTimeStep(unsigned int timeStep)
{
    if (timeStep != m_TimeStep)
    {
        m_TimeStep = timeStep;
    }
}

mitk::Image::Pointer PlanarFigureMaskGenerator::GetMask()
{
    if (IsUpdateRequired())
    {
        this->CalculateMask();
        this->Modified();
    }

    m_InternalMaskUpdateTime = this->GetMTime();
    return m_InternalMask;
}

mitk::Image::ConstPointer PlanarFigureMaskGenerator::extract2DImageSlice(unsigned int axis, unsigned int slice)
{
    // Extract slice with given position and direction from image
    unsigned int dimension = m_InternalTimeSliceImage->GetDimension();

    if (dimension == 3)
    {
      ExtractImageFilter::Pointer imageExtractor = ExtractImageFilter::New();
      imageExtractor->SetInput( m_InternalTimeSliceImage );
      imageExtractor->SetSliceDimension( axis );
      imageExtractor->SetSliceIndex( slice );
      imageExtractor->Update();
      return imageExtractor->GetOutput();
    }
    else if(dimension == 2)
    {
      return m_InternalTimeSliceImage;
    }
    else
    {
      MITK_ERROR << "Unsupported image dimension. Dimension is: " << dimension << ". Only 2D and 3D images are supported.";
      return nullptr;
    }
}

bool PlanarFigureMaskGenerator::IsUpdateRequired() const
{
    unsigned long thisClassTimeStamp = this->GetMTime();
    unsigned long internalMaskTimeStamp = m_InternalMask->GetMTime();
    unsigned long planarFigureTimeStamp = m_PlanarFigure->GetMTime();
    unsigned long inputImageTimeStamp = m_inputImage->GetMTime();

    if (thisClassTimeStamp > m_InternalMaskUpdateTime) // inputs have changed
    {
        return true;
    }

    if (m_InternalMaskUpdateTime < planarFigureTimeStamp || m_InternalMaskUpdateTime < inputImageTimeStamp) // mask image has changed outside of this class
    {
        return true;
    }

    if (internalMaskTimeStamp > m_InternalMaskUpdateTime) // internal mask has been changed outside of this class
    {
        return true;
    }

    return false;
}

}

