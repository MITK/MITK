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

#include "mitkContourUtils.h"
#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"
#include "mitkInstantiateAccessFunctions.h"
#include "ipSegmentation.h"

#include "mitkIOUtil.h"


//testing some stuff
#include "mitkContourModelToPointSetFilter.h"
#include "mitkPointSet.h"
#include "mitkContourModel.h"
#include "itkPolylineMask2DImageFilter.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"
#include "itkVTKPolyDataWriter.h"
#include "itkPoint.h"
#include "mitkImage.h"
#include "itkPolyLineParametricPath.h"

#include "mitkSurface.h"
#include "mitkContourModelToSurfaceFilter.h"
#include "mitkSurfaceToImageFilter.h"
#include "mitkImageWriter.h"
#include "mitkSurfaceVtkWriter.h"
#include "vtkPolyDataToImageStencil.h"
#include "vtkImageStencil.h"
#include "mitkImageVtkAccessor.h"
#include "vtkMetaImageWriter.h"
#include "vtkXMLImageDataWriter.h"
#include "vtkImageStencilToImage.h"
#include "vtkPNGWriter.h"
#include "vtkPointData.h"
#include "vtkDataArray.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkImageData.h"
#include "vtkSphereSource.h"
#include "vtkVersion.h"

#include <vtkCutter.h>
#include <vtkPlane.h>
#include <vtkStripper.h>
#include <vtkLinearExtrusionFilter.h>
#include <vtkXMLPolyDataWriter.h>
#include "vtkTriangleFilter.h"
#include "vtkContourFilter.h"
#include "vtkImageStencilData.h"
#include "vtkXMLImageDataWriter.h"
#include "mitkIOUtil.h"
#include "mitkImagePixelWriteAccessor.h"

#define InstantiateAccessFunction_ItkCopyFilledContourToSlice(pixelType, dim) \
  template void mitk::ContourUtils::ItkCopyFilledContourToSlice(itk::Image<pixelType,dim>*, const mitk::Image*, int);

// explicitly instantiate the 2D version of this method
//InstantiateAccessFunctionForFixedDimension(ItkCopyFilledContourToSlice, 2);

mitk::ContourUtils::ContourUtils()
{
}

mitk::ContourUtils::~ContourUtils()
{
}


mitk::ContourModel::Pointer mitk::ContourUtils::ProjectContourTo2DSlice(Image* slice, Contour* contourIn3D, bool itkNotUsed( correctionForIpSegmentation ), bool constrainToInside)
{
  mitk::Contour::PointsContainerIterator it = contourIn3D->GetPoints()->Begin();
  mitk::Contour::PointsContainerIterator end = contourIn3D->GetPoints()->End();

  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  while(it!=end)
  {
    contour->AddVertex(it.Value());
    it++;
  }
  return this->ProjectContourTo2DSlice(slice, contour, false/*not used*/, constrainToInside );
}

mitk::ContourModel::Pointer mitk::ContourUtils::ProjectContourTo2DSlice(Image* slice, ContourModel* contourIn3D, bool itkNotUsed( correctionForIpSegmentation ), bool constrainToInside)
{
  if ( !slice || !contourIn3D ) return NULL;

  ContourModel::Pointer projectedContour = ContourModel::New();
  projectedContour->Initialize(*contourIn3D);

  const Geometry3D* sliceGeometry = slice->GetGeometry();

  int numberOfTimesteps = contourIn3D->GetTimeGeometry()->CountTimeSteps();

  for(int currentTimestep = 0; currentTimestep < numberOfTimesteps; currentTimestep++)
  {
    ContourModel::VertexIterator iter = contourIn3D->Begin(currentTimestep);
    ContourModel::VertexIterator end = contourIn3D->End(currentTimestep);

    while( iter != end)
    {
      Point3D currentPointIn3D = (*iter)->Coordinates;

      Point3D projectedPointIn2D;
      projectedPointIn2D.Fill(0.0);
      sliceGeometry->WorldToIndex( currentPointIn3D, projectedPointIn2D );
      // MITK_INFO << "world point " << currentPointIn3D << " in index is " << projectedPointIn2D;

      if ( !sliceGeometry->IsIndexInside( projectedPointIn2D ) && constrainToInside )
      {
        MITK_INFO << "**" << currentPointIn3D << " is " << projectedPointIn2D << " --> correct it (TODO)" << std::endl;
      }

      projectedContour->AddVertex( projectedPointIn2D, currentTimestep );
      iter++;
    }
  }

  return projectedContour;
}


mitk::ContourModel::Pointer mitk::ContourUtils::BackProjectContourFrom2DSlice(const Geometry3D* sliceGeometry, Contour* contourIn2D, bool itkNotUsed( correctionForIpSegmentation ) )
{
  mitk::Contour::PointsContainerIterator it = contourIn2D->GetPoints()->Begin();
  mitk::Contour::PointsContainerIterator end = contourIn2D->GetPoints()->End();

  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  while(it!=end)
  {
    contour->AddVertex(it.Value());
    it++;
  }
  return this->BackProjectContourFrom2DSlice(sliceGeometry, contour, false/*not used*/);
}

