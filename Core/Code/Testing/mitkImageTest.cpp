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

// mitk includes
#include <mitkImage.h>
#include <mitkImageDataItem.h>
#include <mitkImageCast.h>
#include <mitkTestingMacros.h>
#include <mitkImageStatisticsHolder.h>
#include "mitkImageGenerator.h"
#include "mitkImageReadAccessor.h"
#include "mitkException.h"
#include "mitkPixelTypeMultiplex.h"
#include "mitkImagePixelReadAccessor.h"
#include "mitkIOUtil.h"

#include "mitkImageSliceSelector.h"

// itk includes
#include <itkImage.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>

// stl includes
#include <fstream>

// vtk includes
#include <vtkImageData.h>

// Checks if reference count is correct after using GetVtkImageData()
bool ImageVtkDataReferenceCheck(const char* fname) {

  const std::string filename = std::string(fname);
  try
  {
    mitk::Image::Pointer image = mitk::IOUtil::LoadImage(filename);
    MITK_TEST_CONDITION_REQUIRED(image.IsNotNull(), "Non-NULL image")

    vtkImageData* vtk = image->GetVtkImageData();

    if(vtk == NULL)
      return false;
  }
  catch(...)
  {
    MITK_TEST_FAILED_MSG(<< "Could not read file for testing: " << filename);
      return false;
  }

  return true;
}

template <class T>
void TestRandomPixelAccess( const mitk::PixelType /*ptype*/, mitk::Image::Pointer image, mitk::Point3D & point, mitk::ScalarType & value )
{
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

  itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer randomGenerator = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();
  randomGenerator->Initialize( std::rand() );      // initialize with random value, to get sensible random points for the image
  point[0] = randomGenerator->GetUniformVariate( image->GetGeometry()->GetOrigin()[0], xMax[0]);
  point[1] = randomGenerator->GetUniformVariate( image->GetGeometry()->GetOrigin()[1], yMax[1]);
  point[2] = randomGenerator->GetUniformVariate( image->GetGeometry()->GetOrigin()[2], zMax[2]);
  MITK_INFO << "RandomPoint " << point[0] << " "<< point[1] << " "<< point[2] << "";

  // test values and max/min
  mitk::ScalarType imageMin = image->GetStatistics()->GetScalarValueMin();
  mitk::ScalarType imageMax = image->GetStatistics()->GetScalarValueMax();

  // test accessing PixelValue with coordinate leading to a negative index
  const mitk::Point3D geom_origin = image->GetGeometry()->GetOrigin();
  const mitk::Point3D geom_center = image->GetGeometry()->GetCenter();

  // shift position from origin outside of the image ( in the opposite direction to [center-origin] vector which points in the inside)
  mitk::Point3D position = geom_origin + (geom_origin - geom_center);

  MITK_INFO << "Testing access outside of the image";
  unsigned int dim = image->GetDimension();
  if(dim == 3 || dim == 4){
    mitk::ImagePixelReadAccessor<T,3> imAccess3(image,image->GetVolumeData(0));

    // Comparison ?>=0 not needed since all position[i] and timestep are unsigned int
    // (position[0]>=0 && position[1] >=0 && position[2]>=0 && timestep>=0)
    // bug-11978 : we still need to catch index with negative values
    if ( point[0] < 0 ||
         point[1] < 0 ||
         point[2] < 0 )
    {
      MITK_WARN << "Given position ("<< point << ") is out of image range, returning 0." ;
    }
    else {
      value = static_cast<mitk::ScalarType>(imAccess3.GetPixelByWorldCoordinates(point));
      MITK_TEST_CONDITION( (value >= imageMin && value <= imageMax), "Value returned is between max/min");
    }
    itk::Index<3> itkIndex;
    image->GetGeometry()->WorldToIndex(position, itkIndex);
    MITK_TEST_FOR_EXCEPTION_BEGIN(mitk::Exception);
    imAccess3.GetPixelByIndexSafe(itkIndex);
    MITK_TEST_FOR_EXCEPTION_END(mitk::Exception);
  }
  MITK_INFO << imageMin << " "<< imageMax << " "<< value << "";

}

class mitkImageTestClass
{
public:
  void SetClonedGeometry_None_ClonedEqualInput()
  {
    mitk::Image::Pointer image = mitk::ImageGenerator::GenerateRandomImage<float>(100, 100, 100, 1, 0.2, 0.3, 0.4);

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
    planegeometry->ChangeImageGeometryConsideringOriginOffset(true);

    image->SetClonedGeometry(planegeometry);

    mitk::BaseGeometry::Pointer imageGeometry = image->GetGeometry();

    bool matrixEqual = mitk::Equal(imageGeometry, planegeometry, mitk::eps, false);

    MITK_TEST_CONDITION(matrixEqual, "Matrix elements of cloned matrix equal original matrix");

  }
};


