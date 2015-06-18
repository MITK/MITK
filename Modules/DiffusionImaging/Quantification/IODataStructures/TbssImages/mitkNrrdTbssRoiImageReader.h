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

#ifndef __mitkNrrdTbssRoiImageReader_h
#define __mitkNrrdTbssRoiImageReader_h

#include "mitkCommon.h"
#include "mitkFileReader.h"
#include "mitkTbssRoiImageSource.h"
#include "itkImage.h"
#include "mitkTbssRoiImage.h"

namespace mitk
{

  /** \brief
  */


  class MITKQUANTIFICATION_EXPORT NrrdTbssRoiImageReader : public mitk::TbssRoiImageSource, public FileReader
  {
  public:

    typedef mitk::TbssRoiImage OutputType;
    typedef itk::Image<unsigned char,3>     ImageType;
    typedef TbssRoiImageSource  TbssVolSourceType;



    mitkClassMacro( NrrdTbssRoiImageReader, TbssVolSourceType )
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    const char* GetFileName() const override;
    void SetFileName(const char* aFileName) override;
    const char* GetFilePrefix() const override;
    void SetFilePrefix(const char* aFilePrefix) override;
    const char* GetFilePattern() const override;
    void SetFilePattern(const char* aFilePattern) override;

    static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

  protected:

    /** Does the real work. */
    virtual void GenerateData() override;

    void ReadRoiInfo(itk::MetaDataDictionary dict);

    std::string m_FileName;
    std::string m_FilePrefix;
    std::string m_FilePattern;

    OutputType::Pointer m_OutputCache;
    itk::TimeStamp m_CacheTime;



  private:
    void operator=(const Self&); //purposely not implemented
  };

} //namespace MITK

//#include "mitkNrrdTbssRoiImageReader.cpp"

#endif // __mitkNrrdTbssImageReader_h
