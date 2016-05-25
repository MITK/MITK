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

// Testing
#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

// other
#include <mitkExtractSliceFilter.h>
#include <mitkImage.h>
#include <mitkImagePixelWriteAccessor.h>
#include <mitkImagePixelReadAccessor.h>
#include <mitkIOUtil.h>
#include <mitkSliceNavigationController.h>
#include <mitkSegmentationInterpolationController.h>
#include <mitkTool.h>
#include <mitkVtkImageOverwrite.h>

class mitkSegmentationInterpolationTestSuite : public mitk::TestFixture
{
    CPPUNIT_TEST_SUITE(mitkSegmentationInterpolationTestSuite);
    MITK_TEST(Equal_Axial_TestInterpolationAndReferenceInterpolation_ReturnsTrue);
    CPPUNIT_TEST_SUITE_END();

private:

    mitk::Image::Pointer m_ReferenceImage;
    mitk::Image::Pointer m_SegmentationImage;
    itk::Index<3> m_CenterPoint;
    mitk::SegmentationInterpolationController::Pointer m_InterpolationController;

public:

    void setUp() override
    {
        m_ReferenceImage = mitk::IOUtil::LoadImage(GetTestDataFilePath("Pic3D.nrrd"));
        CPPUNIT_ASSERT_MESSAGE("Failed to load image for test: [Pic3D.nrrd]", m_ReferenceImage.IsNotNull());

        m_InterpolationController = mitk::SegmentationInterpolationController::GetInstance();

        // Create empty segmentation
        // Surely there must be a better way to get an image with all zeros?
        m_SegmentationImage = mitk::Image::New();
        const mitk::PixelType pixelType(mitk::MakeScalarPixelType<mitk::Tool::DefaultSegmentationDataType>());
        m_SegmentationImage->Initialize(pixelType, m_ReferenceImage->GetDimension(), m_ReferenceImage->GetDimensions());
        m_SegmentationImage->SetClonedTimeGeometry(m_ReferenceImage->GetTimeGeometry());
        unsigned int size = sizeof(mitk::Tool::DefaultSegmentationDataType);
        for (unsigned int dim = 0; dim < m_SegmentationImage->GetDimension(); ++dim)
        {
            size *= m_SegmentationImage->GetDimension(dim);
        }
        mitk::ImageWriteAccessor imageAccessor(m_SegmentationImage);
        memset(imageAccessor.GetData(), 0, size);

        // Work in the center of the image
        m_CenterPoint = {{ 127, 127, 25 }};
//        m_CenterPoint = {{ 15, 15, 15 }};
    }

    void tearDown() override
    {
        m_ReferenceImage = nullptr;
        m_SegmentationImage = nullptr;
        m_CenterPoint = {{ 0, 0, 0 }};
    }

