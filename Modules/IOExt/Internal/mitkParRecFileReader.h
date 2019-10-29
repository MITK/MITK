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

#ifndef PARRECFILEREADER_H_HEADER_INCLUDED_C1F48A22
#define PARRECFILEREADER_H_HEADER_INCLUDED_C1F48A22

#include "mitkCommon.h"
#include "mitkFileReader.h"
#include "mitkImageSource.h"

namespace mitk
{
  //##Documentation
  //## @brief Reader to read files in Philips PAR/REC file format
  class ParRecFileReader : public ImageSource, public FileReader
  {
  public:
    mitkClassMacro(ParRecFileReader, FileReader);

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      itkSetStringMacro(FileName);
    itkGetStringMacro(FileName);

    itkSetStringMacro(FilePrefix);
    itkGetStringMacro(FilePrefix);

    itkSetStringMacro(FilePattern);
    itkGetStringMacro(FilePattern);

    static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

  protected:
    void GenerateData() override;

    void GenerateOutputInformation() override;

    ParRecFileReader();

    ~ParRecFileReader() override;

    //##Description
    //## @brief Time when Header was last read
    itk::TimeStamp m_ReadHeaderTime;

  protected:
    std::string m_FileName;
    std::string m_RecFileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;
  };

} // namespace mitk

#endif /* PARRECFILEREADER_H_HEADER_INCLUDED_C1F48A22 */
