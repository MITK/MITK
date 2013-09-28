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

#include <mitkLabelSetImageToSurfaceFilter.h>

#include <vtkSmartPointer.h>
#include <vtkMarchingCubes.h>
#include <vtkLinearTransform.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageMedian3D.h>
#include <vtkImageResample.h>
#include <vtkCellArray.h>

#include <mitkImageAccessByItk.h>

#include <itkBinaryThresholdImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkAntiAliasBinaryImageFilter.h>
#include <itkBinaryMask3DMeshSource.h>
#include <itkBinaryMinMaxCurvatureFlowImageFilter.h>
#include <itkImageToVTKImageFilter.h>
#include <itkQuadEdgeMesh.h>
#include <itkImageRegionIterator.h>
#include <itkNumericTraits.h>


mitk::LabelSetImageToSurfaceFilter::LabelSetImageToSurfaceFilter() :
m_GenerateAllLabels(false),
m_RequestedLabel(1),
m_BackgroundLabel(0),
m_UseSmoothing(0)
{
}

mitk::LabelSetImageToSurfaceFilter::~LabelSetImageToSurfaceFilter()
{
}

void mitk::LabelSetImageToSurfaceFilter::SetInput(const mitk::Image *image)
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(0, const_cast< mitk::Image * >( image ) );
}
/*
void mitk::LabelSetImageToSurfaceFilter::SetObserver(mitk::ProcessObserver::Pointer observer)
{
   m_Observer = observer;
}
*/
const mitk::Image *mitk::LabelSetImageToSurfaceFilter::GetInput(void)
{
  if (this->GetNumberOfInputs() < 1)
  {
    return 0;
  }

  return static_cast<const mitk::Image * >
    ( this->ProcessObject::GetInput(0) );
}

void mitk::LabelSetImageToSurfaceFilter::GenerateOutputInformation()
{
  itkDebugMacro(<<"GenerateOutputInformation()");
}

void mitk::LabelSetImageToSurfaceFilter::GenerateData()
{
  mitk::Image::ConstPointer inputImage = const_cast<mitk::Image*>( this->GetInput() );
  if ( inputImage.IsNull() ) return;

  mitk::Surface* outputSurface = this->GetOutput( );
  if (!outputSurface) return;

  m_LowerThreshold = m_RequestedLabel;
  m_UpperThreshold = m_RequestedLabel;

  AccessFixedDimensionByItk_1( inputImage, ITKProcessing2, 3, outputSurface );
}

