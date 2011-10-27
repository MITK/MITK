/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __mitkTbssImporter_h
#define __mitkTbssImporter_h

#include "mitkCommon.h"
#include "mitkFileReader.h"

#include "itkImage.h"
#include "itkVectorImage.h"
#include "itkImageFileReader.h"
#include "mitkTbssImage.h"


namespace mitk
{
  //template<class TPixelType>
  class TbssImporter : public itk::Object {

  public:
   // typedef TPixelType PixelType;
    typedef itk::VectorImage<float,3> DataImageType; // type of the 3d vector image containing the skeletonized images
    typedef itk::VectorImage<int,3> VectorImageType; // Datatype of the tbss gradient images
    typedef itk::Image<float, 4> FloatImage4DType;
    typedef itk::ImageFileReader<FloatImage4DType> FileReaderType4D;
    typedef itk::ImageFileReader<VectorImageType> VectorReaderType;

    typedef itk::Image<float, 3> FloatImage3DType;
    typedef itk::ImageFileReader<FloatImage3DType> FileReaderType3D;



    mitkClassMacro( TbssImporter, Object )
    itkNewMacro(Self)




    mitk::TbssImage::Pointer Import();

    mitk::TbssImage::Pointer ImportMeta();


    void SetGroupInfo(std::vector< std::pair<std::string, int> > groups)
    {
      m_Groups = groups;
    }

    std::vector< std::pair<std::string, int> > GetGroupInfo()
    {
      return m_Groups;
    }

    void SetTbssDatasets(std::vector< std::pair<std::string, std::string> > files)
    {
      m_MetaFiles = files;
    }

    void SetMeasurementInfo(std::string s)
    {
      m_MeasurementInfo = s;
    }

    std::string GetMeasurementInfo()
    {
      return m_MeasurementInfo;
    }


    void SetImportVolume(FloatImage4DType::Pointer inputVolume)
    {
      m_InputVolume = inputVolume;
    }

  protected:



    virtual ~TbssImporter(){}

    DataImageType::Pointer m_Data;
    std::vector< std::pair<std::string, int> > m_Groups;
    std::vector< std::pair<std::string, std::string> > m_MetaFiles;

    std::string m_MeasurementInfo;


    FloatImage4DType::Pointer m_InputVolume;
    

    mitk::TbssImage::MetaDataFunction RetrieveTbssFunction(std::string s);


  };

}

#include "mitkTbssImporter.cpp"
#endif // __mitkTbssImporter_h
