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


#ifndef VtiFileReader_H_HEADER_INCLUDED
#define VtiFileReader_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkFileReader.h"
#include "mitkImageSource.h"

namespace mitk {
//##Documentation
//## @brief Reader to read image files in vtk file format
//## @ingroup IO
class MITK_CORE_EXPORT VtiFileReader : public ImageSource, public FileReader
{
public:
  mitkClassMacro(VtiFileReader, FileReader);
  
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

   VtiFileReader();
  
  ~VtiFileReader();

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

#endif /* VtiFileReader_H_HEADER_INCLUDED */
