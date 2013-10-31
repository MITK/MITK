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

      /*vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
      triangleFilter->SetInput(surface->GetVtkPolyData());
      triangleFilter->Update();
      */
      //create triangle strips and/or poly-lines from an input polygon
      /*vtkSmartPointer<vtkStripper> stripper = vtkSmartPointer<vtkStripper>::New();
      stripper->SetInput(surface->GetVtkPolyData()); // valid surface
      stripper->Update();
      */

      // that's our vtkPolyData-Surface
      vtkSmartPointer<vtkPolyData> surface2D = vtkSmartPointer<vtkPolyData>::New();
      //vtkIdType count = surface->GetVtkPolyData()->GetNumberOfPoints();
      //vtkCellArray lines();
      //lines.InsertNextCell(count+1);
      //for(int i = 0; i < count; ++i)
        //lines.InsertCellPoint(i);
      //lines.InsertCellPoint(0);
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

      //create a new image with the size of the surface
      double bounds[6];
      surface2D->GetBounds(bounds);
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
        origin[0] = bounds[0];
        origin[1] = bounds[2];
        origin[2] = bounds[4];
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

      //show the pixels of the empty Image / for testing
      for (vtkIdType i = 0; i < count; ++i)
      {
          //MITK_INFO << "Pixel an der Stelle " << i << " hat den Wert: " << whiteImage->GetPointData()->GetScalars()->GetTuple(i)[0];
      }

      MITK_INFO << "ContourUtils-WhiteImage/origin: " << whiteImage->GetOrigin()[0] << ", " << whiteImage->GetOrigin()[1] << ", " << whiteImage->GetOrigin()[2];
      MITK_INFO << "ContourUtils-WhiteImage/spacing: " << whiteImage->GetSpacing()[0] << ", " << whiteImage->GetSpacing()[1] << ", " << whiteImage->GetSpacing()[2];
      MITK_INFO << "ContourUtils-WhiteImage/Dimension: " << whiteImage->GetDimensions()[0] << ", " << whiteImage->GetDimensions()[1] << ", " << whiteImage->GetDimensions()[2];

      /*vtkCellArray array = surface2D->GetNumberOfCells();
      for(int i = 0; i < array; ++i)
      {

      }
      */
      // sweep polygonal data (this is the important thing with contours!)
      vtkSmartPointer<vtkLinearExtrusionFilter> extruder =
          vtkSmartPointer<vtkLinearExtrusionFilter>::New();

      extruder->SetInput(surface2D);
      extruder->SetScaleFactor(1.0);
      extruder->SetExtrusionTypeToNormalExtrusion();
      extruder->SetVector(0, 0, 1);
      extruder->Update();

      polyDataWriter->SetInput(extruder->GetOutput());
      polyDataWriter->SetFileName("/mes/TestingMethodeOutput/extruderGetOutput.vtp");
      polyDataWriter->SetCompressorTypeToNone();
      polyDataWriter->SetDataModeToAscii();
      polyDataWriter->Write();

      // polygonal data --> image stencil:
      vtkSmartPointer<vtkPolyDataToImageStencil> pol2stenc =
        vtkSmartPointer<vtkPolyDataToImageStencil>::New();
      pol2stenc->SetTolerance(0); // important if extruder->SetVector(0, 0, 1) !!!
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

      vtkSmartPointer<vtkMetaImageWriter> imageWriter =
      vtkSmartPointer<vtkMetaImageWriter>::New();
      imageWriter->SetFileName("/mes/TestingMethodeOutput/BinaryImage.mhd");
      imageWriter->SetInputConnection(imgstenc->GetOutputPort());
      imageWriter->Write();

      /*vtkSmartPointer<vtkXMLImageDataWriter> imageWriter =
        vtkSmartPointer<vtkXMLImageDataWriter>::New();
      imageWriter->SetFileName("/mes/TestingMethodeOutput/labelImage.mhd");
      imageWriter->SetInput(imgstenc->GetOutput());
      imageWriter->Write();
      imageWriter->Update();
      */
    // 3D source sphere
