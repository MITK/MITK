/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

// mitk includes
#include <mitkImage.h>
#include <mitkImageDataItem.h>
#include <mitkImageCast.h>
#include "mitkItkImageFileReader.h"
#include <mitkTestingMacros.h>
#include <mitkImageStatisticsHolder.h>

// itk includes
#include <itkImage.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>

// stl includes
#include <fstream>

// vtk includes
#include <vtkImageData.h>

int mitkImageTest(int argc, char* argv[])
{

  MITK_TEST_BEGIN(mitkImageTest);

  //Create Image out of nowhere
  mitk::Image::Pointer imgMem = mitk::Image::New();
  mitk::PixelType pt = mitk::MakeScalarPixelType<int>();
  unsigned int dim[]={100,100,20};

  MITK_TEST_CONDITION_REQUIRED( imgMem.IsNotNull(), "An image was created. ");

  // Initialize image
  imgMem->Initialize( pt, 3, dim);

  MITK_TEST_CONDITION_REQUIRED( imgMem->IsInitialized(), "Image::IsInitialized() ?");
  MITK_TEST_CONDITION_REQUIRED( imgMem->GetPixelType() == pt, "PixelType was set correctly.");

  int *p = (int*)imgMem->GetData();
  MITK_TEST_CONDITION( p != NULL, "GetData() returned not-NULL pointer.");

  // FIXME: this is directly changing the image data
  // filling image
  const unsigned int size = dim[0]*dim[1]*dim[2];
  for(unsigned int i=0; i<size; ++i, ++p)
    *p= (signed int)i;

  // Getting it again and compare with filled values:
  int *p2 = (int*)imgMem->GetData();
  MITK_TEST_CONDITION( p2 != NULL, "GetData() returned not-NULL pointer.");

  bool isEqual = true;
  for(unsigned int i=0; i<size; ++i, ++p2)
  {
    if(*p2 != (signed int) i )
    {
      isEqual = false;
    }
  }
  MITK_TEST_CONDITION( isEqual, "The values previously set as data are correct [pixelwise comparison].");

  // Testing GetSliceData() and compare with filled values:
  p2 = (int*)imgMem->GetSliceData(dim[2]/2)->GetData();
  MITK_TEST_CONDITION_REQUIRED( p2 != NULL, "Valid slice data returned");

  unsigned int xy_size = dim[0]*dim[1];
  unsigned int start_mid_slice = (dim[2]/2)*xy_size;
  isEqual = true;
  for(unsigned int i=0; i<xy_size; ++i, ++p2)
  {
    if(*p2!=(signed int)(i+start_mid_slice))
    {
      isEqual = false;
    }
  }
  MITK_TEST_CONDITION( isEqual, "The SliceData are correct [pixelwise comparison]. ");

  imgMem = mitk::Image::New();

  // testing re-initialization of test image
  mitk::PixelType pType = mitk::MakePixelType<int, int, 1>();
  imgMem->Initialize( pType , 3, dim);
  MITK_TEST_CONDITION_REQUIRED(imgMem->GetDimension()== 3, "Testing initialization parameter dimension!");
  MITK_TEST_CONDITION_REQUIRED(imgMem->GetPixelType() ==  pType, "Testing initialization parameter pixeltype!");
  MITK_TEST_CONDITION_REQUIRED(imgMem->GetDimension(0) == dim[0] &&
    imgMem->GetDimension(1)== dim[1] && imgMem->GetDimension(2)== dim[2], "Testing initialization of dimensions!");
  MITK_TEST_CONDITION( imgMem->IsInitialized(), "Image is initialized.");

  // Setting volume again:
  imgMem->SetVolume(imgMem->GetData());

  //-----------------
  // geometry information for image
  mitk::Point3D origin;
  mitk::Vector3D right, bottom;
  mitk::Vector3D spacing;
  mitk::FillVector3D(origin, 17.0, 19.92, 7.83);
  mitk::FillVector3D(right, 1.0, 2.0, 3.0);
  mitk::FillVector3D(bottom, 0.0, -3.0, 2.0);
  mitk::FillVector3D(spacing, 0.78, 0.91, 2.23);

  //InitializeStandardPlane(rightVector, downVector, spacing)
  mitk::PlaneGeometry::Pointer planegeometry = mitk::PlaneGeometry::New();
  planegeometry->InitializeStandardPlane(100, 100, right, bottom, &spacing);
  planegeometry->SetOrigin(origin);

  // Testing Initialize(const mitk::PixelType& type, const mitk::Geometry3D& geometry, unsigned int slices) with PlaneGeometry and GetData(): ";
  imgMem->Initialize( mitk::MakePixelType<int, int, 1>(), *planegeometry);
  MITK_TEST_CONDITION_REQUIRED( imgMem->GetGeometry()->GetOrigin() == static_cast<mitk::Geometry3D*>(planegeometry)->GetOrigin(), "Testing correct setting of geometry via initialize!");

  p = (int*)imgMem->GetData();
  MITK_TEST_CONDITION_REQUIRED( p!=NULL, "GetData() returned valid pointer.");

  // Testing Initialize(const mitk::PixelType& type, int sDim, const mitk::PlaneGeometry& geometry) and GetData(): ";
  imgMem->Initialize( mitk::MakePixelType<int, int, 1>() , 40, *planegeometry);

  p = (int*)imgMem->GetData();
  MITK_TEST_CONDITION_REQUIRED( p!=NULL, "GetData() returned valid pointer.");

  //-----------------
  // testing origin information and methods
  MITK_TEST_CONDITION_REQUIRED(  mitk::Equal(imgMem->GetGeometry()->GetOrigin(), origin), "Testing correctness of origin via GetGeometry()->GetOrigin(): ");
  MITK_TEST_CONDITION_REQUIRED(  mitk::Equal(imgMem->GetTimeSlicedGeometry()->GetOrigin(), origin), "Testing correctness of origin via GetTimeSlicedGeometry()->GetOrigin(): ");

  // Setting origin via SetOrigin(origin): ";
  mitk::FillVector3D(origin, 37.0, 17.92, 27.83);
  imgMem->SetOrigin(origin);

  // Test origin
  MITK_TEST_CONDITION_REQUIRED(  mitk::Equal(imgMem->GetGeometry()->GetOrigin(), origin), "Testing correctness of changed origin via GetGeometry()->GetOrigin(): ");
  MITK_TEST_CONDITION_REQUIRED(  mitk::Equal(imgMem->GetTimeSlicedGeometry()->GetOrigin(), origin), "Testing correctness of changed origin via GetTimeSlicedGeometry()->GetOrigin(): ");
  MITK_TEST_CONDITION_REQUIRED(  mitk::Equal(imgMem->GetSlicedGeometry()->GetGeometry2D(0)->GetOrigin(), origin),  "Testing correctness of changed origin via GetSlicedGeometry()->GetGeometry2D(0)->GetOrigin(): ");

  //-----------------
  // testing spacing information and methods
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(imgMem->GetGeometry()->GetSpacing(), spacing), "Testing correct spacing from Geometry3D!");
   MITK_TEST_CONDITION_REQUIRED(mitk::Equal(imgMem->GetTimeSlicedGeometry()->GetSpacing(), spacing), "Testing correctspacing from TimeSlicedGeometry!");

  mitk::FillVector3D(spacing, 7.0, 0.92, 1.83);
  imgMem->SetSpacing(spacing);
  MITK_TEST_CONDITION_REQUIRED(  mitk::Equal(imgMem->GetGeometry()->GetSpacing(), spacing), "Testing correctness of changed spacing via GetGeometry()->GetSpacing(): ");
  MITK_TEST_CONDITION_REQUIRED(  mitk::Equal(imgMem->GetTimeSlicedGeometry()->GetSpacing(), spacing), "Testing correctness of changed spacing via GetTimeSlicedGeometry()->GetSpacing(): ");
  MITK_TEST_CONDITION_REQUIRED(  mitk::Equal(imgMem->GetSlicedGeometry()->GetGeometry2D(0)->GetSpacing(), spacing), "Testing correctness of changed spacing via GetSlicedGeometry()->GetGeometry2D(0)->GetSpacing(): ");

  mitk::Image::Pointer vecImg = mitk::Image::New();
  vecImg->Initialize( imgMem->GetPixelType(), *imgMem->GetGeometry(), 2 /* #channels */, 0 /*tDim*/ );
  vecImg->SetImportChannel(imgMem->GetData(), 0, mitk::Image::CopyMemory );
  vecImg->SetImportChannel(imgMem->GetData(), 1, mitk::Image::CopyMemory );
  MITK_TEST_CONDITION_REQUIRED(vecImg->GetChannelData(0)->GetData() != NULL && vecImg->GetChannelData(1)->GetData() != NULL, "Testing set and return of channel data!");

  MITK_TEST_CONDITION_REQUIRED( vecImg->IsValidSlice(0,0,1) , "");
  MITK_TEST_OUTPUT(<< " Testing whether CopyMemory worked");
  MITK_TEST_CONDITION_REQUIRED(imgMem->GetData() != vecImg->GetData(), "");
  MITK_TEST_OUTPUT(<< " Testing destruction after SetImportChannel");
  vecImg = NULL;
  MITK_TEST_CONDITION_REQUIRED(vecImg.IsNull() , "testing destruction!");

  //-----------------
  MITK_TEST_OUTPUT(<< "Testing initialization via vtkImageData");
  MITK_TEST_OUTPUT(<< " Setting up vtkImageData");
  vtkImageData* vtkimage = vtkImageData::New();
  vtkimage->Initialize();
  vtkimage->SetDimensions( 2, 3, 4);
  double vtkorigin[] =  {-350,-358.203, -1363.5};
  vtkimage->SetOrigin(vtkorigin);
  mitk::Point3D vtkoriginAsMitkPoint;
  mitk::vtk2itk(vtkorigin, vtkoriginAsMitkPoint);
  double vtkspacing[] =  {1.367, 1.367, 2};
  vtkimage->SetSpacing(vtkspacing);
  vtkimage->SetScalarType( VTK_SHORT );
  vtkimage->AllocateScalars();
  std::cout<<"[PASSED]"<<std::endl;
  
  MITK_TEST_OUTPUT(<< " Testing mitk::Image::Initialize(vtkImageData*, ...)");
  mitk::Image::Pointer mitkByVtkImage = mitk::Image::New();
  mitkByVtkImage ->Initialize(vtkimage);
  MITK_TEST_CONDITION_REQUIRED(mitkByVtkImage->IsInitialized(), "");
  vtkimage->Delete();

  MITK_TEST_OUTPUT(<< " Testing whether spacing has been correctly initialized from vtkImageData");
  mitk::Vector3D spacing2 = mitkByVtkImage->GetGeometry()->GetSpacing();
  mitk::Vector3D vtkspacingAsMitkVector;
  mitk::vtk2itk(vtkspacing, vtkspacingAsMitkVector);
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(spacing2,vtkspacingAsMitkVector), "");

  MITK_TEST_OUTPUT(<< " Testing whether GetSlicedGeometry(0)->GetOrigin() has been correctly initialized from vtkImageData");
  mitk::Point3D origin2 = mitkByVtkImage->GetSlicedGeometry(0)->GetOrigin();
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(origin2,vtkoriginAsMitkPoint), "");

  MITK_TEST_OUTPUT(<< " Testing whether GetGeometry()->GetOrigin() has been correctly initialized from vtkImageData");
  origin2 = mitkByVtkImage->GetGeometry()->GetOrigin();
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(origin2,vtkoriginAsMitkPoint), "");

  MITK_TEST_OUTPUT(<< " Testing whether GetTimeSlicedGeometry()->GetOrigin() has been correctly initialized from vtkImageData");
  origin2 = mitkByVtkImage->GetTimeSlicedGeometry()->GetOrigin();
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(origin2,vtkoriginAsMitkPoint), "");

  // TODO test the following initializers on channel-incorporation
  //  void mitk::Image::Initialize(const mitk::PixelType& type, unsigned int dimension, unsigned int *dimensions, unsigned int channels)
  //  void mitk::Image::Initialize(const mitk::PixelType& type, int sDim, const mitk::Geometry2D& geometry2d, bool flipped, unsigned int channels, int tDim )
  //  void mitk::Image::Initialize(const mitk::Image* image) 
  //  void mitk::Image::Initialize(const mitkIpPicDescriptor* pic, int channels, int tDim, int sDim)

  //mitk::Image::Pointer vecImg = mitk::Image::New();
  //vecImg->Initialize(PixelType(typeid(float), 6, itk::ImageIOBase::SYMMETRICSECONDRANKTENSOR), *imgMem->GetGeometry(), 2 /* #channels */, 0 /*tDim*/, false /*shiftBoundingBoxMinimumToZero*/ );
  //vecImg->Initialize(PixelType(typeid(itk::Vector<float,6>)), *imgMem->GetGeometry(), 2 /* #channels */, 0 /*tDim*/, false /*shiftBoundingBoxMinimumToZero*/ );

  // testing access by index coordinates and by world coordinates

  MITK_TEST_CONDITION_REQUIRED(argc == 2, "Check if test image is accessible!"); 
  const std::string filename = std::string(argv[1]);
  mitk::ItkImageFileReader::Pointer imageReader = mitk::ItkImageFileReader::New();
  try
  {
    imageReader->SetFileName(filename);
    imageReader->Update();
  }
  catch(...) {
    MITK_TEST_FAILED_MSG(<< "Could not read file for testing: " << filename);
    return 0;
  }  
  
  mitk::Image::Pointer image = imageReader->GetOutput();
  
  // generate a random point in world coordinates
  mitk::Point3D xMax, yMax, zMax, xMaxIndex, yMaxIndex, zMaxIndex;
  xMaxIndex.Fill(0.0f);
  yMaxIndex.Fill(0.0f);
  zMaxIndex.Fill(0.0f);
  xMaxIndex[0] = image->GetLargestPossibleRegion().GetSize()[0];
  yMaxIndex[1] = image->GetLargestPossibleRegion().GetSize()[1];
  zMaxIndex[2] = image->GetLargestPossibleRegion().GetSize()[2];
  image->GetGeometry()->IndexToWorld(xMaxIndex, xMax);
  image->GetGeometry()->IndexToWorld(yMaxIndex, yMax);
  image->GetGeometry()->IndexToWorld(zMaxIndex, zMax);
  MITK_INFO << "Origin " << image->GetGeometry()->GetOrigin()[0] << " "<< image->GetGeometry()->GetOrigin()[1] << " "<< image->GetGeometry()->GetOrigin()[2] << "";
  MITK_INFO << "MaxExtend " << xMax[0] << " "<< yMax[1] << " "<< zMax[2] << "";
  mitk::Point3D point;

  itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer randomGenerator = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();
  randomGenerator->Initialize( std::rand() );      // initialize with random value, to get sensible random points for the image
  point[0] = randomGenerator->GetUniformVariate( image->GetGeometry()->GetOrigin()[0], xMax[0]);
  point[1] = randomGenerator->GetUniformVariate( image->GetGeometry()->GetOrigin()[1], yMax[1]);
  point[2] = randomGenerator->GetUniformVariate( image->GetGeometry()->GetOrigin()[2], zMax[2]);
  MITK_INFO << "RandomPoint " << point[0] << " "<< point[1] << " "<< point[2] << "";

  // test values and max/min
  mitk::ScalarType imageMin = image->GetStatistics()->GetScalarValueMin();
  mitk::ScalarType imageMax = image->GetStatistics()->GetScalarValueMax();
  mitk::ScalarType value = image->GetPixelValueByWorldCoordinate(point);
  MITK_TEST_CONDITION( (value>=imageMin && value <=imageMax), "Value returned is between max/min");
  MITK_INFO << imageMin << " "<< imageMax << " "<< value << "";

  mitk::Image::Pointer cloneImage = image->Clone();
  MITK_TEST_CONDITION_REQUIRED(cloneImage->GetDimension() == image->GetDimension(), "Clone (testing dimension)");
  MITK_TEST_CONDITION_REQUIRED(cloneImage->GetPixelType() == image->GetPixelType(), "Clone (testing pixel type)");

  for (unsigned int i = 0u; i < cloneImage->GetDimension(); ++i)
  {
    MITK_TEST_CONDITION_REQUIRED(cloneImage->GetDimension(i) == image->GetDimension(i), "Clone (testing dimension " << i << ")");
  }

  //access via itk
  if(image->GetDimension()> 3)    // CastToItk only works with 3d images so we need to check for 4d images
  {
    mitk::ImageTimeSelector::Pointer selector = mitk::ImageTimeSelector::New();
    selector->SetTimeNr(0);
    selector->SetInput(image);
    selector->Update();
    image = selector->GetOutput();
  }

  if(image->GetDimension()==3)
  {
    typedef itk::Image<float,3> ItkFloatImage3D;
    ItkFloatImage3D::Pointer itkimage;
    mitk::CastToItkImage(image, itkimage);
    MITK_TEST_CONDITION_REQUIRED(itkimage.IsNotNull(), "Test conversion to itk::Image!");

    mitk::Point3D itkPhysicalPoint;
    image->GetGeometry()->WorldToItkPhysicalPoint(point, itkPhysicalPoint);
    MITK_INFO << "ITKPoint " << itkPhysicalPoint[0] << " "<< itkPhysicalPoint[1] << " "<< itkPhysicalPoint[2] << "";

    mitk::Point3D backTransformedPoint;
    image->GetGeometry()->ItkPhysicalPointToWorld(itkPhysicalPoint, backTransformedPoint);

    MITK_TEST_CONDITION_REQUIRED( mitk::Equal(point,backTransformedPoint), "Testing world->itk-physical->world consistency");

    itk::Index<3> idx;  
    bool status = itkimage->TransformPhysicalPointToIndex(itkPhysicalPoint, idx);

    MITK_INFO << "ITK Index " << idx[0] << " "<< idx[1] << " "<< idx[2] << "";
   
    if(status)
    {
      float valByItk = itkimage->GetPixel(idx);
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal(valByItk, value), "Compare value of pixel returned by mitk in comparison to itk");
    }
    else
    {
      MITK_WARN<< "Index is out buffered region!";
    }
  }
  else
  {
    MITK_INFO << "Image does not contain three dimensions, some test cases are skipped!";
  } 
  MITK_TEST_END();
}