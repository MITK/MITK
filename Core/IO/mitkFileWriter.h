/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#ifndef FILEWRITER_H_HEADER_INCLUDED
#define FILEWRITER_H_HEADER_INCLUDED

#include "mitkCommon.h"

namespace mitk {

//##Documentation
//## @brief Interface class of writers that write data to files
//## @ingroup Process
class FileWriter
{
  public:
    //##Documentation
    //## @brief Get the specified the file to write
    //## 
    //## Either the FileName or FilePrefix plus FilePattern are used to write.
    virtual const char* GetFileName() const = 0;

    //##Documentation
    //## @brief Specify the file to write.
    //## 
    //## Either the FileName or FilePrefix plus FilePattern are used to write.
    virtual void SetFileName(const char* aFileName) = 0;

    //##Documentation
    //## @brief Get the specified file prefix for the file(s) to write. 
    //## 
    //## You should specify either a FileName or FilePrefix. Use FilePrefix if
    //## the data is stored in multiple files.
    virtual const char* GetFilePrefix() const = 0;

    //##Documentation
    //## @brief Specify file prefix for the file(s) to write.
    //## 
    //## You should specify either a FileName or FilePrefix. Use FilePrefix if
    //## the data is stored in multiple files.
    virtual void SetFilePrefix(const char* aFilePrefix) = 0;

    //##Documentation
    //## @brief Get the specified file pattern for the file(s) to write. The
    //## sprintf format used to build filename from FilePrefix and number.
    //## 
    //## You should specify either a FileName or FilePrefix. Use FilePrefix if
    //## the data is stored in multiple files.
    virtual const char* GetFilePattern() const = 0;

    //##Documentation
    //## @brief Specified file pattern for the file(s) to write. The sprintf
    //## format used to build filename from FilePrefix and number.
    //## 
    //## You should specify either a FileName or FilePrefix. Use FilePrefix if
    //## the data is stored in multiple files.
    virtual void SetFilePattern(const char* aFilePattern) = 0;

    virtual void Write() = 0;
protected:
    FileWriter();

    virtual ~FileWriter();
};

#define mitkWriterMacro                                                     \
virtual void Write()                                                        \
{                                                                           \
  if ( this->GetInput() == NULL )                                           \
    {                                                                       \
    itkExceptionMacro(<<"Write:Please specify an input!");                  \
    return;                                                                 \
    }                                                                       \
  /* Fill in image information.*/                                           \
  this->UpdateOutputInformation();                                          \
  this->GetInput()->SetRequestedRegionToLargestPossibleRegion();            \
  this->PropagateRequestedRegion(NULL);                                     \
  this->UpdateOutputData(NULL);                                             \
}                                                                           \
virtual void Update()                                                       \
{                                                                           \
  Write();                                                                  \
}                                                                           

} // namespace mitk
#endif /* FILEWRITER_H_HEADER_INCLUDED */

