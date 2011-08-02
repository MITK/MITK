/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-07 17:17:57 +0100 (Do, 07 Feb 2008) $
Version:   $Revision: 11989 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef __mitkTbssImage__h
#define __mitkTbssImage__h

#include "mitkImage.h"
#include "itkImage.h"
#include "mitkImageCast.h"

namespace mitk 
{

  /**
  * \brief this class encapsulates diffusion volumes (vectorimages not
  * yet supported by mitkImage)
  */
  template<class TPixelType>
  class TbssImage : public Image
  {

  public:
    typedef TPixelType PixelType;
    typedef typename itk::Image<TPixelType, 3>  ImageType;


    mitkClassMacro( TbssImage, Image )
    itkNewMacro(Self)

    //void SetRequestedRegionToLargestPossibleRegion();
    //bool RequestedRegionIsOutsideOfTheBufferedRegion();
    //virtual bool VerifyRequestedRegion();
    //void SetRequestedRegion(itk::DataObject *data);

    typename ImageType::Pointer GetImage()
    {
      return m_Image;
    }
    void SetImage(typename ImageType::Pointer image )
    {
      this->m_Image = image;
    }

    itkSetMacro(TbssType, std::string)
    itkGetMacro(TbssType, std::string)

    void InitializeFromImage()
    {
        MITK_INFO << "make an mitk image that can be shown by mitk";
        this->InitializeByItk(m_Image.GetPointer(),1,1);
    }

  protected:
    TbssImage(){}
    virtual ~TbssImage(){}

    typename ImageType::Pointer m_Image;

    std::string m_TbssType;

  };

} // namespace mitk

#include "mitkTbssImage.cpp"
#endif /* __mitkTbssImage__h */
