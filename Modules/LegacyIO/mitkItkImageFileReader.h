/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef ItkImageFileReader_H_HEADER_INCLUDED
#define ItkImageFileReader_H_HEADER_INCLUDED

#include "mitkFileReader.h"
#include "mitkImageSource.h"
#include <MitkLegacyIOExports.h>

namespace mitk
{
  //##Documentation
  //## @brief Reader to read file formats supported by itk
  //## @ingroup MitkLegacyIOModule
  //## @deprecatedSince{2014_10} Use mitk::IOUtils or mitk::FileReaderRegistry instead.
  class MITKLEGACYIO_EXPORT ItkImageFileReader : public ImageSource, public FileReader
  {
  public:
    mitkClassMacro(ItkImageFileReader, FileReader);

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    itkSetStringMacro(FileName);
    itkGetStringMacro(FileName);

    itkSetStringMacro(FilePrefix);
    itkGetStringMacro(FilePrefix);

    itkSetStringMacro(FilePattern);
    itkGetStringMacro(FilePattern);

    static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

  protected:
    void GenerateData() override;

    ItkImageFileReader();

    ~ItkImageFileReader() override;

    std::string m_FileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;
  };

} // namespace mitk

#endif /* ItkImageFileReader_H_HEADER_INCLUDED */
