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

#ifndef __mitkTbssRoiImage__cpp
#define __mitkTbssRoiImage__cpp


#include "mitkTbssRoiImage.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "mitkImageCast.h"




mitk::TbssRoiImage::TbssRoiImage()
{
  m_PreprocessedFA = false;
}


/*

mitk::TbssRoiImage::~TbssImage()
{

}



void mitk::TbssRoiImage::InitializeFromImage()
{

}

*/

#endif /* __mitkTbssImage__cpp */
