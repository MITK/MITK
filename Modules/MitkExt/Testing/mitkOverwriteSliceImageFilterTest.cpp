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
#include "mitkOverwriteSliceImageFilter.h"
#include "mitkCoreObjectFactory.h"
#include "mitkDataNodeFactory.h"
#include "mitkCompareImageSliceTestHelper.h"

class mitkOverwriteSliceImageFilterTestClass
{

  public:

static void Test3D( mitk::OverwriteSliceImageFilter* filter, mitk::Image* image, unsigned int& numberFailed )
{
  assert(filter);
  assert(image);

  filter->SetInput( image );
  
  unsigned int initialNumberFailed = numberFailed;
  bool exception = false;
  // first extract slices and rewrite them
  for ( unsigned int sliceDimension = 0; sliceDimension < 6; ++sliceDimension )
  {
    mitk::ExtractImageFilter::Pointer extractor = mitk::ExtractImageFilter::New();
    extractor->SetInput( image );
    extractor->SetSliceDimension( sliceDimension );
    extractor->SetSliceIndex( 2 ); // third slice in that direction

    try 
    {
      extractor->Update();
    }
    catch(...)
    {
      if ( sliceDimension < 3 )
      {
         // probably no sliceindex 2 there or extractor just doesn't work (check the corresponding test)
        std::cout << "  (WW) Couldn't extract slice number 3 from a 3D image. This could be a problem if the image is not only two slices big." << std::endl;
        continue;
      }
      else
      {
        continue; // good
      }
    }

    mitk::Image::Pointer slice = extractor->GetOutput();

    filter->SetSliceDimension( sliceDimension );
    filter->SetSliceIndex( 1 ); // second slice in that direction
    filter->SetSliceImage( slice );

    try
    {
      filter->Update(); // try to overwrite
    }
    catch(...)
    {
      if ( sliceDimension < 3 )
      {
        ++numberFailed;
        std::cerr << "  (EE) Couln't overwrite a slice with data from a neigbor in a " 
                  << image->GetDimension() 
                  << "-dimensional image, sliceDimension " 
                  << sliceDimension 
                  << " sliceIndex 1-2." << "(l. " << __LINE__ << ")" << std::endl;
      }
      else
      {
        // this was expected and is nice to see
        continue;
      }
    }

    mitk::Image::Pointer output = filter->GetOutput();

    if (output.IsNull())
    {
      ++numberFailed;
      std::cerr << "  (EE) Overwrite filter has output NULL and gave no exception for an "
                << image->GetDimension() 
                << "-dimensional image, sliceDimension " 
                << sliceDimension 
                << " sliceIndex 1-2." << "(l. " << __LINE__ << ")" << std::endl;
       continue;
    }

    if (!CompareImageSliceTestHelper::CompareSlice( image, sliceDimension , 1 , slice ))
    {
      ++numberFailed;
      std::cerr << "  (EE) Overwriting a slice seemed to work, but the pixels are not correct for an "
                << image->GetDimension() 
                << "-dimensional image, sliceDimension " 
                << sliceDimension 
                << " sliceIndex 1-2." << "(l. " << __LINE__ << ")" << std::endl;
    }


    // try inserting at a position outside the image
    filter->SetSliceDimension( sliceDimension );
    filter->SetSliceIndex( image->GetDimension(sliceDimension)  ); // last possible slice index + 1
    filter->SetSliceImage( slice );

    exception = false;
    try
    {
      filter->Update(); // try to overwrite
    }
    catch(...)
    {
      exception = true;
    }
    
    if (!exception)
    {
      ++numberFailed;
      std::cerr << "  (EE) Inserting a slice outside the 3D volume did NOT throw an exception for an " 
                << image->GetDimension() 
                << "-dimensional image, sliceDimension " 
                << sliceDimension 
                << " sliceIndex 1-2." << "(l. " << __LINE__ << ")" << std::endl;
    }

 
    mitk::Image::Pointer originalSlice = slice;

    // now test slices that just don't fit (slice too big)
    {
      unsigned int dim[]={ slice->GetDimension(0) + 2, slice->GetDimension(1) + 2 };
      slice = mitk::Image::New();
      slice-> Initialize(mitk::PixelType(typeid(signed int)), 2, dim);
      unsigned int i;
      signed int *p = (signed int*)slice->GetData();
      unsigned int size = dim[0]*dim[1];
      for(i=0; i<size; ++i, ++p)
        *p= (signed int)i;

      // try to insert this bad slice 
      filter->SetSliceImage( slice );
      exception = false;
      try
      {
        filter->Update(); // try to overwrite
      }
      catch(...)
      {
        exception = true;
      }

      if (!exception)
      {
        ++numberFailed;
        std::cerr << "  (EE) Trying to insert a slice of bad dimensions (larger) did NOT throw an exception in an "
                  << image->GetDimension() 
                  << "-dimensional image, sliceDimension " 
                  << sliceDimension 
                  << " sliceIndex 1-2." << "(l. " << __LINE__ << ")" << std::endl;
      }
    }

    // now test slices that just don't fit (slice too small)
    {
      slice = originalSlice;
      if ( (slice->GetDimension(0) <3) || (slice->GetDimension(1) <3) ) continue; // not possible shrink the image much further
      unsigned int dim[]={ slice->GetDimension(0) - 2, slice->GetDimension(1) - 2 };
      slice = mitk::Image::New();
      slice-> Initialize(mitk::PixelType(typeid(signed int)), 2, dim);
      unsigned int i;
      signed int *p = (signed int*)slice->GetData();
      unsigned int size = dim[0]*dim[1];
      for(i=0; i<size; ++i, ++p)
        *p= (signed int)i;

      // try to insert this bad slice 
      filter->SetSliceImage( slice );
      exception = false;
      try
      {
        filter->Update(); // try to overwrite
      }
      catch(...)
      {
        exception = true;
      }

      if (!exception)
      {
        ++numberFailed;
        std::cerr << "  (EE) Trying to insert a slice of bad dimensions (smaller) did NOT throw an exception in an "
                  << image->GetDimension() 
                  << "-dimensional image, sliceDimension " 
                  << sliceDimension 
                  << " sliceIndex 1-2." << "(l. " << __LINE__ << ")" << std::endl;
      }
    }
   
  }
  


  if ( numberFailed == initialNumberFailed )
  {
    std::cout << "  (II) Overwriting works nicely (gives result, pixels are good) "
              << image->GetDimension() 
              << "-dimensional image." << "(l. " << __LINE__ << ")" << std::endl;
  }
}


static void Test2D( mitk::OverwriteSliceImageFilter* filter, mitk::Image* image, unsigned int& numberFailed )
{
  assert(filter);
  assert(image);

  filter->SetInput( image );
  filter->SetSliceImage( image );
  bool exception = false;
  try
  {
    filter->Update();
  }
  catch(...)
  {
    exception = true;
  }

  if (!exception)
  {
    std::cerr << "  (EE) Using OverwriteImageFilter for 2D -> 2D did not throw an exception "
              << "(l. " << __LINE__ << ")" << std::endl;
  }

  unsigned int initialNumberFailed = numberFailed;
  if ( numberFailed == initialNumberFailed )
  {
    std::cout << "  (II) Overwriting works nicely (gives result, pixels are good) "
              << image->GetDimension() 
              << "-dimensional image." << "(l. " << __LINE__ << ")" << std::endl;
  }
}

static void TestOtherD( mitk::OverwriteSliceImageFilter* filter, mitk::Image* image, unsigned int& numberFailed )
{
  assert(filter);
  assert(image);

  filter->SetInput( image );
  filter->SetSliceImage( image );
  bool exception = false;
  try
  {
    filter->Update();
  }
  catch(...)
  {
    exception = true;
  }

  if (!exception)
  {
    std::cerr << "  (EE) Using OverwriteImageFilter did not throw an exception "
              << "(l. " << __LINE__ << ")" << std::endl;
  }

  unsigned int initialNumberFailed = numberFailed;
  if ( numberFailed == initialNumberFailed )
  {
    std::cout << "  (II) Overwriting works nicely (gives result, pixels are good) "
              << image->GetDimension() 
              << "-dimensional image." << "(l. " << __LINE__ << ")" << std::endl;
  }
}


};

/// ctest entry point
int mitkOverwriteSliceImageFilterTest(int argc, char* argv[])
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
    mitk::DataNodeFactory::Pointer factory = mitk::DataNodeFactory::New();
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
      mitk::DataNode::Pointer node = factory->GetOutput( 0 );
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
    mitk::OverwriteSliceImageFilter::Pointer filter = mitk::OverwriteSliceImageFilter::New();
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
      mitkOverwriteSliceImageFilterTestClass::Test2D( filter, image, numberFailed );
    }
    else if ( image->GetDimension() == 3 )
    {
      mitkOverwriteSliceImageFilterTestClass::Test3D( filter, image, numberFailed );
    }
    else
    {
      mitkOverwriteSliceImageFilterTestClass::TestOtherD( filter, image, numberFailed );
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

