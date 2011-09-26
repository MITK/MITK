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
#include "itkVectorImage.h"
#include "mitkImageCast.h"
#include "MitkDiffusionImagingExports.h"

namespace mitk 
{

  /**
  * \brief this class encapsulates diffusion volumes (vectorimages not
  * yet supported by mitkImage)
  */

  class MitkDiffusionImaging_EXPORT TbssImage : public Image
  {

  public:

    typedef itk::VectorImage<float, 3> ImageType;


    typedef itk::Index<3> IndexType;
    //typedef typename std::vector <Index<3> > RoiType;

    mitkClassMacro( TbssImage, Image )
    itkNewMacro(Self)


    //void SetRequestedRegionToLargestPossibleReg tbssRoi->SetTbssType(mitk::TbssImage<char>::ROI);ion();
    //bool RequestedRegionIsOutsideOfTheBufferedRegion();
    //virtual bool VerifyRequestedRegion();
    //void SetRequestedRegion(itk::DataObject *data);

    ImageType::Pointer GetImage()
    {
      return m_Image;
    }
    void SetImage(ImageType::Pointer image )
    {
      this->m_Image = image;
    }


    void InitializeFromImage()
    {
      MITK_INFO << "make an mitk image that can be shown by mitk";
      this->InitializeByItk(m_Image.GetPointer(),1,1);
    }

    void SetGroupInfo( std::vector< std::pair<std::string, int> > info)
    {
      m_GroupInfo = info;
    }

    std::vector< std::pair<std::string, int> > GetGroupInfo()
    {
      return m_GroupInfo;
    }

    void InitializeFromVectorImage();

    void SetDisplayIndexForRendering(int displayIndex);



  protected:

    TbssImage();

    virtual ~TbssImage(){}

    ImageType::Pointer m_Image;


    std::vector< std::pair<std::string, int> > m_GroupInfo;

    int m_DisplayIndex;




  };

/*
  // Does the same es the normal CastToMitkImage, but needed to reimplemented due to the templatet pixeltype
  template <typename ItkOutputImageType>
  void CastToTbssImage(const ItkOutputImageType* itkimage, itk::SmartPointer< mitk::TbssImage >& tbssoutputimage)
  {
    if(tbssoutputimage.IsNull())
    {
      tbssoutputimage = mitk::TbssImage::New();
    }
    tbssoutputimage->InitializeByItk(itkimage);
    tbssoutputimage->SetChannel(itkimage->GetBufferPointer());
  }


*/



} // namespace mitk
//#include "mitkTbssImage.cpp"
#endif /* __mitkTbssImage__h */
