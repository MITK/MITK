/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarFigureMaskGenerator.h>
#include <mitkBaseGeometry.h>
#include <mitkITKImageImport.h>
#include <mitkExtractSliceFilter.h>
#include <mitkConvert2Dto3DImageFilter.h>
#include <mitkImageTimeSelector.h>
#include <mitkIOUtil.h>

#include <itkCastImageFilter.h>
#include <itkVTKImageExport.h>
#include <itkVTKImageImport.h>
#include <itkImageDuplicator.h>
#include <itkMacro.h>
#include <itkLineIterator.h>

#include <vtkBox.h>
#include <vtkPoints.h>
#include <vtkImageStencil.h>
#include <vtkImageImport.h>
#include <vtkImageExport.h>
#include <vtkLassoStencilSource.h>
#include <vtkSmartPointer.h>

#include <algorithm>
#include <array>
#include <cmath>



namespace mitk
{

void PlanarFigureMaskGenerator::SetPlanarFigure(const mitk::PlanarFigure* planarFigure)
{
    if (nullptr == planarFigure )
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

void PlanarFigureMaskGenerator::InternalCalculateMaskFromClosedPlanarFigure(
  const itk::ImageRegion<2>& sliceRegion, unsigned int axis )
{
  typedef itk::Image< unsigned short, 2 > MaskImage2DType;

  MaskImage2DType::Pointer maskImage = MaskImage2DType::New();
  maskImage->SetRegions(sliceRegion);
  maskImage->Allocate();
  maskImage->FillBuffer(1);

  // The closed contours of the PlanarFigure are stored as vtkPoints objects.
  // These points are used by vtkLassoStencilSources to create
  // vtkImageStencils.
  const mitk::PlaneGeometry *planarFigurePlaneGeometry = m_PlanarFigure->GetPlaneGeometry();
  const mitk::BaseGeometry *imageGeometry3D = m_InputImage->GetGeometry( 0 );

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

  // Converts the 2D points of a contour to the local index coordinates of the selected image.
  const auto toSliceIndices = [&](const std::vector<Point2D>& contourPoints) {
    auto slicePoints = vtkSmartPointer<vtkPoints>::New();

    for (const auto& point : contourPoints)
    {
      Point3D point3D;
      planarFigurePlaneGeometry->Map(point, point3D);
      imageGeometry3D->WorldToIndex(point3D, point3D);
      slicePoints->InsertNextPoint(point3D[i0], point3D[i1], 0);
    }

    return slicePoints;
  };

  vtkSmartPointer<vtkPoints> points;
  std::vector<vtkSmartPointer<vtkPoints>> holes;

  for (const auto& contour : m_PlanarFigure->GetContours())
  {
    if (!contour.Closed)
      continue;

    if (contour.Role == PlanarFigure::ContourRole::Hole)
    {
      holes.push_back(toSliceIndices(contour.Points));
    }
    else if (points.GetPointer() == nullptr)
    {
      points = toSliceIndices(contour.Points);
    }
    else
    {
      mitkThrow() << "Figure has more than one outline and cannot be used for masking.";
    }
  }

  if (points.GetPointer() == nullptr)
    mitkThrow() << "Figure has no closed outline and cannot be used for masking.";

  // mark a malformed 2D planar figure ( i.e. area = 0 ) as out of bounds
  // this can happen when all control points of a rectangle lie on the same line = two of the three extents are zero
  double bounds[6] = {0};
  points->GetBounds(bounds);
  bool extent_x = (fabs(bounds[0] - bounds[1])) < mitk::eps;
  bool extent_y = (fabs(bounds[2] - bounds[3])) < mitk::eps;
  bool extent_z = (fabs(bounds[4] - bounds[5])) < mitk::eps;

  // throw an exception if a closed planar figure is deformed, i.e. has only one non-zero extent
  if (m_PlanarFigure->IsClosed() && ((extent_x && extent_y) || (extent_x && extent_z)  || (extent_y && extent_z)))
  {
    mitkThrow() << "Figure has a zero area and cannot be used for masking.";
  }

  // create a vtkLassoStencilSource and set the points of the Polygon
  vtkSmartPointer<vtkLassoStencilSource> lassoStencil = vtkSmartPointer<vtkLassoStencilSource>::New();
  lassoStencil->SetShapeToPolygon();
  lassoStencil->SetPoints(points);

  // Export from ITK to VTK (to use a VTK filter)
  typedef itk::VTKImageImport< MaskImage2DType > ImageImportType;
  typedef itk::VTKImageExport< MaskImage2DType > ImageExportType;

  ImageExportType::Pointer itkExporter = ImageExportType::New();
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

  // Every hole clears its inside again
  auto lastStencilFilter = imageStencilFilter;

  for (const auto& hole : holes)
  {
    auto holeLassoStencil = vtkSmartPointer<vtkLassoStencilSource>::New();
    holeLassoStencil->SetShapeToPolygon();
    holeLassoStencil->SetPoints(hole);

    auto holeStencilFilter = vtkSmartPointer<vtkImageStencil>::New();
    holeStencilFilter->SetInputConnection(lastStencilFilter->GetOutputPort());
    holeStencilFilter->SetStencilConnection(holeLassoStencil->GetOutputPort());
    holeStencilFilter->ReverseStencilOn();
    holeStencilFilter->SetBackgroundValue(0);
    holeStencilFilter->Update();

    lastStencilFilter = holeStencilFilter;
  }

  // Export from VTK back to ITK
  vtkSmartPointer<vtkImageExport> vtkExporter = vtkSmartPointer<vtkImageExport>::New();
  vtkExporter->SetInputConnection(lastStencilFilter->GetOutputPort());
  vtkExporter->Update();

  ImageImportType::Pointer itkImporter = ImageImportType::New();
  this->ConnectPipelines( vtkExporter, itkImporter );
  itkImporter->Update();

  typedef itk::ImageDuplicator< ImageImportType::OutputImageType > DuplicatorType;
  DuplicatorType::Pointer duplicator = DuplicatorType::New();
  duplicator->SetInputImage( itkImporter->GetOutput() );
  duplicator->Update();

  // Store mask
  m_InternalITKImageMask2D = duplicator->GetOutput();
}

void PlanarFigureMaskGenerator::InternalCalculateMaskFromOpenPlanarFigure(
  const itk::ImageRegion<2>& sliceRegion, unsigned int axis )
{
  typedef itk::Image< unsigned short, 2 >       MaskImage2DType;
  typedef itk::LineIterator< MaskImage2DType >  LineIteratorType;
  typedef MaskImage2DType::IndexType            IndexType2D;

  MaskImage2DType::Pointer maskImage = MaskImage2DType::New();
  maskImage->SetRegions(sliceRegion);
  maskImage->Allocate();
  maskImage->FillBuffer(0);

  const mitk::PlaneGeometry *planarFigurePlaneGeometry = m_PlanarFigure->GetPlaneGeometry();
  const mitk::BaseGeometry *imageGeometry3D = m_InputImage->GetGeometry( 0 );

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

  // The line iterator writes without bounds checks, so segments are clipped to the
  // slice first. Voxel i covers the continuous indices from i - 0.5 to i + 0.5.
  const auto &regionIndex = sliceRegion.GetIndex();
  const auto &regionSize = sliceRegion.GetSize();
  const double sliceBounds[6] = {
    regionIndex[0] - 0.5, regionIndex[0] + static_cast<double>(regionSize[0]) - 0.5,
    regionIndex[1] - 0.5, regionIndex[1] + static_cast<double>(regionSize[1]) - 0.5,
    0.0, 0.0 };

  const auto toIndex = [&](const double point[3]) {
    IndexType2D index;

    for (unsigned int i = 0; i < 2; ++i)
    {
      const auto last = regionIndex[i] + static_cast<IndexType2D::IndexValueType>(regionSize[i]) - 1;
      index[i] = std::clamp(static_cast<IndexType2D::IndexValueType>(std::llround(point[i])), regionIndex[i], last);
    }

    return index;
  };

  for (const auto& contour : m_PlanarFigure->GetContours())
  {
    // store the contour as continuous slice indices
    std::vector<std::array<double, 3>> points;
    for(const auto& point : contour.Points)
    {
      Point3D point3D;

      planarFigurePlaneGeometry->Map(point, point3D);
      imageGeometry3D->WorldToIndex(point3D, point3D);

      points.push_back({ point3D[i0], point3D[i1], 0.0 });
    }

    if (contour.Closed && !points.empty())
      points.push_back(points.front());

    for (size_t i = 0; i + 1 < points.size(); ++i)
    {
      double t1 = 0.0;
      double t2 = 1.0;
      double from[3];
      double to[3];
      int fromPlane = -1;
      int toPlane = -1;

      if (0 == vtkBox::IntersectWithLine(sliceBounds, points[i].data(), points[i + 1].data(), t1, t2, from, to, fromPlane, toPlane))
        continue;

      LineIteratorType lineIt(maskImage, toIndex(from), toIndex(to));
      while (!lineIt.IsAtEnd())
      {
        lineIt.Set(1);
        ++lineIt;
      }
    }
  }

  // Store mask
  m_InternalITKImageMask2D = maskImage;
}

bool PlanarFigureMaskGenerator::CheckPlanarFigureIsNotTilted(const PlaneGeometry* planarGeometry, const BaseGeometry *geometry)
{
  if (!planarGeometry) return false;
  if (!geometry) return false;

  unsigned int axis;
  return GetPrincipalAxis(geometry,planarGeometry->GetNormal(), axis);
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

    //normal mitk::eps is to pedantic for this check. See e.g. T27122
    //therefore choose a larger epsilon. The value was set a) as small as
    //possible but b) still allowing to datasets like in (T27122) to pass
    //when floating rounding errors sum up.
    const double epsilon = 5e-5;
    if ( fabs( fabs( axisVector * vector ) - 1.0) < epsilon)
    {
      axis = i;
      return true;
    }
  }

  return false;
}

void PlanarFigureMaskGenerator::CalculateMask()
{
    if (m_InputImage.IsNull())
    {
      mitkThrow() << "Image is not set.";
    }

    if (m_PlanarFigure.IsNull())
    {
      mitkThrow() << "PlanarFigure is not set.";
    }

    const BaseGeometry *imageGeometry = m_InputImage->GetGeometry();
    if ( imageGeometry == nullptr )
    {
      mitkThrow() << "Image geometry invalid!";
    }

    auto timePointImage = SelectImageByTimePoint(m_InputImage, m_TimePoint);

    if (timePointImage.IsNull()) mitkThrow() << "Cannot generate mask. Passed time point is not supported by input image.";

    m_InternalITKImageMask2D = nullptr;
    const PlaneGeometry *planarFigurePlaneGeometry = m_PlanarFigure->GetPlaneGeometry();
    const auto *planarFigureGeometry = dynamic_cast< const PlaneGeometry * >( planarFigurePlaneGeometry );

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
    mitk::Image::ConstPointer inputImageSlice = Extract2DImageSlice(timePointImage, axis, slice);

    // The mask is rasterized in index space and gets the geometry of the slice below,
    // so only the extent of the slice matters, not its pixel type.
    itk::ImageRegion<2>::SizeType sliceSize;
    sliceSize[0] = inputImageSlice->GetDimension(0);
    sliceSize[1] = inputImageSlice->GetDimension(1);
    const itk::ImageRegion<2> sliceRegion(sliceSize);

    // Compute mask from PlanarFigure
    // rastering for open planar figure:
    if ( !m_PlanarFigure->IsClosed() )
    {
      this->InternalCalculateMaskFromOpenPlanarFigure(sliceRegion, axis);
    }
    else//for closed planar figure
    {
      this->InternalCalculateMaskFromClosedPlanarFigure(sliceRegion, axis);
    }

    //convert itk mask to mitk::Image::Pointer and return it
    mitk::Image::Pointer planarFigureMaskImage;
    planarFigureMaskImage = mitk::GrabItkImageMemory(m_InternalITKImageMask2D);
    // GrabItkImageMemory can only derive a flat 2D geometry (z = 0) from the
    // ITK image; the mask has to lie on the plane of the reference slice.
    planarFigureMaskImage->SetClonedGeometry(inputImageSlice->GetGeometry());

    m_ReferenceImage = inputImageSlice;
    m_InternalMask = planarFigureMaskImage;
}

unsigned int PlanarFigureMaskGenerator::GetNumberOfMasks() const
{
  return 1;
}

mitk::Image::ConstPointer PlanarFigureMaskGenerator::DoGetMask(unsigned int)
{
    if (IsUpdateRequired())
    {
        this->CalculateMask();
        this->Modified();
    }

    m_InternalMaskUpdateTime = this->GetMTime();
    return m_InternalMask;
}

mitk::Image::ConstPointer PlanarFigureMaskGenerator::Extract2DImageSlice(const Image* input, unsigned int axis, unsigned int slice) const
{
    // Extract slice with given position and direction from image
    unsigned int dimension = input->GetDimension();

    if (dimension == 3)
    {
      // The slice plane is built in index space of the image, so the pixels of the
      // slice correspond to the image indices along the two remaining axes.
      AnatomicalPlane orientation = AnatomicalPlane::Axial;
      unsigned int width = input->GetDimension(0);
      unsigned int height = input->GetDimension(1);

      switch (axis)
      {
        case 0:
          orientation = AnatomicalPlane::Sagittal;
          width = input->GetDimension(1);
          height = input->GetDimension(2);
          break;
        case 1:
          orientation = AnatomicalPlane::Coronal;
          width = input->GetDimension(0);
          height = input->GetDimension(2);
          break;
        default:
          break;
      }

      const BaseGeometry* geometry = input->GetGeometry();

      auto plane = PlaneGeometry::New();
      plane->InitializeStandardPlane(width, height, geometry->GetIndexToWorldTransform(), orientation, static_cast<ScalarType>(slice));

      // ExtractSliceFilter expects the plane origin at the corner of the first pixel
      // and shifts it to the pixel center itself, both for sampling and for the
      // geometry of the extracted slice.
      Point3D cornerIndex;
      FillVector3D(cornerIndex, -0.5, -0.5, 0.0);
      plane->SetOrigin(plane->GetIndexToWorldTransform()->TransformPoint(cornerIndex));

      auto extractor = ExtractSliceFilter::New();
      extractor->SetInput(input);
      extractor->SetWorldGeometry(plane);
      extractor->SetInPlaneResampleExtentByGeometry(true);
      extractor->SetInterpolationMode(ExtractSliceFilter::RESLICE_NEAREST);
      extractor->SetVtkOutputRequest(false);
      extractor->SetResliceTransformByGeometry(geometry);
      extractor->Update();

      return extractor->GetOutput();
    }
    else if(dimension == 2)
    {
      return input;
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
    unsigned long inputImageTimeStamp = m_InputImage->GetMTime();

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