mitk::ContourModel::Pointer mitk::ContourUtils::BackProjectContourFrom2DSlice(const Geometry3D* sliceGeometry, ContourModel* contourIn2D, bool itkNotUsed( correctionForIpSegmentation ) )
{
  if ( !sliceGeometry || !contourIn2D ) return NULL;

  ContourModel::Pointer worldContour = ContourModel::New();
  worldContour->Initialize(*contourIn2D);

  int numberOfTimesteps = contourIn2D->GetTimeGeometry()->CountTimeSteps();

  for(int currentTimestep = 0; currentTimestep < numberOfTimesteps; currentTimestep++)
  {
  ContourModel::VertexIterator iter = contourIn2D->Begin(currentTimestep);
  ContourModel::VertexIterator end = contourIn2D->End(currentTimestep);

  while( iter != end)
  {
    Point3D currentPointIn2D = (*iter)->Coordinates;

    Point3D worldPointIn3D;
    worldPointIn3D.Fill(0.0);
    sliceGeometry->IndexToWorld( currentPointIn2D, worldPointIn3D );
    //MITK_INFO << "index " << currentPointIn2D << " world " << worldPointIn3D << std::endl;

    worldContour->AddVertex( worldPointIn3D, currentTimestep );
    iter++;
  }
  }

  return worldContour;
}



void mitk::ContourUtils::FillContourInSlice( Contour* projectedContour, Image* sliceImage, int paintingPixelValue )
{
  mitk::Contour::PointsContainerIterator it = projectedContour->GetPoints()->Begin();
  mitk::Contour::PointsContainerIterator end = projectedContour->GetPoints()->End();

  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  while(it!=end)
  {
    contour->AddVertex(it.Value());
    it++;
  }
  this->FillContourInSlice(contour, sliceImage, paintingPixelValue);
}


void mitk::ContourUtils::FillContourInSlice( ContourModel* projectedContour, Image* sliceImage, int paintingPixelValue )
{
  this->FillContourInSlice(projectedContour, 0, sliceImage, paintingPixelValue);
}


