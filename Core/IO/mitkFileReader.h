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


#ifndef FILEREADER_H_HEADER_INCLUDED_C1E7E521
#define FILEREADER_H_HEADER_INCLUDED_C1E7E521

#include "mitkCommon.h"

namespace mitk {

//##Documentation
//## @brief Interface class of readers that read from files
//## @ingroup Process
class MITK_CORE_EXPORT FileReader
{
  public:
    //##Documentation
    //## @brief Get the specified the file to load.
    //## 
    //## Either the FileName or FilePrefix plus FilePattern are used to read.
    virtual const char* GetFileName() const = 0;

    //##Documentation
    //## @brief Specify the file to load.
    //## 
    //## Either the FileName or FilePrefix plus FilePattern are used to read.
    virtual void SetFileName(const char* aFileName) = 0;

    //##Documentation
    //## @brief Get the specified file prefix for the file(s) to load. 
    //## 
    //## You should specify either a FileName or FilePrefix. Use FilePrefix if
    //## the data is stored in multiple files.
    virtual const char* GetFilePrefix() const = 0;

    //##Documentation
    //## @brief Specify file prefix for the file(s) to load.
    //## 
    //## You should specify either a FileName or FilePrefix. Use FilePrefix if
    //## the data is stored in multiple files.
    virtual void SetFilePrefix(const char* aFilePrefix) = 0;

    //##Documentation
    //## @brief Get the specified file pattern for the file(s) to load. The
    //## sprintf format used to build filename from FilePrefix and number.
    //## 
    //## You should specify either a FileName or FilePrefix. Use FilePrefix if
    //## the data is stored in multiple files.
    virtual const char* GetFilePattern() const = 0;

    //##Documentation
    //## @brief Specified file pattern for the file(s) to load. The sprintf
    //## format used to build filename from FilePrefix and number.
    //## 
    //## You should specify either a FileName or FilePrefix. Use FilePrefix if
    //## the data is stored in multiple files.
    virtual void SetFilePattern(const char* aFilePattern) = 0;

protected:
    FileReader();

    virtual ~FileReader();
public:

protected:
};
} // namespace mitk
#endif /* FILEREADER_H_HEADER_INCLUDED_C1E7E521 */


