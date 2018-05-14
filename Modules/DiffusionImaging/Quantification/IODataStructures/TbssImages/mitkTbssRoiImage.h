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


#ifndef __mitkTbssRoiImage__h
#define __mitkTbssRoiImage__h

#include "mitkImage.h"
#include "itkImage.h"
#include "mitkImageCast.h"
#include "MitkQuantificationExports.h"

namespace mitk
{

  /**
  * \brief this class encapsulates diffusion volumes (vectorimages not
  * yet supported by mitkImage)
  */

  class MITKQUANTIFICATION_EXPORT TbssRoiImage : public Image
  {

  public:

    typedef itk::Image<unsigned char, 3>  ImageType;
    typedef itk::Index<3> IndexType;
    //typedef typename std::vector <Index<3> > RoiType;

    mitkClassMacro( TbssRoiImage, Image )
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)


    //void SetRequestedRegionToLargestPossibleRegion();
    //bool RequestedRegionIsOutsideOfTheBufferedRegion();
    //virtual bool VerifyRequestedRegion();
    //void SetRequestedRegion( const itk::DataObject *data);

    ImageType::Pointer GetImage()
    {
      return m_Image;
    }
    void SetImage(ImageType::Pointer image )
    {
      this->m_Image = image;
    }

    itkGetMacro(PreprocessedFA, bool)
    itkSetMacro(PreprocessedFA, bool)
    itkGetMacro(PreprocessedFAFile, std::string)
    itkSetMacro(PreprocessedFAFile, std::string)
    itkGetMacro(Structure, std::string)
    itkSetMacro(Structure, std::string)

    void SetRoi(std::vector< itk::Index<3> > roi)
    {
      m_Roi = roi;
    }


    // depricated
    std::vector< itk::Index<3> > GetRoi()
    {
      return m_Roi;
    }


    void InitializeFromImage()
    {
      MITK_INFO << "make an mitk image that can be shown by mitk";
      this->InitializeByItk(m_Image.GetPointer());

      this->SetChannel(m_Image->GetBufferPointer());
    }




  protected:

    TbssRoiImage();

    ~TbssRoiImage() override{}

    ImageType::Pointer m_Image;

    std::vector< itk::Index<3> > m_Roi;

    bool m_PreprocessedFA;
    std::string m_PreprocessedFAFile;

    std::string m_Structure;


  };


  // Does the same es the normal CastToMitkImage, but needed to reimplemented due to the templatet pixeltype
  /*
  template <typename ItkOutputImageType>
  void CastToTbssImage(const ItkOutputImageType* itkimage, itk::SmartPointer< mitk::TbssRoiImage >& tbssoutputimage)
  {
    if(tbssoutputimage.IsNull())
    {
      tbssoutputimage = mitk::TbssRoiImage::New();
    }
    tbssoutputimage->InitializeByItk(itkimage);
    tbssoutputimage->SetChannel(itkimage->GetBufferPointer());
  }
*/





} // namespace mitk

//#include "mitkTbssRoiImage.cpp"
#endif /* __mitkTbssRoiImage__h */