int mitkImageTest(int argc, char* argv[])
{

  MITK_TEST_BEGIN(mitkImageTest);

  mitkImageTestClass tester;
  tester.SetClonedGeometry_None_ClonedEqualInput();


  //Create Image out of nowhere
  mitk::Image::Pointer imgMem = mitk::Image::New();
  mitk::PixelType pt = mitk::MakeScalarPixelType<int>();
  unsigned int dim[]={100,100,20};

  MITK_TEST_CONDITION_REQUIRED( imgMem.IsNotNull(), "An image was created. ");

  // Initialize image
  imgMem->Initialize( pt, 3, dim);

  MITK_TEST_CONDITION_REQUIRED( imgMem->IsInitialized(), "Image::IsInitialized() ?");
  MITK_TEST_CONDITION_REQUIRED( imgMem->GetPixelType() == pt, "PixelType was set correctly.");


  int *p = NULL;
  int *p2 = NULL;
  try
  {
    mitk::ImageReadAccessor imgMemAcc(imgMem);
    p = (int*)imgMemAcc.GetData();
  }
  catch (mitk::Exception& e)
  {
    MITK_ERROR << e.what();
  }
  MITK_TEST_CONDITION( p != NULL, "GetData() returned not-NULL pointer.");

  // filling image
  const unsigned int size = dim[0]*dim[1]*dim[2];
  for(unsigned int i=0; i<size; ++i, ++p)
    *p = (signed int)i;

  // Getting it again and compare with filled values:
  try
  {
    mitk::ImageReadAccessor imgMemAcc(imgMem);
    p2 = (int*)imgMemAcc.GetData();
  }
  catch (mitk::Exception &e)
  {
    MITK_ERROR << e.what();
  }
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

  try
  {
    mitk::ImageReadAccessor imgMemAcc(imgMem, imgMem->GetSliceData(dim[2]/2));
    p2 = (int*)imgMemAcc.GetData();
  }
  catch (mitk::Exception& e)
  {
    MITK_ERROR << e.what();
  }
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
  try
  {
    mitk::ImageReadAccessor imgMemAcc(imgMem);
    imgMem->SetVolume(imgMemAcc.GetData());
  }
  catch (mitk::Exception& e)
  {
    MITK_ERROR << e.what();
  }

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
  MITK_TEST_CONDITION_REQUIRED( imgMem->GetGeometry()->GetOrigin() == static_cast<mitk::BaseGeometry*>(planegeometry)->GetOrigin(), "Testing correct setting of geometry via initialize!");

  try
  {
    mitk::ImageReadAccessor imgMemAcc(imgMem);
    p = (int*)imgMemAcc.GetData();
  }
  catch (mitk::Exception& e)
  {
    MITK_ERROR << e.what();
  }
  MITK_TEST_CONDITION_REQUIRED( p!=NULL, "GetData() returned valid pointer.");

  // Testing Initialize(const mitk::PixelType& type, int sDim, const mitk::PlaneGeometry& geometry) and GetData(): ";
  imgMem->Initialize( mitk::MakePixelType<int, int, 1>() , 40, *planegeometry);

  try
  {
    mitk::ImageReadAccessor imgMemAcc(imgMem);
    p = (int*)imgMemAcc.GetData();
  }
  catch (mitk::Exception& e)
  {
    MITK_ERROR << e.what();
  }
  MITK_TEST_CONDITION_REQUIRED( p != NULL, "GetData() returned valid pointer.");

  //-----------------
  // testing origin information and methods
  MITK_TEST_CONDITION_REQUIRED(  mitk::Equal(imgMem->GetGeometry()->GetOrigin(), origin), "Testing correctness of origin via GetGeometry()->GetOrigin(): ");

  // Setting origin via SetOrigin(origin): ";
  mitk::FillVector3D(origin, 37.0, 17.92, 27.83);  imgMem->SetOrigin(origin);

  // Test origin
  MITK_TEST_CONDITION_REQUIRED(  mitk::Equal(imgMem->GetGeometry()->GetOrigin(), origin), "Testing correctness of changed origin via GetGeometry()->GetOrigin(): ");
  MITK_TEST_CONDITION_REQUIRED(  mitk::Equal(imgMem->GetSlicedGeometry()->GetPlaneGeometry(0)->GetOrigin(), origin),  "Testing correctness of changed origin via GetSlicedGeometry()->GetPlaneGeometry(0)->GetOrigin(): ");

  //-----------------
  // testing spacing information and methodsunsigned int dim[]={100,100,20};
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(imgMem->GetGeometry()->GetSpacing(), spacing), "Testing correct spacing from Geometry3D!");

  mitk::FillVector3D(spacing, 7.0, 0.92, 1.83);
  imgMem->SetSpacing(spacing);
  MITK_TEST_CONDITION_REQUIRED(  mitk::Equal(imgMem->GetGeometry()->GetSpacing(), spacing), "Testing correctness of changed spacing via GetGeometry()->GetSpacing(): ");
  MITK_TEST_CONDITION_REQUIRED(  mitk::Equal(imgMem->GetSlicedGeometry()->GetPlaneGeometry(0)->GetSpacing(), spacing), "Testing correctness of changed spacing via GetSlicedGeometry()->GetPlaneGeometry(0)->GetSpacing(): ");

  mitk::Image::Pointer vecImg = mitk::Image::New();
  try
  {
    mitk::ImageReadAccessor imgMemAcc(imgMem);
    vecImg->Initialize( imgMem->GetPixelType(), *imgMem->GetGeometry(), 2 /* #channels */, 0 /*tDim*/ );
    vecImg->SetImportChannel(const_cast<void*>(imgMemAcc.GetData()), 0, mitk::Image::CopyMemory );
    vecImg->SetImportChannel(const_cast<void*>(imgMemAcc.GetData()), 1, mitk::Image::CopyMemory );

    mitk::ImageReadAccessor vecImgAcc(vecImg);
    mitk::ImageReadAccessor vecImgAcc0(vecImg, vecImg->GetChannelData(0));
    mitk::ImageReadAccessor vecImgAcc1(vecImg, vecImg->GetChannelData(1));

    MITK_TEST_CONDITION_REQUIRED(vecImgAcc0.GetData() != NULL && vecImgAcc1.GetData() != NULL, "Testing set and return of channel data!");

    MITK_TEST_CONDITION_REQUIRED( vecImg->IsValidSlice(0,0,1) , "");
    MITK_TEST_OUTPUT(<< " Testing whether CopyMemory worked");
    MITK_TEST_CONDITION_REQUIRED(imgMemAcc.GetData() != vecImgAcc.GetData(), "");
    MITK_TEST_OUTPUT(<< " Testing destruction after SetImportChannel");
    vecImg = NULL;
    MITK_TEST_CONDITION_REQUIRED(vecImg.IsNull() , "testing destruction!");
  }
  catch (mitk::Exception& e)
  {
    MITK_ERROR << e.what();
  }
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
  vtkimage->AllocateScalars(VTK_SHORT,1);
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

  // TODO test the following initializers on channel-incorporation
  //  void mitk::Image::Initialize(const mitk::PixelType& type, unsigned int dimension, unsigned int *dimensions, unsigned int channels)
  //  void mitk::Image::Initialize(const mitk::PixelType& type, int sDim, const mitk::PlaneGeometry& geometry2d, bool flipped, unsigned int channels, int tDim )
  //  void mitk::Image::Initialize(const mitk::Image* image)
  //  void mitk::Image::Initialize(const mitkIpPicDescriptor* pic, int channels, int tDim, int sDim)

  //mitk::Image::Pointer vecImg = mitk::Image::New();
  //vecImg->Initialize(PixelType(typeid(float), 6, itk::ImageIOBase::SYMMETRICSECONDRANKTENSOR), *imgMem->GetGeometry(), 2 /* #channels */, 0 /*tDim*/, false /*shiftBoundingBoxMinimumToZero*/ );
  //vecImg->Initialize(PixelType(typeid(itk::Vector<float,6>)), *imgMem->GetGeometry(), 2 /* #channels */, 0 /*tDim*/, false /*shiftBoundingBoxMinimumToZero*/ );

  // testing access by index coordinates and by world coordinates

  MITK_TEST_CONDITION_REQUIRED(argc == 2, "Check if test image is accessible!");
  const std::string filename = std::string(argv[1]);
  mitk::Image::Pointer image;
  try
  {
    image = mitk::IOUtil::LoadImage(filename);
    MITK_TEST_CONDITION_REQUIRED(image.IsNotNull(), "Non-NULL image")
  }
  catch(...) {
    MITK_TEST_FAILED_MSG(<< "Could not read file for testing: " << filename);
    return 0;
  }

  mitk::Point3D point;
  mitk::ScalarType value = -1.;

  mitkPixelTypeMultiplex3(TestRandomPixelAccess,image->GetImageDescriptor()->GetChannelTypeById(0),image,point,value)

  {
    // testing the clone method of mitk::Image
    mitk::Image::Pointer cloneImage = image->Clone();
  MITK_TEST_CONDITION_REQUIRED(cloneImage->GetDimension() == image->GetDimension(), "Clone (testing dimension)");
  MITK_TEST_CONDITION_REQUIRED(cloneImage->GetPixelType() == image->GetPixelType(), "Clone (testing pixel type)");
  // After cloning an image the geometry of both images should be equal too
  MITK_TEST_CONDITION_REQUIRED(cloneImage->GetGeometry()->GetOrigin() == image->GetGeometry()->GetOrigin(), "Clone (testing origin)");
  MITK_TEST_CONDITION_REQUIRED(cloneImage->GetGeometry()->GetSpacing() == image->GetGeometry()->GetSpacing(), "Clone (testing spacing)");
  MITK_TEST_CONDITION_REQUIRED(mitk::MatrixEqualElementWise(cloneImage->GetGeometry()->GetIndexToWorldTransform()->GetMatrix(), image->GetGeometry()->GetIndexToWorldTransform()->GetMatrix()),
    "Clone (testing transformation matrix)");
  MITK_TEST_CONDITION_REQUIRED(mitk::MatrixEqualElementWise(cloneImage->GetTimeGeometry()->GetGeometryForTimeStep(cloneImage->GetDimension(3)-1)->GetIndexToWorldTransform()->GetMatrix(),
    cloneImage->GetTimeGeometry()->GetGeometryForTimeStep(image->GetDimension(3)-1)->GetIndexToWorldTransform()->GetMatrix()), "Clone(testing time sliced geometry)");

  for (unsigned int i = 0u; i < cloneImage->GetDimension(); ++i)
  {
    MITK_TEST_CONDITION_REQUIRED(cloneImage->GetDimension(i) == image->GetDimension(i), "Clone (testing dimension " << i << ")");
  }
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
    typedef itk::Image<double,3> ItkFloatImage3D;
    ItkFloatImage3D::Pointer itkimage;
    try
    {
    mitk::CastToItkImage(image, itkimage);
    MITK_TEST_CONDITION_REQUIRED(itkimage.IsNotNull(), "Test conversion to itk::Image!");
    }
    catch (std::exception& e)
    {
      MITK_INFO << e.what();
    }
    mitk::Point3D itkPhysicalPoint;
    image->GetGeometry()->WorldToItkPhysicalPoint(point, itkPhysicalPoint);
    MITK_INFO << "ITKPoint " << itkPhysicalPoint[0] << " "<< itkPhysicalPoint[1] << " "<< itkPhysicalPoint[2] << "";

    mitk::Point3D backTransformedPoint;
    image->GetGeometry()->ItkPhysicalPointToWorld(itkPhysicalPoint, backTransformedPoint);

    MITK_TEST_CONDITION_REQUIRED( mitk::Equal(point,backTransformedPoint), "Testing world->itk-physical->world consistency");

    itk::Index<3> idx;
    bool status = itkimage->TransformPhysicalPointToIndex(itkPhysicalPoint, idx);

    MITK_INFO << "ITK Index " << idx[0] << " "<< idx[1] << " "<< idx[2] << "";

    if(status && value != -1.)
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

  // clone generated 3D image with one slice in z direction (cf. bug 11058)
  unsigned int* threeDdim = new unsigned int[3];
  threeDdim[0] = 100;
  threeDdim[1] = 200;
  threeDdim[2] = 1;
  mitk::Image::Pointer threeDImage = mitk::Image::New();
  threeDImage->Initialize(mitk::MakeScalarPixelType<float>(), 3, threeDdim);
  mitk::Image::Pointer cloneThreeDImage = threeDImage->Clone();
  // check that the clone image has the same dimensionality as the source image
  MITK_TEST_CONDITION_REQUIRED( cloneThreeDImage->GetDimension() == 3, "Testing if the clone image initializes with 3D!");

  MITK_TEST_CONDITION_REQUIRED( ImageVtkDataReferenceCheck(argv[1]), "Checking reference count of Image after using GetVtkImageData()");

  MITK_TEST_END();
}