//      vtkSmartPointer<vtkSphereSource> sphereSource =
//        vtkSmartPointer<vtkSphereSource>::New();
//      sphereSource->SetPhiResolution(30);
//      sphereSource->SetThetaResolution(30);
//      sphereSource->SetCenter(40, 40, 0);
//      sphereSource->SetRadius(20);
//      // generate circle by cutting the sphere with an implicit plane
//      // (through its center, axis-aligned)
//      vtkSmartPointer<vtkCutter> circleCutter = vtkSmartPointer<vtkCutter>::New();
//      circleCutter->SetInputConnection(sphereSource->GetOutputPort());
//      vtkSmartPointer<vtkPlane> cutPlane = vtkSmartPointer<vtkPlane>::New();
//      cutPlane->SetOrigin(sphereSource->GetCenter());
//      cutPlane->SetNormal(0, 0, 1);
//      circleCutter->SetCutFunction(cutPlane);
//      vtkSmartPointer<vtkStripper> stripper = vtkSmartPointer<vtkStripper>::New();
//      MITK_INFO << "ContourUtilsEXAMPLE-circleCutter-VtkPolyData/Extent: " << circleCutter->GetOutput()->GetWholeExtent()[0] << " - " << circleCutter->GetOutput()->GetBounds()[1] << ", " << circleCutter->GetOutput()->GetBounds()[2] << " - " << circleCutter->GetOutput()->GetBounds()[3] << ", " << circleCutter->GetOutput()->GetBounds()[4] << " - " << circleCutter->GetOutput()->GetBounds()[5];
//      //circleCutter->GetOutput generate a vtkPolyData
//      stripper->SetInputConnection(circleCutter->GetOutputPort()); // valid circle
//      stripper->Update();
//      // that's our circle
//      vtkSmartPointer<vtkPolyData> circle = stripper->GetOutput();

//      // write circle out
//      vtkSmartPointer<vtkXMLPolyDataWriter> polyDataWriter =
//          vtkSmartPointer<vtkXMLPolyDataWriter>::New();

//      polyDataWriter->SetInput(sphereSource->GetOutput());
//      polyDataWriter->SetFileName("/mes/TestingMethodeOutput/EXAMPLEsphereSource-GetOutput.vtp");
//      polyDataWriter->SetCompressorTypeToNone();
//      polyDataWriter->SetDataModeToAscii();
//      polyDataWriter->Write();

//      polyDataWriter->SetInput(circleCutter->GetOutput());
//      polyDataWriter->SetFileName("/mes/TestingMethodeOutput/EXAMPLEcutter-GetOutput.vtp");
//      polyDataWriter->SetCompressorTypeToNone();
//      polyDataWriter->SetDataModeToAscii();
//      polyDataWriter->Write();

//      polyDataWriter->SetInput(circle);
//      polyDataWriter->SetFileName("/mes/TestingMethodeOutput/EXAMPLEstripper-GetOutput.vtp");
//      polyDataWriter->SetCompressorTypeToNone();
//      polyDataWriter->SetDataModeToAscii();
//      polyDataWriter->Write();

//      // prepare the binary image's voxel grid
//      vtkSmartPointer<vtkImageData> whiteImage =
//          vtkSmartPointer<vtkImageData>::New();
//      double bounds[6];
//      circle->GetBounds(bounds);
//      double spacing[3]; // desired volume spacing
//      spacing[0] = 0.5;
//      spacing[1] = 0.5;
//      spacing[2] = 0.5;
//      whiteImage->SetSpacing(spacing);

//      // compute dimensions
//      int dim[3];
//      for (int i = 0; i < 3; i++)
//        {
//        dim[i] = static_cast<int>(ceil((bounds[i * 2 + 1] - bounds[i * 2]) /
//            spacing[i])) + 1;
//        if (dim[i] < 1)
//          dim[i] = 1;
//        }
//      whiteImage->SetDimensions(dim);
//      whiteImage->SetExtent(0, dim[0] - 1, 0, dim[1] - 1, 0, dim[2] - 1);
//      double origin[3];
//      // NOTE: I am not sure whether or not we had to add some offset!
//      origin[0] = bounds[0];// + spacing[0] / 2;
//      origin[1] = bounds[2];// + spacing[1] / 2;
//      origin[2] = bounds[4];// + spacing[2] / 2;
//      whiteImage->SetOrigin(origin);
//    #if VTK_MAJOR_VERSION <= 5
//      whiteImage->SetScalarTypeToUnsignedChar();
//      whiteImage->AllocateScalars();
//    #else
//      whiteImage->AllocateScalars(VTK_UNSIGNED_CHAR,1);
//    #endif
//      // fill the image with foreground voxels:
//      unsigned char inval = 255;
//      unsigned char outval = 0;
//      vtkIdType count = whiteImage->GetNumberOfPoints();
//      for (vtkIdType i = 0; i < count; ++i)
//        {
//        whiteImage->GetPointData()->GetScalars()->SetTuple1(i, inval);
//        }

