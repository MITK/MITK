/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVtiFileReader_h
#define mitkVtiFileReader_h

#include "mitkFileReader.h"
#include "mitkImageSource.h"
#include <MitkLegacyIOExports.h>

namespace mitk
{
  //##Documentation
  //## @brief Reader to read image files in vtk file format
  //## @ingroup MitkLegacyIOModule
  //## @deprecatedSince{2014_10} Use mitk::IOUtils or mitk::FileReaderRegistry instead.
  class DEPRECATED() MITKLEGACYIO_EXPORT VtiFileReader : public ImageSource, public FileReader
  {
  public:
    mitkClassMacro(VtiFileReader, FileReader);

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
    VtiFileReader();

    ~VtiFileReader() override;

    void GenerateData() override;

    //##Description
    //## @brief Time when Header was last read
    // itk::TimeStamp m_ReadHeaderTime;

  protected:
    std::string m_FileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;
  };

} // namespace mitk

#endif
