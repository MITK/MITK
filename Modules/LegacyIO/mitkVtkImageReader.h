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


#ifndef VtkImageReader_H_HEADER_INCLUDED
#define VtkImageReader_H_HEADER_INCLUDED

#include <MitkLegacyIOExports.h>
#include "mitkFileReader.h"
#include "mitkImageSource.h"

namespace mitk {
//##Documentation
//## @brief Reader to read image files in vtk file format
//## @ingroup IO
//## @deprecatedSince{2014_10} Use mitk::IOUtils or mitk::FileReaderRegistry instead.
class DEPRECATED() MitkLegacyIO_EXPORT VtkImageReader : public ImageSource, public FileReader
{
public:
  mitkClassMacro(VtkImageReader, FileReader);

  /** Method for creation through the object factory. */
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  itkSetStringMacro(FileName);
  itkGetStringMacro(FileName);

  itkSetStringMacro(FilePrefix);
  itkGetStringMacro(FilePrefix);

  itkSetStringMacro(FilePattern);
  itkGetStringMacro(FilePattern);

  static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

protected:

   VtkImageReader();

  ~VtkImageReader();

  virtual void GenerateData();

  //##Description
  //## @brief Time when Header was last read
  //itk::TimeStamp m_ReadHeaderTime;

protected:
  std::string m_FileName;

  std::string m_FilePrefix;

  std::string m_FilePattern;
};

} // namespace mitk

#endif /* VtkImageReader_H_HEADER_INCLUDED */
