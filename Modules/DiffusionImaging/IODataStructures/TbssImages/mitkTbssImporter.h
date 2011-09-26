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
    typedef itk::Image<float, 4> FloatImage4DType;
    typedef itk::ImageFileReader<FloatImage4DType> FileReaderType4D;

    TbssImporter();

    mitkClassMacro( TbssImporter, Object )
    itkNewMacro(Self)



    TbssImporter(std::string path);

    void SetInputPath(std::string p)
    {
      m_InputPath = p;
    }

    mitk::TbssImage::Pointer Import();


    void SetGroupInfo(std::vector< std::pair<std::string, int> > groups)
    {
      m_Groups = groups;
    }

    std::vector< std::pair<std::string, int> > GetGroupInfo()
    {
      return m_Groups;
    }


  protected:



    virtual ~TbssImporter(){}

    std::string m_InputPath;

    DataImageType::Pointer m_Data;
    std::vector< std::pair<std::string, int> > m_Groups;


  };

}

#include "mitkTbssImporter.cpp"
#endif // __mitkTbssImporter_h
