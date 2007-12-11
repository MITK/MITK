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

#include "mitkExtractImageFilter.h"
#include "mitkCoreObjectFactory.h"
#include "mitkDataTreeNodeFactory.h"
#include "mitkCompareImageSliceTestHelper.h"

unsigned int CompareImageSliceTestHelper::m_Dimension0 = 0;
unsigned int CompareImageSliceTestHelper::m_Dimension1 = 0;
unsigned int CompareImageSliceTestHelper::m_SliceDimension = 0;
unsigned int CompareImageSliceTestHelper::m_SliceIndex = 0;
bool CompareImageSliceTestHelper::m_ComparisonResult = false;
mitk::Image* CompareImageSliceTestHelper::m_SliceImage = NULL;
 


class mitkExtractImageFilterTestClass
{

  public:

static void Test3D( mitk::ExtractImageFilter* filter, mitk::Image* image, unsigned int& numberFailed )
{
  // we expect the result to be the same as the input for 2D (the only possible slice)
  assert(filter);
  assert(image);

  filter->SetInput( image );

  unsigned int initialNumberFailed = numberFailed;

  for ( unsigned int sliceDimension = 0; sliceDimension < 6; ++sliceDimension )
  {
    for ( unsigned int sliceIndex = 1; sliceIndex < 3; ++sliceIndex )
    {
      filter->SetSliceDimension( sliceDimension );
      filter->SetSliceIndex( sliceIndex ); // second slice in that direction
      try
      {
        filter->Update();
      }
      catch(...)
      {
        if ( sliceDimension < 3 )
        {
          ++numberFailed;
          std::cerr << "  (EE) Extracting produced an exception for " 
                    << image->GetDimension() 
                    << "-dimensional image, sliceDimension " 
                    << sliceDimension 
                    << " sliceIndex " << sliceIndex << "." << "(l. " << __LINE__ << ")" << std::endl;
          continue;
        }
        else
        {
          // this was expected and is nice to see
          continue;
        }
      }

      if ( sliceDimension >= 3 )
      {
        // we would expect to get an exception earlier
        ++numberFailed;
        std::cerr << "  (EE) Extracting produced no exception (although it should) for " 
                  << image->GetDimension() 
                  << "-dimensional image, sliceDimension " 
                  << sliceDimension 
                    << " sliceIndex " << sliceIndex << "." << "(l. " << __LINE__ << ")" << std::endl;
        continue;
      }

      mitk::Image::Pointer output = filter->GetOutput();

      if (output.GetPointer() == filter->GetInput())
      {
        ++numberFailed;
        std::cerr << "  (EE) Extracting failed with wrong result (output == input) for " 
                  << image->GetDimension() 
                  << "-dimensional image, sliceDimension " 
                  << sliceDimension 
                    << " sliceIndex " << sliceIndex << "." << "(l. " << __LINE__ << ")" << std::endl;
       }

      if (output->GetDimension() == 2)
      {
        if (!CompareImageSliceTestHelper::CompareSlice( image, sliceDimension , sliceIndex , output ))
        {
          ++numberFailed;
          std::cerr << "  (EE) Extracting extracted the wrong pixels or somehow messed up with a " 
                    << image->GetDimension() 
                    << "-dimensional image, sliceDimension " 
                    << sliceDimension 
                    << " sliceIndex " << sliceIndex << "." << "(l. " << __LINE__ << ")" << std::endl;
        }
      }
      else
      {
        ++numberFailed;
        std::cerr << "  (EE) Extracting failed with wrong result (not 2D) for " 
                  << image->GetDimension() 
                  << "-dimensional image, sliceDimension " 
                  << sliceDimension 
                  << " sliceIndex " << sliceIndex << "." << "(l. " << __LINE__ << ")" << std::endl;
       }

    }
  }

  if ( numberFailed == initialNumberFailed )
  {
    std::cout << "  (II) Extracting works like expected (2D result and all pixels the same) for " 
              << image->GetDimension() 
              << "-dimensional image." << "(l. " << __LINE__ << ")" << std::endl;
  }
}


static void Test2D( mitk::ExtractImageFilter* filter, mitk::Image* image, unsigned int& numberFailed )
{
  // we expect the result to be the same as the input for 2D (the only possible slice)
  assert(filter);
  assert(image);

  filter->SetInput( image );

  unsigned int initialNumberFailed = numberFailed;

  for ( unsigned int sliceDimension = 0; sliceDimension < 6; ++sliceDimension )
  {
    filter->SetSliceDimension( sliceDimension );
    filter->SetSliceIndex( 1 ); // second slice in that direction
    try
    {
      filter->Update();
    }
    catch(...)
    {
      ++numberFailed;
      std::cerr << "  (EE) Extracting produced an exception for " 
                << image->GetDimension() 
                << "-dimensional image, sliceDimension " 
                << sliceDimension 
                << " sliceIndex 1." << "(l. " << __LINE__ << ")" << std::endl;
      continue;
    }

    mitk::Image::Pointer output = filter->GetOutput();

    if (output.GetPointer() != filter->GetInput())
    {
      ++numberFailed;
      std::cerr << "  (EE) Extracting failed with wrong result for " 
                << image->GetDimension() 
                << "-dimensional image, sliceDimension " 
                << sliceDimension 
                << " sliceIndex 1." << "(l. " << __LINE__ << ")" << std::endl;
     }

  }

  if ( numberFailed == initialNumberFailed )
  {
    std::cout << "  (II) Extracting works like expected for " 
              << image->GetDimension() 
              << "-dimensional image." << "(l. " << __LINE__ << ")" << std::endl;
  }
}


static void TestOtherD( mitk::ExtractImageFilter* filter, mitk::Image* image, unsigned int& numberFailed )
{
  // we expect the result to be empty for images other than 2D or 3D
  assert(filter);
  assert(image);

  filter->SetInput( image );

  unsigned int initialNumberFailed = numberFailed;

  for ( unsigned int sliceDimension = 0; sliceDimension < 6; ++sliceDimension )
  {
    filter->SetSliceDimension( sliceDimension );
    filter->SetSliceIndex( 1 ); // second slice in that direction
    try
    {
      filter->Update();
    }
    catch(...)
    {
     continue;
    }

    // no exception
    ++numberFailed;
    std::cerr << "  (EE) Extracting produced no exception for " 
              << image->GetDimension() 
              << "-dimensional image, sliceDimension " 
              << sliceDimension 
              << " sliceIndex 1." << "(l. " << __LINE__ << ")" << std::endl;
 }

  if ( numberFailed == initialNumberFailed )
  {
    std::cout << "  (II) Extracting works like expected for " 
              << image->GetDimension() 
              << "-dimensional image." << "(l. " << __LINE__ << ")" << std::endl;
  }
}

};

