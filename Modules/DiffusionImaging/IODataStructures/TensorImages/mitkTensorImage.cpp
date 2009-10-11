/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2008-02-08 11:19:03 +0100 (Fr, 08 Feb 2008) $
Version:   $Revision: 11989 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkTensorImage.h"

mitk::TensorImage::TensorImage() : Image()
{
}

mitk::TensorImage::~TensorImage()
{

}
//
//void mitk::TensorImage::CopyConstruct(mitk::Image::Pointer img)
//{
//  m_LargestPossibleRegion = img->GetLargestPossibleRegion();
//  m_RequestedRegion = img->GetRequestedRegion();
//  m_Channels.push_back(img->GetChannelData(0).GetPointer());
//  m_Volumes.push_back(img->GetVolumeData(0).GetPointer());
//  m_Slices.push_back(img->GetSliceData(0).GetPointer());
//  m_Dimension = img->GetDimension();
//  m_Dimensions = img->GetDimensions();
//  m_PixelType	 = img->GetPixelType();
//}

