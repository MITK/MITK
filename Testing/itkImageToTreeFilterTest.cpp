/*=========================================================================

Program:   Insight Segmentation & Registration Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) Insight Software Consortium. All rights reserved.
See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <itkImage.h>
#include <itkITTFilterContext.h>
#include <itkImageToTreeFilter.h>

int itkImageToTreeFilterTest(int, char* argv[] )
{
  typedef unsigned char   PixelType;
  const unsigned int      Dimension = 3;

  typedef itk::Image<PixelType, Dimension>        ImageType;
  typedef ImageType::Pointer                      ImagePointer;
  typedef ImageType::PointType                    PointType;
//   typedef itk::ImageFileReader<ImageType>         ImageReaderType;
//   typedef ImageReaderType::Pointer                ImageReaderPointer;

  // test classes
  typedef itk::ImageToTreeFilter<ImageType>       ImageToTreeFilterType;
  typedef ImageToTreeFilterType::Pointer          ImageToTreeFilterPointer;
  typedef itk::ITTFilterContext<ImageType, PointType>
                                                  FilterContextType;
  typedef FilterContextType::Pointer              FilterContextPointer;
  typedef FilterContextType::PointQueueType       PointQueueType;

  // TODO: read files with mitk and convert to itk
  // read in first file
//   ImageReaderPointer reader = ImageReaderType::New();
//   reader->SetFileName(argv[1]);
//   reader->Update();
//
//   ImagePointer testImage = reader->GetOutput();
//   ImagePointer testImage2;

  /******************************************************************
   * TEST 1: Saving and loading data objects to the filter context
  *****************************************************************/
  std::cout << "Testing ITTFilterContext for storage of objects...\n";
  FilterContextPointer filterContext = FilterContextType::New();

  PointType testPoint1;
  testPoint1.Fill(0);


  std::cout << "[TEST PASSED]\n";

  /****************************************************************
   * TEST 2: Initialising the filter
   ****************************************************************/
  ImageToTreeFilterPointer testFilter = ImageToTreeFilterType::New();



  std::cout << "[TEST DONE]\n";
  return EXIT_SUCCESS;
}