/// ctest entry point
int mitkExtractImageFilterTest(int argc, char* argv[])
{
  // one big variable to tell if anything went wrong
    unsigned int numberFailed(0);
 
  // need one parameter (image filename)
    if(argc==0)
    {
      std::cerr<<"No file specified [FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
 
    std::cout << "Creating CoreObjectFactory" << std::endl;
    itk::ObjectFactoryBase::RegisterFactory(mitk::CoreObjectFactory::New());

  // load the image

    mitk::Image::Pointer image = NULL;
    mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();
    try
    {
      std::cout << "Testing with parameter '" << argv[1] << "'" << std::endl;
      factory->SetFileName( argv[1] );
      factory->Update();

      if(factory->GetNumberOfOutputs()<1)
      {
        std::cerr<<"File could not be loaded [FAILED]"<<std::endl;
        return EXIT_FAILURE;
      }
      mitk::DataTreeNode::Pointer node = factory->GetOutput( 0 );
      image = dynamic_cast<mitk::Image*>(node->GetData());
      if(image.IsNull())
      {
        std::cout<<"File not an image - test will not be applied [PASSED]"<<std::endl;
        std::cout<<"[TEST DONE]"<<std::endl;
        return EXIT_SUCCESS;
      }
    }
    catch ( itk::ExceptionObject & ex )
    {
      ++numberFailed;
      std::cerr << "Exception: " << ex << "[FAILED]" << std::endl;
      return EXIT_FAILURE;
    }

    std::cout << "  (II) Could load image." << std::endl;
    std::cout << "Testing filter instantiation" << std::endl;

  // instantiation
    mitk::ExtractImageFilter::Pointer filter = mitk::ExtractImageFilter::New();
    if (filter.IsNotNull())
    {
      std::cout << "  (II) Instantiation works." << std::endl;
    }
    else
    {
      ++numberFailed;
      std::cout << "Test failed, and it's the ugliest one!" << std::endl;
      return EXIT_FAILURE;
    }

  // some real work
    if ( image->GetDimension() == 2 )
    {
      mitkExtractImageFilterTestClass::Test2D( filter, image, numberFailed );
    }
    else if ( image->GetDimension() == 3 )
    {
      mitkExtractImageFilterTestClass::Test3D( filter, image, numberFailed );
    }
    else
    {
      mitkExtractImageFilterTestClass::TestOtherD( filter, image, numberFailed );
    }
  
    std::cout << "Testing filter destruction" << std::endl;

  // freeing
    filter = NULL;
      
    std::cout << "  (II) Freeing works." << std::endl;

    if (numberFailed > 0)
    {
      std::cerr << numberFailed << " tests failed." << std::endl;
      return EXIT_FAILURE;
    }
    else
    {
      std::cout << "PASSED all tests." << std::endl;
      return EXIT_SUCCESS;
    }
}