//      // sweep polygonal data (this is the important thing with contours!)
//      vtkSmartPointer<vtkLinearExtrusionFilter> extruder =
//          vtkSmartPointer<vtkLinearExtrusionFilter>::New();
//    #if VTK_MAJOR_VERSION <= 5
//      extruder->SetInput(circle);
//    #else
//      extruder->SetInputData(circle);
//    #endif
//      extruder->SetScaleFactor(1.);
//      extruder->SetExtrusionTypeToNormalExtrusion();
//      extruder->SetVector(0, 0, 1);
//      extruder->Update();

//      // polygonal data --> image stencil:
//      vtkSmartPointer<vtkPolyDataToImageStencil> pol2stenc =
//        vtkSmartPointer<vtkPolyDataToImageStencil>::New();
//      pol2stenc->SetTolerance(0); // important if extruder->SetVector(0, 0, 1) !!!
//      pol2stenc->SetInputConnection(extruder->GetOutputPort());
//      pol2stenc->SetOutputOrigin(origin);
//      pol2stenc->SetOutputSpacing(spacing);
//      pol2stenc->SetOutputWholeExtent(whiteImage->GetExtent());
//      pol2stenc->Update();

//      // cut the corresponding white image and set the background:
//      vtkSmartPointer<vtkImageStencil> imgstenc =
//        vtkSmartPointer<vtkImageStencil>::New();
//    #if VTK_MAJOR_VERSION <= 5
//      imgstenc->SetInput(whiteImage);
//      imgstenc->SetStencil(pol2stenc->GetOutput());
//    #else
//      imgstenc->SetInputData(whiteImage);
//      imgstenc->SetStencilConnection(pol2stenc->GetOutputPort());
//    #endif
//      imgstenc->ReverseStencilOff();
//      imgstenc->SetBackgroundValue(outval);
//      imgstenc->Update();

