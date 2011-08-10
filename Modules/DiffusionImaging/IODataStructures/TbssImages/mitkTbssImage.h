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
#include "MitkDiffusionImagingExports.h"

namespace mitk 
{

  /**
  * \brief this class encapsulates diffusion volumes (vectorimages not
  * yet supported by mitkImage)
  */
  template<class TPixelType>
  class MitkDiffusionImaging_EXPORT TbssImage : public Image
  {

  public:
    typedef TPixelType PixelType;
    typedef typename itk::Image<TPixelType, 3>  ImageType;
    typedef itk::Index<3> IndexType;
    typedef typename std::vector<IndexType> RoiType;

    mitkClassMacro( TbssImage, Image )
    itkNewMacro(Self)

    enum Type{
      ROI,
      SKELETON
    };


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

    itkSetMacro(TbssType, Type)
    itkGetMacro(TbssType, Type)
    itkGetMacro(PreprocessedFA, bool)
    itkSetMacro(PreprocessedFA, bool)
    itkGetMacro(PreprocessedFAFile, std::string)
    itkSetMacro(PreprocessedFAFile, std::string)
    itkGetMacro(Structure, std::string)
    itkSetMacro(Structure, std::string)

    void SetRoi(RoiType roi)
    {
      m_Roi = roi;
    }

    RoiType GetRoi()
    {
      return m_Roi;
    }

    void InitializeFromImage()
    {
      MITK_INFO << "make an mitk image that can be shown by mitk";
      this->InitializeByItk(m_Image.GetPointer(),1,1);
    }

  protected:
    TbssImage():m_PreprocessedFA(false)
    {

    }
    virtual ~TbssImage(){}

    typename ImageType::Pointer m_Image;

    Type m_TbssType;

    RoiType m_Roi;

    bool m_PreprocessedFA;
    std::string m_PreprocessedFAFile;

    std::string m_Structure;

  };


  // Does the same es the normal CastToMitkImage, but needed to reimplemented due to the templatet pixeltype
  template <typename ItkOutputImageType, typename PixelType>
  void CastToTbssImage(const ItkOutputImageType* itkimage, itk::SmartPointer< mitk::TbssImage<PixelType> >& tbssoutputimage)
  {
    if(tbssoutputimage.IsNull())
    {
      tbssoutputimage = mitk::TbssImage<PixelType>::New();
    }
    tbssoutputimage->InitializeByItk(itkimage);
    tbssoutputimage->SetChannel(itkimage->GetBufferPointer());
  }

} // namespace mitk

#include "mitkTbssImage.cpp"
#endif /* __mitkTbssImage__h */