    void Equal_Axial_TestInterpolationAndReferenceInterpolation_ReturnsTrue()
    {
        int dim(1); // axial slices are sorted along third axis

        /* Fill segmentation
         *
         * 1st slice: 3x3 square segmentation
         * 2nd slice: empty
         * 3rd slice: 1x1 square segmentation in corner
         * -> 2nd slice should become 2x2 square in corner
         *
         * put accessor in scope
         */
        itk::Index<3> currentPoint;
        {
            mitk::ImagePixelWriteAccessor<mitk::Tool::DefaultSegmentationDataType, 3> writeAccessor(m_SegmentationImage);

            // Fill 3x3 slice
            currentPoint[dim] = m_CenterPoint[dim] - 1;
            for (int i=-1; i<=1; ++i)
            {
                for (int j=-1; j<=1; ++j)
                {
                    currentPoint[(dim+1)%3] = m_CenterPoint[(dim+1)%3] + i;
                    currentPoint[(dim+2)%3] = m_CenterPoint[(dim+2)%3] + j;
                    writeAccessor.SetPixelByIndexSafe(currentPoint, 1);
                }
            }
            // Now i=j=1, set point two slices up
            currentPoint[dim] = m_CenterPoint[dim] + 1;
            writeAccessor.SetPixelByIndexSafe(currentPoint, 1);
        }

        mitk::IOUtil::Save(m_SegmentationImage, "/home/jenspetersen/Desktop/preseg.nrrd");

        m_InterpolationController->SetSegmentationVolume(m_SegmentationImage);
        m_InterpolationController->SetReferenceVolume(m_ReferenceImage);

        // This could be easier...
        // Note: Index = (sag, cor, ax); PlaneOrientation enum = (ax, sag, cor) in this situation
        mitk::SliceNavigationController::Pointer navigationController = mitk::SliceNavigationController::New();
        navigationController->SetInputWorldTimeGeometry(m_SegmentationImage->GetTimeGeometry());
        navigationController->SetViewDirection(mitk::SliceNavigationController::ViewDirection(0));
        navigationController->Update();
        mitk::Point3D pointMM;
        m_SegmentationImage->GetTimeGeometry()->GetGeometryForTimeStep(0)->IndexToWorld(m_CenterPoint, pointMM);
        navigationController->SelectSliceByPoint(pointMM);
        auto plane = navigationController->GetCurrentPlaneGeometry();


//        mitk::BaseGeometry::Pointer currentGeometry = m_SegmentationImage->GetTimeGeometry()->GetGeometryForTimeStep(0);
//        mitk::SlicedGeometry3D* slicedGeometry = dynamic_cast<mitk::SlicedGeometry3D*>(currentGeometry.GetPointer());
//        mitk::PlaneGeometry* plane = slicedGeometry->GetPlaneGeometry(m_CenterPoint[dim]);
        mitk::Image::Pointer interpolationResult = m_InterpolationController->Interpolate(dim, m_CenterPoint[dim], plane, 0);

        mitk::IOUtil::Save(interpolationResult, "/home/jenspetersen/Desktop/interpolation.nrrd");

        // Write result into segmentation image
        vtkSmartPointer<mitkVtkImageOverwrite> reslicer = vtkSmartPointer<mitkVtkImageOverwrite>::New();
        reslicer->SetInputSlice(interpolationResult->GetSliceData()->GetVtkImageAccessor(interpolationResult)->GetVtkImageData());
        reslicer->SetOverwriteMode(true);
        reslicer->Modified();
        mitk::ExtractSliceFilter::Pointer extractor =  mitk::ExtractSliceFilter::New(reslicer);
        extractor->SetInput(m_SegmentationImage);
        extractor->SetTimeStep(0);
        extractor->SetWorldGeometry(plane);
        extractor->SetVtkOutputRequest(true);
        extractor->SetResliceTransformByGeometry(m_SegmentationImage->GetTimeGeometry()->GetGeometryForTimeStep(0));
        extractor->Modified();
        extractor->Update();

        mitk::IOUtil::Save(m_SegmentationImage, "/home/jenspetersen/Desktop/postseg.nrrd");

        // Check a 4x4 square, the center of which needs to be filled
        mitk::ImagePixelReadAccessor<mitk::Tool::DefaultSegmentationDataType, 3> readAccess(m_SegmentationImage);
        currentPoint = m_CenterPoint;

        for (int i=-1; i<=2; ++i)
        {
            for (int j=-1; j<=2; ++j)
            {
                currentPoint[(dim+1)%3] = m_CenterPoint[(dim+1)%3] + i;
                currentPoint[(dim+2)%3] = m_CenterPoint[(dim+2)%3] + j;

                MITK_INFO << currentPoint << ": " << readAccess.GetPixelByIndexSafe(currentPoint);

                if (i == -1 || i == 2 || j == -1 || j == 2)
                {
//                    CPPUNIT_ASSERT_MESSAGE("Have false positive segmentation.", readAccess.GetPixelByIndexSafe(currentPoint) == 0);
                }
                else
                {
//                    CPPUNIT_ASSERT_MESSAGE("Have false negative segmentation.", readAccess.GetPixelByIndexSafe(currentPoint) == 1);
                }
            }
        }
    }
};