//      vtkSmartPointer<vtkMetaImageWriter> imageWriter =
//        vtkSmartPointer<vtkMetaImageWriter>::New();
//      imageWriter->SetFileName("/mes/TestingMethodeOutput/EXAMPLEImage.mhd");
//      imageWriter->SetInputConnection(imgstenc->GetOutputPort());
//      imageWriter->Write();
  /*// 1. Use ipSegmentation to draw a filled(!) contour into a new 8 bit 2D image, which will later be copied back to the slice.
  //    We don't work on the "real" working data, because ipSegmentation would restrict us to 8 bit images

  // convert the projected contour into a ipSegmentation format
  mitkIpInt4_t* picContour = new mitkIpInt4_t[2 * projectedContour->GetNumberOfVertices(timeStep)];
  unsigned int index(0);
  ContourModel::VertexIterator iter = projectedContour->Begin(timeStep);
  ContourModel::VertexIterator end = projectedContour->End(timeStep);

  while( iter != end)
  {
    picContour[ 2 * index + 0 ] = static_cast<mitkIpInt4_t>( (*iter)->Coordinates[0] + 1.0 ); // +0.5 wahrscheinlich richtiger
    picContour[ 2 * index + 1 ] = static_cast<mitkIpInt4_t>( (*iter)->Coordinates[1] + 1.0 );
    //MITK_INFO << "mitk 2d [" << (*iter)[0] << ", " << (*iter)[1] << "]  pic [" << picContour[ 2*index+0] << ", " << picContour[ 2*index+1] << "]";
    iter++;
    index++;
  }

  assert( sliceImage->GetSliceData() );
  mitkIpPicDescriptor* originalPicSlice = mitkIpPicNew();
  CastToIpPicDescriptor( sliceImage, originalPicSlice);
  mitkIpPicDescriptor* picSlice = ipMITKSegmentationNew( originalPicSlice );
  ipMITKSegmentationClear( picSlice );

  assert( originalPicSlice && picSlice );

  // here comes the actual contour filling algorithm (from ipSegmentation/Graphics Gems)
  ipMITKSegmentationCombineRegion ( picSlice, picContour, projectedContour->GetNumberOfVertices(timeStep), NULL, IPSEGMENTATION_OR,  1); // set to 1

  delete[] picContour;

  // 2. Copy the filled contour to the working data slice
  //    copy all pixels that are non-zero to the original image (not caring for the actual type of the working image). perhaps make the replace value a parameter ( -> general painting tool ).
  //    make the pic slice an mitk/itk image (as little ipPic code as possible), call a templated method with accessbyitk, iterate over the original and the modified slice

  Image::Pointer ipsegmentationModifiedSlice = Image::New();
  ipsegmentationModifiedSlice->Initialize( CastToImageDescriptor( picSlice ) );
  ipsegmentationModifiedSlice->SetSlice( picSlice->data );

  AccessFixedDimensionByItk_2( sliceImage, ItkCopyFilledContourToSlice, 2, ipsegmentationModifiedSlice, paintingPixelValue );

  ipsegmentationModifiedSlice = NULL; // free MITK header information
  ipMITKSegmentationFree( picSlice ); // free actual memory



    /*MeshFilterType::Pointer meshFilter = MeshFilterType::New();
    contourModel2itkMesh(projectedContour);
    meshFilter->SetInput(m_mesh);
    meshFilter->Update();
    std::cout << "meshFilter: " << meshFilter->GetOutput();

    ImageType::Pointer filledSegmentationSlice = ImageType::New();
    filledSegmentationSlice = meshFilter->GetOutput();

    //filledSegmentationSlice = NULL;
    */
    /*typedef typename itk::BinaryMask3DMeshSource<  ImageType, MeshType> MeshFilterType;


    // Convert ROI to Mesh

    typename MeshFilterType::Pointer resampler = MeshFilterType::New();

    typename ImageType::Pointer itkReference = ImageType::New();
    typename ImageType::Pointer itkTarget = ImageType::New();
    CastToItkImage(roi,itkTarget);
    CastToItkImage(reference,itkReference);

    resampler->SetObjectValue(1);
    resampler->SetInput(itkTarget);

    try
    {
      resampler->Update();
    }
    catch( itk::ExceptionObject & exp )
    {
      std::cerr << "Exception thrown during Update() " << std::endl;
      std::cerr << exp << std::endl;
    }

    */
    // Create ROI from Mesh with desired dimension, etc.

    /*itk::TriangleMeshToBinaryImageFilter<MeshType, OutputImageType>::Pointer meshFilter = itk::TriangleMeshToBinaryImageFilter<MeshType, OutputImageType>::New();
    MeshType::Pointer mesh = contourModel2itkMesh(projectedContour); //convert the contourModel to a itk::mesh

    //testing mesh
    typedef itk::VTKPolyDataWriter<MeshType> WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetInput(mesh);
    writer->SetFileName("/home/woitzik/TestingMethodeOutput/test.vtk");
    writer->Update();


    itk::Image<unsigned char, 2>::Pointer workingImageSliceInITK = itk::Image<unsigned char, 2>::New();
    OutputImageType::Pointer outputImage = OutputImageType::New();

    //CastToItkImage(sliceImage, workingImageSliceInITK);

    mitk::Image::Pointer sliceImageInMITK = mitk::Image::New();
    sliceImageInMITK->Initialize(sliceImage->GetImageDescriptor());

    mitk::CastToItkImage(sliceImageInMITK, outputImage);


    /*typedef itk::Image<unsigned char, 2> InputImageType;
    typedef itk::Image<unsigned char, 3> OutputIm;
    typedef itk::CastImageFilter<InputImageType, OutputIm> Cast2DTo3DFilterType;
    Cast2DTo3DFilterType::Pointer cast2DTo3DFilter = Cast2DTo3DFilterType::New();
    cast2DTo3DFilter->SetInput(workingImageSliceInITK);
    cast2DTo3DFilter->Update();
    */
    //variables for the filter
    /*const OutputImageType::PointType orgOrigin = workingImageSliceInITK->GetOrigin();
    const OutputImageType::SizeType orgSize = workingImageSliceInITK->GetLargestPossibleRegion().GetSize();
    const OutputImageType::SpacingType orgSpacing = workingImageSliceInITK->GetSpacing();
    const OutputImageType::DirectionType orgDirection = workingImageSliceInITK->GetDirection();

    outputImage->SetRegions(orgSize);
    outputImage->SetSpacing(orgSpacing);
    outputImage->SetOrigin(orgOrigin);
    outputImage->SetDirection(orgDirection);
    outputImage->Allocate();
    */
    /*OutputImageType::PointType orgi;
    orgi.Fill(0.0);
    orgi[0] = orgOrigin[0];
    orgi[1] = orgOrigin[1];
    */

    /*itk::Point<float, 3> orgi;
    orgi.Fill(0.0);
    */
    //roiSampler->SetInfoImage( workingImageSliceInITK );
    /*meshFilter->SetOrigin(orgi);
    meshFilter->SetSpacing(orgSpacing);
    meshFilter->SetSize(orgSize);
    meshFilter->SetIndex(orgIndex);
    meshFilter->SetTolerance(1.0);
    */
    /*typedef unsigned char OutputPixelType;

    const OutputPixelType empty = 0;
    const OutputPixelType fill = 256;

    meshFilter->SetInput(mesh);
    meshFilter->SetInfoImage( outputImage );
    meshFilter->SetTolerance(1.0);
    meshFilter->SetInsideValue(fill);
    meshFilter->SetOutsideValue(empty);

    //meshFilter->SetOrigin(workingImageSliceInITK);
    //write the output to a file
    itk::ImageFileWriter< OutputImageType >::Pointer writer1 = itk::ImageFileWriter< OutputImageType >::New();
    writer1->SetFileName("/home/woitzik/test.png");
    writer1->SetInput(meshFilter->GetOutput());
    try
    {
    meshFilter->Update();
    writer1->Update();
    }
    catch( itk::ExceptionObject &err )
    {
        MITK_INFO << "ExceptionObject caught !";
        MITK_INFO << err;
    }
    //MITK_INFO << "roiSampler->Update();";
    //MITK_INFO << "infoImage: " << roiSampler->GetOutput();

    //mitk::Image::Pointer workingImageSliceInMITK = mitk::Image::New();

    //mitk::CastToMitkImage(meshFilter->GetOutput(), workingImageSliceInMITK);

    //workingImageSliceInITK = meshFilter->GetOutput();
    //workingImageSliceInMITK->GetGeometry()->SetOrigin(sliceImage->GetGeometry()->GetOrigin());
    //workingImageSliceInMITK->GetGeometry()->SetIndexToWorldTransform(sliceImage->GetGeometry()->GetIndexToWorldTransform());

    //MITK_INFO << "MITK Image: " << workingImageSliceInMITK;

    //mitk::IOUtil::SaveImage(workingImageSliceInMITK, "/home/woitzik/TestingMethodeOutput/workingImage.nrrd");
    */
}

