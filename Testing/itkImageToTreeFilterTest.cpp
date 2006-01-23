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
#include <itkImageToTreeFilter.h>


namespace itk
{
  typedef unsigned char   PixelType;
  const unsigned int      Dimension = 3;

  typedef Image<PixelType, Dimension>     ImageType;

  typedef ImageToTreeFilter<ImageType>    ImageToTreeFilterType;
  typedef ImageToTreeFilterType::Pointer  ImageToTreeFilterPointer;

  int itkImageToTreeFilterTest(int, char* [] )
  {
    /******************************************************************
     * TEST 1: Saving and loading data objects to the filter context
    *****************************************************************/

    //itkITTFilterContext::Pointer filterContext = itkITTFilterContext::New();

    /****************************************************************
     * TEST 2: Initialising the filter
     ****************************************************************/
    ImageToTreeFilterPointer testFilter = ImageToTreeFilterType::New();




    return EXIT_SUCCESS;
  }

}