void mitk::ContourUtils::FillContourInSlice( ContourModel* projectedContour, unsigned int timeStep, Image* sliceImage, int paintingPixelValue )
{
      //create a surface of the input ContourModel
      mitk::Surface::Pointer surface = mitk::Surface::New();
      mitk::ContourModelToSurfaceFilter::Pointer contourModelFilter = mitk::ContourModelToSurfaceFilter::New();
      contourModelFilter->SetInput(projectedContour);
      contourModelFilter->Update();
      surface = contourModelFilter->GetOutput();
      contourModelFilter->Update();

      // that's our vtkPolyData-Surface
      vtkSmartPointer<vtkPolyData> surface2D = vtkSmartPointer<vtkPolyData>::New();

      surface2D->SetPoints(surface->GetVtkPolyData()->GetPoints());
      surface2D->SetLines(surface->GetVtkPolyData()->GetLines());
      surface2D->Modified();
      surface2D->Update();

      //write the surface to disc
      vtkSmartPointer<vtkXMLPolyDataWriter> polyDataWriter =
          vtkSmartPointer<vtkXMLPolyDataWriter>::New();

      polyDataWriter->SetInput(surface2D);
      polyDataWriter->SetFileName("/mes/TestingMethodeOutput/surface2D.vtp");
      polyDataWriter->SetCompressorTypeToNone();
      polyDataWriter->SetDataModeToAscii();
      polyDataWriter->Write();

      // prepare the binary image's voxel grid
      vtkSmartPointer<vtkImageData> whiteImage =
          vtkSmartPointer<vtkImageData>::New();

     //whiteImage = sliceImage->GetVtkImageData();

      //create a new image with the size of the surface
      double bounds[6];
      sliceImage->GetVtkImageData()->GetBounds(bounds);
      MITK_INFO << "Bounds: " << bounds[0] << " - " << bounds[1] << ", " << bounds[2] << " - " << bounds[3] << ", " << bounds[4] << " - " << bounds[5];
      double spacing[3];
      spacing[0] = sliceImage->GetGeometry()->GetSpacing()[0];
      spacing[1] = sliceImage->GetGeometry()->GetSpacing()[1];
      spacing[2] = sliceImage->GetGeometry()->GetSpacing()[2];
      whiteImage->SetSpacing(spacing);

      // compute dimensions
        int dim[3];
        for (int i = 0; i < 3; i++)
          {
          dim[i] = static_cast<int>(ceil((bounds[i * 2 + 1] - bounds[i * 2]) /
              spacing[i])) + 1;
          if (dim[i] < 1)
            dim[i] = 1;
          }
        whiteImage->SetDimensions(dim);
        whiteImage->SetExtent(0, dim[0] - 1, 0, dim[1] - 1, 0, dim[2] - 1);
        double origin[3];
        origin[0] = sliceImage->GetGeometry()->GetOrigin()[0];
        origin[1] = sliceImage->GetGeometry()->GetOrigin()[1];
        origin[2] = sliceImage->GetGeometry()->GetOrigin()[2];
        whiteImage->SetOrigin(origin);

      //define the ImageType and allocate it
      whiteImage->SetScalarTypeToUnsignedChar();
      whiteImage->AllocateScalars();

      // fill the image with foreground voxels:
      unsigned char inval = 255;
      unsigned char outval = 0;
      vtkIdType count = whiteImage->GetNumberOfPoints();
      for (vtkIdType i = 0; i < count; ++i)
        {
        whiteImage->GetPointData()->GetScalars()->SetTuple1(i, inval);
        }

      MITK_INFO << "ContourUtils-WhiteImage/origin: " << whiteImage->GetOrigin()[0] << ", " << whiteImage->GetOrigin()[1] << ", " << whiteImage->GetOrigin()[2];
      MITK_INFO << "ContourUtils-WhiteImage/spacing: " << whiteImage->GetSpacing()[0] << ", " << whiteImage->GetSpacing()[1] << ", " << whiteImage->GetSpacing()[2];
      MITK_INFO << "ContourUtils-WhiteImage/Dimension: " << whiteImage->GetDimensions()[0] << ", " << whiteImage->GetDimensions()[1] << ", " << whiteImage->GetDimensions()[2];

      // polygonal data --> image stencil:
      vtkSmartPointer<vtkPolyDataToImageStencil> pol2stenc =
        vtkSmartPointer<vtkPolyDataToImageStencil>::New();
      //pol2stenc->SetTolerance(0); // important if extruder->SetVector(0, 0, 1) !!!
      pol2stenc->SetInput(surface2D);
      pol2stenc->SetOutputOrigin(whiteImage->GetOrigin());
      pol2stenc->SetOutputSpacing(whiteImage->GetSpacing());
      pol2stenc->SetOutputWholeExtent(whiteImage->GetExtent());
      pol2stenc->Update();

      // cut the corresponding white image and set the background:
      vtkSmartPointer<vtkImageStencil> imgstenc =
        vtkSmartPointer<vtkImageStencil>::New();

      imgstenc->SetInput(whiteImage);
      imgstenc->ReverseStencilOff();
      imgstenc->SetStencil(pol2stenc->GetOutput());
      imgstenc->SetBackgroundValue(0);
      imgstenc->Update();

      mitk::Image::Pointer mitkImage = mitk::Image::New();

      //sliceImage->Initialize(imgstenc->GetOutput());
      sliceImage->SetVolume(imgstenc->GetOutput()->GetScalarPointer());

      //sliceImage = mitkImage;

      vtkSmartPointer<vtkMetaImageWriter> imageWriter =
      vtkSmartPointer<vtkMetaImageWriter>::New();
      imageWriter->SetFileName("/mes/TestingMethodeOutput/BinaryImage.mhd");
      imageWriter->SetInputConnection(imgstenc->GetOutputPort());
      imageWriter->Write();

      mitk::ImageWriter::Pointer mitkWriter = mitk::ImageWriter::New();

      mitkWriter->SetFileName("/mes/TestingMethodeOutput/sliceImage.mhd");
      mitkWriter->SetInput(sliceImage);
      mitkWriter->Write();
      mitkWriter->Update();
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::ContourUtils::ItkCopyFilledContourToSlice( itk::Image<TPixel,VImageDimension>* originalSlice, const Image* filledContourSlice, int overwritevalue )
{
  typedef itk::Image<TPixel,VImageDimension> SliceType;

  typename SliceType::Pointer filledContourSliceITK;
  CastToItkImage( filledContourSlice, filledContourSliceITK );

  // now the original slice and the ipSegmentation-painted slice are in the same format, and we can just copy all pixels that are non-zero
  typedef itk::ImageRegionIterator< SliceType >        OutputIteratorType;
  typedef itk::ImageRegionConstIterator< SliceType >   InputIteratorType;

  InputIteratorType inputIterator( filledContourSliceITK, filledContourSliceITK->GetLargestPossibleRegion() );
  OutputIteratorType outputIterator( originalSlice, originalSlice->GetLargestPossibleRegion() );

  outputIterator.GoToBegin();
  inputIterator.GoToBegin();

  while ( !outputIterator.IsAtEnd() )
  {
    if ( inputIterator.Get() != 0 )
    {
      outputIterator.Set( overwritevalue );
    }

    ++outputIterator;
    ++inputIterator;
  }
}
