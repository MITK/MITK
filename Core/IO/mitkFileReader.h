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

//##ModelId=3E186F28009B
//##Documentation
//## @brief Interface class of readers that read from files
//## @ingroup Process
class FileReader
{
  public:
    //##ModelId=3EF4A7BC0213
    //##Documentation
    //## @brief Get the specified the file to load.
    //## 
    //## Either the FileName or FilePrefix plus FilePattern are used to read.
    virtual const char* GetFileName() const = 0;

    //##ModelId=3EF4A7F70240
    //##Documentation
    //## @brief Specify the file to load.
    //## 
    //## Either the FileName or FilePrefix plus FilePattern are used to read.
    virtual void SetFileName(const char* aFileName) = 0;

    //##ModelId=3EF4A81800E9
    //##Documentation
    //## @brief Get the specified file prefix for the file(s) to load. 
    //## 
    //## You should specify either a FileName or FilePrefix. Use FilePrefix if
    //## the data is stored in multiple files.
    virtual const char* GetFilePrefix() const = 0;

    //##ModelId=3EF4A826037E
    //##Documentation
    //## @brief Specify file prefix for the file(s) to load.
    //## 
    //## You should specify either a FileName or FilePrefix. Use FilePrefix if
    //## the data is stored in multiple files.
    virtual void SetFilePrefix(const char* aFilePrefix) = 0;

    //##ModelId=3EF4A81E0305
    //##Documentation
    //## @brief Get the specified file pattern for the file(s) to load. The
    //## sprintf format used to build filename from FilePrefix and number.
    //## 
    //## You should specify either a FileName or FilePrefix. Use FilePrefix if
    //## the data is stored in multiple files.
    virtual const char* GetFilePattern() const = 0;

    //##ModelId=3EF4A84E03B8
    //##Documentation
    //## @brief Specified file pattern for the file(s) to load. The sprintf
    //## format used to build filename from FilePrefix and number.
    //## 
    //## You should specify either a FileName or FilePrefix. Use FilePrefix if
    //## the data is stored in multiple files.
    virtual void SetFilePattern(const char* aFilePattern) = 0;

protected:
    //##ModelId=3E188F1F027E
    FileReader();

    //##ModelId=3E188F2800DC
    virtual ~FileReader();
public:

protected:
};
} // namespace mitk
#endif /* FILEREADER_H_HEADER_INCLUDED_C1E7E521 */