template < typename TPixel, unsigned int VImageDimension >
void mitk::LabelSetImageToSurfaceFilter::ITKProcessing( itk::Image<TPixel, VImageDimension>* input, mitk::Surface* surface )
{
   typedef itk::Image<TPixel, VImageDimension> InputImageType;
  // typedef itk::Image< unsigned char, VImageDimension> BinaryImageType;
   typedef itk::BinaryThresholdImageFilter< InputImageType, InputImageType > BinaryThresholdFilterType;

   typedef double FloatPixelType;
   typedef itk::Image<FloatPixelType, VImageDimension> RealImageType;
//   typedef itk::CastImageFilter< BinaryImageType, FloatImageType> CastImageFilterType;

   typedef itk::AntiAliasBinaryImageFilter< InputImageType, RealImageType > AntiAliasFilterType;
   typedef itk::ImageToVTKImageFilter< InputImageType > ConvertType;

   mitk::Vector3D spacing = this->GetInput()->GetGeometry(0)->GetSpacing();

   typename BinaryThresholdFilterType::Pointer thresholdFilter = BinaryThresholdFilterType::New();
   thresholdFilter->SetInput( input );
   thresholdFilter->SetLowerThreshold(m_LowerThreshold);
   thresholdFilter->SetUpperThreshold(m_UpperThreshold);
   thresholdFilter->SetOutsideValue(0);
   thresholdFilter->SetInsideValue(1);
   thresholdFilter->Update();
//   thresholdFilter->ReleaseDataFlagOn();

//   typename CastImageFilterType::Pointer castFilter = CastImageFilterType::New();
//   castFilter->SetInput( thresholdFilter->GetOutput() );

/*
   typename AntiAliasFilterType::Pointer antiAliasFilter;
   // todo: check why input parameters are not used in the ITK filter
   antiAliasFilter = AntiAliasFilterType::New();
   antiAliasFilter->SetMaximumRMSError(0.01);
   antiAliasFilter->SetNumberOfLayers(2);
   antiAliasFilter->SetNumberOfIterations(15);
   antiAliasFilter->SetInput( thresholdFilter->GetOutput() );
//   antiAliasFilter->ReleaseDataFlagOn();

   if (m_Observer.IsNotNull())
   {
      m_Observer->SetRemainingProgress(100);
      thresholdFilter->AddObserver(itk::AnyEvent(), m_Observer);
      antiAliasFilter->AddObserver(itk::AnyEvent(), m_Observer);
      m_Observer->AddStepsToDo(100);
   }
*/
   //antiAliasFilter->Update();

/*
   if (m_Observer.IsNotNull())
   {
      thresholdFilter->RemoveAllObservers();
      antiAliasFilter->RemoveAllObservers();
      m_Observer->SetRemainingProgress(100);
   }
*/
//   typename FloatImageType::Pointer outputFloatImage;

//   outputFloatImage = antiAliasFilter->GetOutput();
//   outputFloatImage->DisconnectPipeline();

   typename ConvertType::Pointer connector = ConvertType::New();
   connector->SetInput( thresholdFilter->GetOutput() );
   connector->ReleaseDataFlagOn();

   //connector->Update();
/*
   vtkSmartPointer<vtkImageResample> imageresample = vtkSmartPointer<vtkImageResample>::New();
   imageresample->SetInput(connector->GetOutput());
   imageresample->SetAxisOutputSpacing(0, spacing[0] * 1.5);
   imageresample->SetAxisOutputSpacing(1, spacing[1] * 1.5);
   imageresample->SetAxisOutputSpacing(2, spacing[2] * 1.5);
   imageresample->UpdateInformation();
   imageresample->Update();
*/
   vtkSmartPointer<vtkImageMedian3D> median = vtkSmartPointer<vtkImageMedian3D>::New();
   median->SetInput(connector->GetOutput());
   median->SetKernelSize(3,3,3);
   median->ReleaseDataFlagOn();
   median->UpdateInformation();
  // median->Update();

   vtkSmartPointer<vtkImageChangeInformation> indexCoordinatesImageFilter = vtkSmartPointer<vtkImageChangeInformation>::New();
   indexCoordinatesImageFilter->SetInput(median->GetOutput());
   indexCoordinatesImageFilter->SetOutputOrigin(0.0,0.0,0.0);

   vtkSmartPointer<vtkMarchingCubes> marching = vtkSmartPointer<vtkMarchingCubes>::New();
   marching->ComputeNormalsOn();
   marching->ComputeGradientsOn();
   marching->SetInput(indexCoordinatesImageFilter->GetOutput());
   marching->SetValue(1,1);
   marching->ReleaseDataFlagOn();

   marching->Update();

   vtkPolyData* polydata = marching->GetOutput();

   if ( (!polydata) || (!polydata->GetNumberOfPoints()) )
      mitkThrow() << "Marching cubes filter has failed";


  vtkPoints * points = polydata->GetPoints();
  vtkMatrix4x4 *vtkmatrix = vtkMatrix4x4::New();
  this->GetInput()->GetGeometry(0)->GetVtkTransform()->GetMatrix(vtkmatrix);
  double (*matrix)[4] = vtkmatrix->Element;

  unsigned int i,j;
  for(i=0;i<3;++i)
    for(j=0;j<3;++j)
      matrix[i][j]/=spacing[j];

  unsigned int n = points->GetNumberOfPoints();
  vtkFloatingPointType point[3];

  for (i = 0; i < n; i++)
  {
    points->GetPoint(i, point);
    mitkVtkLinearTransformPoint(matrix,point,point);
    points->SetPoint(i, point);
  }
  vtkmatrix->Delete();

  surface->SetVtkPolyData( polydata );
}


