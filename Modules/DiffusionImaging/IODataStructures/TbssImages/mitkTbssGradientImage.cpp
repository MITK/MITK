/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#ifndef __mitkTbssGradientImage__cpp
#define __mitkTbssGradientImage__cpp


#include "mitkTbssGradientImage.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "mitkImageCast.h"




mitk::TbssGradientImage::TbssGradientImage()
{

}


void mitk::TbssGradientImage::InitializeFromVectorImage()
{
  if(!m_Image)
  {
    MITK_INFO << "TBSS Gradient Image could not be initialized. Set all members first!" << std::endl;
    return;
  }


  typedef itk::Image<float,3> ImgType;
  ImgType::Pointer img = ImgType::New();
  img->SetSpacing( m_Image->GetSpacing() );   // Set the image spacing
  img->SetOrigin( m_Image->GetOrigin() );     // Set the image origin
  img->SetDirection( m_Image->GetDirection() );  // Set the image direction
  img->SetLargestPossibleRegion( m_Image->GetLargestPossibleRegion());
  img->SetBufferedRegion( m_Image->GetLargestPossibleRegion() );
  img->Allocate();

  InitializeByItk( img.GetPointer(), 1, 3 );

  itk::ImageRegionIterator<ImgType> itw (img, img->GetLargestPossibleRegion() );
  itw = itw.Begin();

  itk::ImageRegionConstIterator<ImageType> itr (m_Image, m_Image->GetLargestPossibleRegion() );
  itr = itr.Begin();

  while(!itr.IsAtEnd())
  {
    itw.Set(itr.Get().GetElement(0));
    ++itr;
    ++itw;
  }


  SetImportVolume(img->GetBufferPointer());//, 0, 0, CopyMemory);

  m_DisplayIndex = 0;
  MITK_INFO << "Tbss-Image successfully initialized.";

}


void mitk::TbssGradientImage::SetDisplayIndexForRendering(int displayIndex)
{
  MITK_INFO << "displayindex: " << displayIndex;
  int index = displayIndex;
  int vecLength = m_Image->GetVectorLength();
  index = index > vecLength-1 ? vecLength-1 : index;
  if( m_DisplayIndex != index )
  {
    typedef itk::Image<float,3> ImgType;
    ImgType::Pointer img = ImgType::New();
    CastToItkImage<ImgType>(this, img);

    itk::ImageRegionIterator<ImgType> itw (img, img->GetLargestPossibleRegion() );
    itw = itw.Begin();

    itk::ImageRegionConstIterator<ImageType> itr (m_Image, m_Image->GetLargestPossibleRegion() );
    itr = itr.Begin();

    while(!itr.IsAtEnd())
    {
      itw.Set(itr.Get().GetElement(index));
      ++itr;
      ++itw;
    }
  }

  m_DisplayIndex = index;
}



#endif /* __mitkTbssGradientImage__cpp */