/*itk::Mesh< float , 2 >::Pointer mitk::ContourUtils::contourModel2itkMesh(ContourModel* contourModel)
{
    mitk::ContourModel::VertexIterator it = contourModel->Begin();
    mitk::ContourModel::VertexIterator itEnd = contourModel->End();

    MeshType::Pointer mesh = MeshType::New();
    MeshType::PointType point;

    mitk::Point2D center;
    center[0] = 0;
    center[1] = 0;

    int i;

    for( i=0 ; it!=itEnd ; i++ )
    {
        mitk::Point3D contourVertex = (*it)->Coordinates;
        point[0] = contourVertex[0];
        point[1] = contourVertex[1];
        MITK_INFO << point;
        mesh->SetPoint( i , point );
        it++;
        center[0] = center[0] + point[0];
        center[1] = center[1] + point[1];
    }

    center[0] = center[0]/i;
    center[1] = center[1]/i;
    MITK_INFO << "center" << center;

    mesh->SetPoint( i, center);

    int centerPointId = i;
    TriangleCellType::CellAutoPointer trianglepointer;

    for( int j = 1; j!=mesh->GetNumberOfPoints(); j++)
    {
        trianglepointer.TakeOwnership( new TriangleCellType );

    //every point and his next neighbour are combined with the center point to a triangle cell of the mesh
    if(j!=(mesh->GetNumberOfPoints()-1))
    {
    trianglepointer->SetPointId( 0, j-1 );
    trianglepointer->SetPointId( 1, j );
    trianglepointer->SetPointId( 2, centerPointId );
    mesh->SetCell( j-1, trianglepointer );
    }
    //for the last triangle cell between the first point and the last point
    else
    {
    trianglepointer->SetPointId( 0, 0 );
    trianglepointer->SetPointId( 1, j-1 );
    trianglepointer->SetPointId( 2, centerPointId );
    mesh->SetCell( j-1, trianglepointer );
    }
    }

    MITK_INFO << "Points: " << mesh->GetNumberOfPoints();
    MITK_INFO << "Cells: " << mesh->GetNumberOfCells();

   return mesh;
}*/

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
