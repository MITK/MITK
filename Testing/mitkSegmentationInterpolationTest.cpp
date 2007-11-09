/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "mitkSegmentationInterpolation.h"
#include "mitkCoreObjectFactory.h"
#include "mitkStandardFileLocations.h"
#include "mitkDataTreeNodeFactory.h"
#include "ipSegmentation.h"
#include "mitkCompareImageSliceTestHelper.h"

class mitkSegmentationInterpolationTestClass
{
  public:
    mitkSegmentationInterpolationTestClass() {}
    ~mitkSegmentationInterpolationTestClass() {}

    bool Test()
    {
      return CreateNewInterpolator()
          && CreateSegmentation()
          && ClearSegmentation()
          && CreateTwoSlices(2)
          && TestInterpolation(2)
          && ClearSegmentation()
          && CreateTwoSlices(1)
          && TestInterpolation(1)
          && ClearSegmentation()
          && CreateTwoSlices(0)
          && TestInterpolation(0)
          && DeleteInterpolator()
          && CreateNewInterpolator()
          && LoadTestImages()
          && CompareInterpolationsToDefinedReference();
    }

  protected:
    
    bool CreateNewInterpolator();
    bool CreateSegmentation();
    bool ClearSegmentation();
    bool CreateTwoSlices(int);
    bool TestInterpolation(int);
    bool DeleteInterpolator();
    bool LoadTestImages();
    bool CompareInterpolationsToDefinedReference();

    mitk::Image::Pointer LoadImage(const std::string& filename);

    mitk::SegmentationInterpolation::Pointer m_Interpolator;
    mitk::Image::Pointer m_Image;
    
    mitk::Image::Pointer m_ManualSlices;
    mitk::Image::Pointer m_InterpolatedSlices;

    unsigned int dim[3];
    int pad[3];
};
    
bool mitkSegmentationInterpolationTestClass::CreateNewInterpolator()
{
  std::cout << "Instantiation" << std::endl;

// instantiation
  m_Interpolator = mitk::SegmentationInterpolation::New();
  if (m_Interpolator.IsNotNull())
  {
    std::cout << "  (II) Instantiation works." << std::endl;
  }
  else
  {
    std::cout << " Instantiation test failed!" << std::endl;
    return false;
  }

  return true;
}

bool mitkSegmentationInterpolationTestClass::CreateSegmentation()
{
  m_Image = mitk::Image::New();
  dim[0]=15;
  dim[1]=20;
  dim[2]=25;
  pad[0]=2;
  pad[1]=3;
  pad[2]=4;

  m_Image->Initialize(mitk::PixelType(typeid(int)), 3, dim);

  return true;
}

bool mitkSegmentationInterpolationTestClass::ClearSegmentation()
{
  int* p = (int*)m_Image->GetData(); // pointer to pixel data
  int size = dim[0]*dim[1]*dim[2];
  for(int i=0; i<size; ++i, ++p) *p=0; // fill image with zeros
  
  return true;
}


/**
 * Creates a square segmentation in slices 0 and 2.
*/
bool mitkSegmentationInterpolationTestClass::CreateTwoSlices(int slicedim)
{
  int* p = (int*)m_Image->GetData(); // pointer to pixel data

  int size = dim[0]*dim[1]*dim[2];
  for(int i=0; i<size; ++i, ++p)
  {
    int x,y,z;
    int xdim,ydim;
    switch (slicedim)
    {
      case 0:
        z = i % dim[0];
        y = i / dim[0] %  dim[1];
        x = i / (dim[1]*dim[0]);
        xdim = 2;
        ydim = 1;
        break;
      case 1:
        x = i % dim[0];
        z = i / dim[0] %  dim[1];
        y = i / (dim[1]*dim[0]);
        xdim = 0;
        ydim = 2;
        break;
      case 2:
      default:
        x = i % dim[0];
        y = i / dim[0] %  dim[1];
        z = i / (dim[1]*dim[0]);
        xdim = 0;
        ydim = 1;
        break;
    }
      
    if ( ((z == 0) || (z == 2)) && (x >= pad[xdim]) && (x < ( (signed) dim[xdim]-pad[xdim])) && (y >= pad[ydim]) && (y < ( (signed) dim[ydim]-pad[ydim])) )
    {
      *p = 1;
    }
    else
    {
      *p = 0;
    }
  }

  m_Interpolator->SetSegmentationVolume( m_Image );
  std::cout << "  (II) SetSegmentationVolume works (slicedim " << slicedim << ")" << std::endl;

  return true;
}

