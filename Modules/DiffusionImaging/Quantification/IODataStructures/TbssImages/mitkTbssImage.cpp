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

#ifndef __mitkTbssImage__cpp
#define __mitkTbssImage__cpp


#include "mitkTbssImage.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "mitkImageCast.h"




mitk::TbssImage::TbssImage()
{

}


void mitk::TbssImage::InitializeFromVectorImage()
{
  if(!m_Image)
  {
    MITK_INFO << "TBSS Image could not be initialized. Set all members first!" << std::endl;
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

  int vecLength = m_Image->GetVectorLength();
  InitializeByItk( img.GetPointer(), 1, vecLength );

  //for(int i=0; i<vecLength; i++)
  //{TbssImage();


  itk::ImageRegionIterator<ImgType> itw (img, img->GetLargestPossibleRegion() );
  itw.GoToBegin();

  itk::ImageRegionConstIterator<ImageType> itr (m_Image, m_Image->GetLargestPossibleRegion() );
  itr.GoToBegin();

  while(!itr.IsAtEnd())
  {
    itw.Set(itr.Get().GetElement(0));
    ++itr;
    ++itw;
  }

  // init
  SetImportVolume(img->GetBufferPointer());//, 0, 0, CopyMemory);
    //SetVolume( img->GetBufferPointer(), i );
  //}::

  m_DisplayIndex = 0;
  MITK_INFO << "Tbss-Image successfully initialized.";

}

void mitk::TbssImage::SetDisplayIndexForRendering(int displayIndex)
{
  MITK_INFO << "displayindex: " << displayIndex;
  int index = displayIndex;
  int vecLength = m_Image->GetVectorLength();
  index = index > vecLength-1 ? vecLength-1 : index;
  if( m_DisplayIndex != index )
  {
    typedef itk::Image<float,3> ImgType;
    ImgType::Pointer img = ImgType::New();
    CastToItkImage(this, img);

    itk::ImageRegionIterator<ImgType> itw (img, img->GetLargestPossibleRegion() );
    itw.GoToBegin();

    itk::ImageRegionConstIterator<ImageType> itr (m_Image, m_Image->GetLargestPossibleRegion() );
    itr.GoToBegin();

    while(!itr.IsAtEnd())
    {
      itw.Set(itr.Get().GetElement(index));
      ++itr;
      ++itw;
    }
  }

  m_DisplayIndex = index;
}



#endif /* __mitkTbssImage__cpp */