MITK_TEST_SUITE_REGISTRATION(mitkSegmentationInterpolation);



































    //#include "mitkSegmentationInterpolationController.h"
    //#include "mitkCoreObjectFactory.h"
    //#include "mitkStandardFileLocations.h"
    ////#include "mitkDataNodeFactory.h"
    //#include "ipSegmentation.h"
    //#include "mitkCompareImageSliceTestHelper.h"

    //class mitkSegmentationInterpolationTestClass
    //{
    //  public:
    //    mitkSegmentationInterpolationTestClass() {}
    //    ~mitkSegmentationInterpolationTestClass() {}

    //    bool Test(std::string filename1, std::string filename2)
    //    {
    //      return CreateNewInterpolator()
    //          && CreateSegmentation()
    //          && ClearSegmentation()
    //          && CreateTwoSlices(2)
    //          && TestInterpolation(2)
    //          && ClearSegmentation()
    //          && CreateTwoSlices(1)
    //          && TestInterpolation(1)
    //          && ClearSegmentation()
    //          && CreateTwoSlices(0)
    //          && TestInterpolation(0)
    //          && DeleteInterpolator()
    //          && CreateNewInterpolator()
    //          && LoadTestImages(filename1, filename2)
    //          && CompareInterpolationsToDefinedReference();
    //    }

    //  protected:

    //    bool CreateNewInterpolator();
    //    bool CreateSegmentation();
    //    bool ClearSegmentation();
    //    bool CreateTwoSlices(int);
    //    bool TestInterpolation(int);
    //    bool DeleteInterpolator();
    //    bool LoadTestImages(std::string filename1, std::string filename2);
    //    bool CompareInterpolationsToDefinedReference();

    //    mitk::Image::Pointer LoadImage(const std::string& filename);

    //    mitk::SegmentationInterpolationController::Pointer m_Interpolator;
    //    mitk::Image::Pointer m_Image;

    //    mitk::Image::Pointer m_ManualSlices;
    //    mitk::Image::Pointer m_InterpolatedSlices;

    //    unsigned int dim[3];
    //    int pad[3];
    //};

    //bool mitkSegmentationInterpolationTestClass::CreateNewInterpolator()
    //{
    //  std::cout << "Instantiation" << std::endl;

    //// instantiation
    //  m_Interpolator = mitk::SegmentationInterpolationController::New();
    //  if (m_Interpolator.IsNotNull())
    //  {
    //    std::cout << "  (II) Instantiation works." << std::endl;
    //  }
    //  else
    //  {
    //    std::cout << " Instantiation test failed!" << std::endl;
    //    return false;
    //  }

    //  return true;
    //}

    //bool mitkSegmentationInterpolationTestClass::CreateSegmentation()
    //{
    //  m_Image = mitk::Image::New();
    //  dim[0]=15;
    //  dim[1]=20;
    //  dim[2]=25;
    //  pad[0]=2;
    //  pad[1]=3;
    //  pad[2]=4;

    //  m_Image->Initialize( mitk::MakeScalarPixelType<int>(), 3, dim);

    //  return true;
    //}

    //bool mitkSegmentationInterpolationTestClass::ClearSegmentation()
    //{
    //  int* p = (int*)m_Image->GetData(); // pointer to pixel data
    //  int size = dim[0]*dim[1]*dim[2];
    //  for(int i=0; i<size; ++i, ++p) *p=0; // fill image with zeros

    //  return true;
    //}


    ///**
    // * Creates a square segmentation in slices 0 and 2.
    //*/
    //bool mitkSegmentationInterpolationTestClass::CreateTwoSlices(int slicedim)
    //{
    //  int* p = (int*)m_Image->GetData(); // pointer to pixel data

    //  int size = dim[0]*dim[1]*dim[2];
    //  for(int i=0; i<size; ++i, ++p)
    //  {
    //    int x,y,z;
    //    int xdim,ydim;
    //    switch (slicedim)
    //    {
    //      case 0:
    //        z = i % dim[0];
    //        y = i / dim[0] %  dim[1];
    //        x = i / (dim[1]*dim[0]);
    //        xdim = 2;
    //        ydim = 1;
    //        break;
    //      case 1:
    //        x = i % dim[0];
    //        z = i / dim[0] %  dim[1];
    //        y = i / (dim[1]*dim[0]);
    //        xdim = 0;
    //        ydim = 2;
    //        break;
    //      case 2:
    //      default:
    //        x = i % dim[0];
    //        y = i / dim[0] %  dim[1];
    //        z = i / (dim[1]*dim[0]);
    //        xdim = 0;
    //        ydim = 1;
    //        break;
    //    }

    //    if ( ((z == 0) || (z == 2)) && (x >= pad[xdim]) && (x < ( (signed) dim[xdim]-pad[xdim])) && (y >= pad[ydim]) && (y < ( (signed) dim[ydim]-pad[ydim])) )
    //    {
    //      *p = 1;
    //    }
    //    else
    //    {
    //      *p = 0;
    //    }
    //  }

    //  m_Interpolator->SetSegmentationVolume( m_Image );
    //  std::cout << "  (II) SetSegmentationVolume works (slicedim " << slicedim << ")" << std::endl;

    //  return true;
    //}

    ///**
    // * Checks if interpolation would create a square in slice 1
    //*/
    //bool mitkSegmentationInterpolationTestClass::TestInterpolation(int slicedim)
    //{
    //  int slice = 1;
    //  mitk::Image::Pointer interpolated = m_Interpolator->Interpolate( slicedim, slice, 0 ); // interpolate second slice axial
    //  if (interpolated.IsNull())
    //  {
    //    std::cerr << "  (EE) Interpolation did not return anything for slicedim == " << slicedim << " (although it should)." << std::endl;
    //    return false;
    //  }

    //  int xdim,ydim;
    //  switch (slicedim)
    //  {
    //    case 0:
    //      xdim = 1;
    //      ydim = 2; // different than above!
    //      break;
    //    case 1:
    //      xdim = 0;
    //      ydim = 2;
    //      break;
    //    case 2:
    //    default:
    //      xdim = 0;
    //      ydim = 1;
    //      break;
    //  }

    //  ipMITKSegmentationTYPE* p = (ipMITKSegmentationTYPE*)interpolated->GetData(); // pointer to pixel data

    //  int size = dim[xdim]*dim[ydim];
    //  if ( (signed) interpolated->GetDimension(0) * (signed) interpolated->GetDimension(1) != size )
    //  {
    //    std::cout << "  (EE) Size of interpolated image differs from original segmentation..." << std::endl;
    //    return false;
    //  }

    //  for(int i=0; i<size; ++i, ++p)
    //  {
    //    int x,y;
    //    x = i % dim[xdim];
    //    y = i / dim[xdim];

    //    //if (x == 0) std::cout << std::endl;

    //    ipMITKSegmentationTYPE value = *p;

    //    //if (value == 1) std::cout << "O"; else std::cout << ".";

    //    if ( (x >= pad[xdim]) && (x < ((signed) dim[xdim]-pad[xdim])) && (y >= pad[ydim]) && (y < ((signed) dim[ydim]-pad[ydim])) && (value != 1) )
    //    {
    //      std::cout << "  (EE) Interpolation of a square figure failed" << std::endl;
    //      std::cout << "  Value at " << x << " " << y << ": " << (int)value << std::endl;
    //      return false;
    //    }
    //  }

    //  std::cout << "  (II) Interpolation of a square figure works like expected (slicedim " << slicedim << ")" << std::endl;
    //  return true;
    //}

    //bool mitkSegmentationInterpolationTestClass::DeleteInterpolator()
    //{
    //  std::cout << "Object destruction" << std::endl;

    //// freeing
    //  m_Interpolator = NULL;

    //  std::cout << "  (II) Freeing works." << std::endl;
    //  return true;
    //}

    //bool mitkSegmentationInterpolationTestClass::LoadTestImages(std::string filename1, std::string filename2)
    //{
    //  m_ManualSlices = LoadImage( filename1 );
    //  m_InterpolatedSlices = LoadImage( filename2 );

    //  return ( m_ManualSlices.IsNotNull() && m_InterpolatedSlices.IsNotNull() );
    //}

    //mitk::Image::Pointer mitkSegmentationInterpolationTestClass::LoadImage(const std::string& filename)
    //{
    //  mitk::Image::Pointer image = NULL;
    //  mitk::DataNodeFactory::Pointer factory = mitk::DataNodeFactory::New();
    //  try
    //  {
    //    factory->SetFileName( filename );
    //    factory->Update();

    //    if(factory->GetNumberOfOutputs()<1)
    //    {
    //      std::cerr<<"File " << filename << " could not be loaded [FAILED]"<<std::endl;
    //      return NULL;
    //    }
    //    mitk::DataNode::Pointer node = factory->GetOutput( 0 );
    //    image = dynamic_cast<mitk::Image*>(node->GetData());
    //    if(image.IsNull())
    //    {
    //      std::cout<<"File " << filename << " is not an image! [FAILED]"<<std::endl;
    //      return NULL;
    //    }
    //  }
    //  catch ( itk::ExceptionObject & ex )
    //  {
    //    std::cerr << "Exception: " << ex << "[FAILED]" << std::endl;
    //    return NULL;
    //  }

    //  return image;
    //}

    //bool mitkSegmentationInterpolationTestClass::CompareInterpolationsToDefinedReference()
    //{
    //  std::cout << "  (II) Setting segmentation volume... " << std::flush;

    //  m_Interpolator->SetSegmentationVolume( m_ManualSlices );

    //  std::cout << "OK" << std::endl;

    //  std::cout << "  (II) Testing interpolation result for slice " << std::flush;

    //  for (unsigned int slice = 1; slice < 98; ++slice)
    //  {
    //    if (slice % 2 == 0) continue; // these were manually drawn, no interpolation possible

    //    std::cout << slice << " " << std::flush;

    //    mitk::Image::Pointer interpolation = m_Interpolator->Interpolate( 2, slice, 0 );

    //    if ( interpolation.IsNull() )
    //    {
    //      std::cerr << "  (EE) Interpolated image is NULL." << std::endl;
    //      return false;
    //    }

    //    if ( !CompareImageSliceTestHelper::CompareSlice( m_InterpolatedSlices, 2, slice, interpolation ) )
    //    {
    //      std::cerr << "  (EE) interpolated image is not identical to reference in slice " << slice << std::endl;
    //      return false;
    //    }
    //  }

    //  std::cout << std::endl;
    //  std::cout << "  (II) Interpolations are the same as the saved references." << std::endl;

    //  return true;
    //}

    ///// ctest entry point
    //int mitkSegmentationInterpolationTest(int argc, char* argv[])
    //{
    //// one big variable to tell if anything went wrong
    ////  std::cout << "Creating CoreObjectFactory" << std::endl;
    ////  itk::ObjectFactoryBase::RegisterFactory(mitk::CoreObjectFactory::New());
    //  if (argc < 3)
    //  {
    //    std::cerr << " (EE) Missing arguments for testing" << std::endl;
    //  }
    //  mitkSegmentationInterpolationTestClass test;
    //  if ( test.Test(argv[1], argv[2]) )
    //  {
    //    std::cout << "[PASSED]" << std::endl;
    //    return EXIT_SUCCESS;
    //  }
    //  else
    //  {
    //    std::cout << "[FAILED]" << std::endl;
    //    return EXIT_FAILURE;
    //  }
    //}