/**
 * Checks if interpolation would create a square in slice 1
*/
bool mitkSegmentationInterpolationTestClass::TestInterpolation(int slicedim)
{
  int slice = 1;
  mitk::Image::Pointer interpolated = m_Interpolator->Interpolate( slicedim, slice ); // interpolate second slice transversal
  if (interpolated.IsNull())
  {
    std::cerr << "  (EE) Interpolation did not return anything for slicedim == " << slicedim << " (although it should)." << std::endl;
    return false;
  }

  int xdim,ydim;
  switch (slicedim)
  {
    case 0:
      xdim = 1;
      ydim = 2; // different than above!
      break;
    case 1:
      xdim = 0;
      ydim = 2;
      break;
    case 2:
    default:
      xdim = 0;
      ydim = 1;
      break;
  }
 
  ipMITKSegmentationTYPE* p = (ipMITKSegmentationTYPE*)interpolated->GetData(); // pointer to pixel data

  int size = dim[xdim]*dim[ydim];
  if ( (signed) interpolated->GetDimension(0) * (signed) interpolated->GetDimension(1) != size )
  {
    std::cout << "  (EE) Size of interpolated image differs from original segmentation..." << std::endl;
    return false;
  }

  for(int i=0; i<size; ++i, ++p)
  {
    int x,y;
    x = i % dim[xdim];
    y = i / dim[xdim];

    //if (x == 0) std::cout << std::endl;
      
    ipMITKSegmentationTYPE value = *p;
   
    //if (value == 1) std::cout << "O"; else std::cout << ".";

    if ( (x >= pad[xdim]) && (x < ((signed) dim[xdim]-pad[xdim])) && (y >= pad[ydim]) && (y < ((signed) dim[ydim]-pad[ydim])) && (value != 1) )
    {
      std::cout << "  (EE) Interpolation of a square figure failed" << std::endl;
      std::cout << "  Value at " << x << " " << y << ": " << (int)value << std::endl;
      return false;
    }
  }
 
  std::cout << "  (II) Interpolation of a square figure works like expected (slicedim " << slicedim << ")" << std::endl;
  return true;
}

bool mitkSegmentationInterpolationTestClass::DeleteInterpolator()
{
  std::cout << "Object destruction" << std::endl;

// freeing
  m_Interpolator = NULL;
    
  std::cout << "  (II) Freeing works." << std::endl;
  return true;
}

bool mitkSegmentationInterpolationTestClass::LoadTestImages()
{
  std::string filename1 = mitk::StandardFileLocations::GetInstance()->FindFile("interpolation_test_manual.pic.gz", "../mitk/Testing/Data/");
  std::cout << "Found test image (manual slices) in '" << filename1 << "'" << std::endl;

  std::string filename2 = mitk::StandardFileLocations::GetInstance()->FindFile("interpolation_test_result.pic.gz", "../mitk/Testing/Data/");
  std::cout << "Found test image (reference for interpolation) in '" << filename2 << "'" << std::endl;
  
  if ( filename1.empty() || filename2.empty() )
  {
    return false;
  }
  else
  {
    m_ManualSlices = LoadImage( filename1 );
    m_InterpolatedSlices = LoadImage( filename2 );

    return ( m_ManualSlices.IsNotNull() && m_InterpolatedSlices.IsNotNull() );
  }

  return true;
}
    
mitk::Image::Pointer mitkSegmentationInterpolationTestClass::LoadImage(const std::string& filename)
{
  mitk::Image::Pointer image = NULL;
  mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();
  try
  {
    factory->SetFileName( filename );
    factory->Update();

    if(factory->GetNumberOfOutputs()<1)
    {
      std::cerr<<"File " << filename << " could not be loaded [FAILED]"<<std::endl;
      return NULL;
    }
    mitk::DataTreeNode::Pointer node = factory->GetOutput( 0 );
    image = dynamic_cast<mitk::Image*>(node->GetData());
    if(image.IsNull())
    {
      std::cout<<"File " << filename << " is not an image! [FAILED]"<<std::endl;
      return NULL;
    }
  }
  catch ( itk::ExceptionObject & ex )
  {
    std::cerr << "Exception: " << ex << "[FAILED]" << std::endl;
    return NULL;
  }

  return image;
}
          
bool mitkSegmentationInterpolationTestClass::CompareInterpolationsToDefinedReference()
{
  std::cout << "  (II) Setting segmentation volume... " << std::flush;

  m_Interpolator->SetSegmentationVolume( m_ManualSlices );

  std::cout << "OK" << std::endl;
    
  std::cout << "  (II) Testing interpolation result for slice " << std::flush;

  for (unsigned int slice = 1; slice < 98; ++slice)
  {
    if (slice % 2 == 0) continue; // these were manually drawn, no interpolation possible

    std::cout << slice << " " << std::flush;

    mitk::Image::Pointer interpolation = m_Interpolator->Interpolate( 2, slice );

    if ( interpolation.IsNull() )
    {
      std::cerr << "  (EE) Interpolated image is NULL." << std::endl;
      return false;
    }

    if ( !CompareImageSliceTestHelper::CompareSlice( m_InterpolatedSlices, 2, slice, interpolation ) )
    {
      std::cerr << "  (EE) interpolated image is not identical to reference in slice " << slice << std::endl;
      return false;
    }
  }

  std::cout << std::endl;
  std::cout << "  (II) Interpolations are the same as the saved references." << std::endl;

  return true;
}

/// ctest entry point
int mitkSegmentationInterpolationTest(int /*argc*/, char* /*argv*/[])
{
// one big variable to tell if anything went wrong
  std::cout << "Creating CoreObjectFactory" << std::endl;
  itk::ObjectFactoryBase::RegisterFactory(mitk::CoreObjectFactory::New());

  mitkSegmentationInterpolationTestClass test;
  if ( test.Test() )
  {
    std::cout << "[PASSED]" << std::endl;
    return EXIT_SUCCESS;
  }
  else
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
}