template < typename TPixel, unsigned int VImageDimension >
void mitk::LabelSetImageToSurfaceFilter::ITKProcessing2( itk::Image<TPixel, VImageDimension>* input, mitk::Surface* surface )
{
  typedef itk::Image<TPixel, VImageDimension> InputImageType;
  typedef itk::Image<float, VImageDimension> RealImageType;
  typedef itk::BinaryThresholdImageFilter< InputImageType, RealImageType > BinaryThresholdFilterType;
  typedef itk::BinaryMinMaxCurvatureFlowImageFilter< InputImageType, RealImageType > SmoothingFilterType;
  typedef itk::Mesh<double> MeshType;
  typedef itk::BinaryMask3DMeshSource< RealImageType, MeshType > MeshSourceType;

  mitk::Vector3D spacing = this->GetInput()->GetGeometry(0)->GetSpacing();

  typename BinaryThresholdFilterType::Pointer thresholdFilter = BinaryThresholdFilterType::New();
  thresholdFilter->SetInput( input );
  thresholdFilter->SetLowerThreshold(m_LowerThreshold);
  thresholdFilter->SetUpperThreshold(m_UpperThreshold);
  thresholdFilter->SetOutsideValue(0);
  thresholdFilter->SetInsideValue(1);
//  thresholdFilter->ReleaseDataFlagOn();
  thresholdFilter->Update();
/*
  typename SmoothingFilterType::Pointer filter = SmoothingFilterType::New();
  filter->SetNumberOfIterations(100);
  filter->SetThreshold(1.0);
  filter->SetInput( thresholdFilter->GetOutput() );
  filter->Update();
*/
  MeshSourceType::Pointer meshSource = MeshSourceType::New();
  meshSource->SetInput( thresholdFilter->GetOutput() );
  meshSource->SetObjectValue( 1 );
  meshSource->Update();

  if ( (!meshSource->GetNumberOfNodes()) || (!meshSource->GetNumberOfCells()) )
    mitkThrow() << "Mesh creation failed";

  MeshType::Pointer mesh = meshSource->GetOutput();

  unsigned int numberOfPoints = mesh->GetNumberOfPoints();

  vtkPoints* vpoints = vtkPoints::New( VTK_DOUBLE );
  vpoints->SetNumberOfPoints(numberOfPoints);

  typedef MeshType::PointsContainer    PointsContainer;
  const MeshType::PointsContainer * points = mesh->GetPoints();

  typedef MeshType::PointIdentifier PointIdentifier;
  typedef PointsContainer::ConstIterator    PointIterator;

  std::map< MeshType::PointIdentifier, MeshType::PointIdentifier > IdMap;
  typename MeshType::PointIdentifier k = 0;

  if( points )
  {
    MeshType::PointsContainer::ConstIterator pointIterator = points->Begin();
    MeshType::PointsContainer::ConstIterator pointEnd = points->End();

    while( pointIterator != pointEnd )
    {
      MeshType::PointType point = pointIterator.Value();
      vpoints->SetPoint(pointIterator.Index(), point[0], point[1], point[2]);
      IdMap[ pointIterator.Index() ] = k++;
      pointIterator++;
    }
  }

  unsigned int numberOfVertices = 0;
  unsigned int numberOfEdges = 0;
  unsigned int numberOfPolygons = 0;

  typedef  MeshType::CellsContainer     CellsContainer;
  const CellsContainer * cells = mesh->GetCells();

  typedef CellsContainer::ConstIterator     CellIterator;

  // create vtk cells and allocate
  vtkCellArray* cellArray = vtkCellArray::New();

  if( cells )
  {
  CellIterator cellIterator = cells->Begin();
  CellIterator cellEnd = cells->End();

  while( cellIterator != cellEnd )
    {
    switch( cellIterator.Value()->GetType() )
      {
      case 0: //VERTEX_CELL:
        numberOfVertices++;
        break;
      case 1: //LINE_CELL:
      case 7: //QUADRATIC_EDGE_CELL:
        numberOfEdges++;
        break;
      case 2: //TRIANGLE_CELL:
      case 3: //QUADRILATERAL_CELL:
      case 4: //POLYGON_CELL:
      case 8: //QUADRATIC_TRIANGLE_CELL:
        numberOfPolygons++;
        break;
      default:
        std::cerr << "Unhandled cell (volumic?)." << std::endl;
      }
    cellIterator++;
    }

  cellArray->Allocate(numberOfPolygons);

  // POLYGONS
  if( numberOfPolygons )
    {
    typedef MeshType::CellType::PointIdIterator         PointIdIterator;
    cellIterator = cells->Begin();
    while( cellIterator != cellEnd )
      {
      MeshType::CellType * cellPointer = cellIterator.Value();
      switch( cellIterator.Value()->GetType() )
        {
        case 2: //TRIANGLE_CELL:
        case 3: //QUADRILATERAL_CELL:
        case 4: //POLYGON_CELL:
        case 8: //QUADRATIC_TRIANGLE_CELL:
            {
            PointIdIterator pointIdIterator = cellPointer->PointIdsBegin();
            PointIdIterator pointIdEnd = cellPointer->PointIdsEnd();
            cellArray->InsertNextCell(cellPointer->GetNumberOfPoints());
            while( pointIdIterator != pointIdEnd )
              {
              cellArray->InsertCellPoint(IdMap[ *pointIdIterator ]);
              pointIdIterator++;
              }
            break;
            }
        default:
          break;
        }
      cellIterator++;
      }
    }
  }

  vtkSmartPointer<vtkPolyData> polydata =  vtkSmartPointer<vtkPolyData>::New();
  polydata->SetPoints(vpoints);
  polydata->SetPolys(cellArray);

  if ( (!polydata) || (!polydata->GetNumberOfPoints()) )
    mitkThrow() << "Mesh creation failed";

//  vtkPoints * points = polydata->GetPoints();
  vtkMatrix4x4 *vtkmatrix = vtkMatrix4x4::New();
  this->GetInput()->GetGeometry(0)->GetVtkTransform()->GetMatrix(vtkmatrix);
  double (*matrix)[4] = vtkmatrix->Element;

  unsigned int i,j;
  for(i=0;i<3;++i)
  for(j=0;j<3;++j)
    matrix[i][j]/=spacing[j];

  unsigned int n = vpoints->GetNumberOfPoints();
  vtkFloatingPointType point[3];

  for (i = 0; i < n; i++)
  {
  vpoints->GetPoint(i, point);
  mitkVtkLinearTransformPoint(matrix,point,point);
  vpoints->SetPoint(i, point);
  }
  vtkmatrix->Delete();

  surface->SetVtkPolyData( polydata );
}
