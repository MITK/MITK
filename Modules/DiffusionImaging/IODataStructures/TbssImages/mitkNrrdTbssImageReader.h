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

#ifndef __mitkNrrdTbssImageReader_h
#define __mitkNrrdTbssImageReader_h

#include "mitkCommon.h"
#include "mitkFileReader.h"
#include "mitkTbssImageSource.h"
#include "itkImage.h"
#include "itkVectorImage.h"
#include "mitkTbssImage.h"

namespace mitk
{

  /** \brief
  */


  class MitkDiffusionImaging_EXPORT NrrdTbssImageReader : public mitk::TbssImageSource, public FileReader
  {
  public:

    typedef mitk::TbssImage OutputType;
    typedef itk::VectorImage<float,3>     ImageType;
    typedef TbssImageSource  TbssVolSourceType;



    mitkClassMacro( NrrdTbssImageReader, TbssVolSourceType )
    itkNewMacro(Self)

    const char* GetFileName() const;
    void SetFileName(const char* aFileName);
    const char* GetFilePrefix() const;
    void SetFilePrefix(const char* aFilePrefix);
    const char* GetFilePattern() const;
    void SetFilePattern(const char* aFilePattern);

    static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

  protected:

    /** Does the real work. */
    virtual void GenerateData();
    virtual void GenerateOutputInformation();


    std::string m_FileName;
    std::string m_FilePrefix;
    std::string m_FilePattern;

    std::vector< std::pair<std::string, int> > m_GroupInfo;

    OutputType::Pointer m_OutputCache;
    itk::TimeStamp m_CacheTime;


    void Tokenize(const std::string& str,
                  std::vector<std::string>& tokens,
                  const std::string& delimiters = " ")
    {
      // Skip delimiters at beginning.
      std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
      // Find first "non-delimiter".
      std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

      while (std::string::npos != pos || std::string::npos != lastPos)
      {
          // Found a token, add it to the vector.
          tokens.push_back(str.substr(lastPos, pos - lastPos));
          // Skip delimiters.  Note the "not_of"
          lastPos = str.find_first_not_of(delimiters, pos);
          // Find next "non-delimiter"
          pos = str.find_first_of(delimiters, lastPos);
      }
    }



  private:
    void operator=(const Self&); //purposely not implemented
  };

} //namespace MITK

//#include "mitkNrrdTbssImageReader.cpp"

#endif // __mitkNrrdTbssImageReader_h
