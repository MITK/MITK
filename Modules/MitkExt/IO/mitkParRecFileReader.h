/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef PARRECFILEREADER_H_HEADER_INCLUDED_C1F48A22
#define PARRECFILEREADER_H_HEADER_INCLUDED_C1F48A22

#include "mitkCommon.h"
#include "mitkFileReader.h"
#include "mitkImageSource.h"

namespace mitk {
//##Documentation
//## @brief Reader to read files in Philips PAR/REC file format
//## @ingroup IO
class MITKEXT_CORE_EXPORT ParRecFileReader : public ImageSource, public FileReader
{
public:
  mitkClassMacro(ParRecFileReader, FileReader);
  
  /** Method for creation through the object factory. */
  itkNewMacro(Self);
  
  itkSetStringMacro(FileName);
  itkGetStringMacro(FileName);

  itkSetStringMacro(FilePrefix);
  itkGetStringMacro(FilePrefix);

  itkSetStringMacro(FilePattern);
  itkGetStringMacro(FilePattern);

  static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);
  
protected:
  virtual void GenerateData();
  
  virtual void GenerateOutputInformation();
  
  ParRecFileReader();
  
  ~ParRecFileReader();
  
  //##Description 
  //## @brief Time when Header was last read
  itk::TimeStamp m_ReadHeaderTime;
  
  int m_StartFileIndex;
protected:
  std::string m_FileName;
  std::string m_RecFileName;
  
  std::string m_FilePrefix;
  
  std::string m_FilePattern;
};

} // namespace mitk

#endif /* PARRECFILEREADER_H_HEADER_INCLUDED_C1F48A22 */


