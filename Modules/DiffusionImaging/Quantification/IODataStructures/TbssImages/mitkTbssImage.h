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


#ifndef __mitkTbssImage__h
#define __mitkTbssImage__h

#include "mitkImage.h"
#include "itkVectorImage.h"
#include "mitkImageCast.h"
#include "MitkQuantificationExports.h"

namespace mitk
{

  /**
  * \brief this class encapsulates diffusion volumes (vectorimages not
  * yet supported by mitkImage)
  */

  class MITKQUANTIFICATION_EXPORT TbssImage : public Image
  {

  public:

    typedef itk::VectorImage<float, 3> ImageType;


    typedef itk::Index<3> IndexType;
    //typedef typename std::vector <Index<3> > RoiType;

    mitkClassMacro( TbssImage, Image )
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)


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


    void SetMeasurementInfo(std::string s)
    {
      m_MeasurementInfo = s;
    }

    std::string GetMeasurementInfo()
    {
      return m_MeasurementInfo;
    }

    void InitializeFromVectorImage();

    void SetDisplayIndexForRendering(int displayIndex);





  protected:

    TbssImage();

    ~TbssImage() override{}

    ImageType::Pointer m_Image;


    std::vector< std::pair<std::string, int> > m_GroupInfo;

    int m_DisplayIndex;

    std::string m_MeasurementInfo;



  };


} // namespace mitk
//#include "mitkTbssImage.cpp"
#endif /* __mitkTbssImage__h */
