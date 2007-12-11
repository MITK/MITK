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

#ifndef FILEWRITERWITHINFORMATION_H_INCLUDED
#define FILEWRITERWITHINFORMATION_H_INCLUDED

#include "mitkFileWriter.h"
#include "mitkBaseData.h"

namespace mitk {
/**
 * \brief Interface for FileWriters with extra information.
 * Should be merged into FileWriter. 
*/
class FileWriterWithInformation : public FileWriter {
  public:
    mitkClassMacro(FileWriterWithInformation,FileWriter);

    virtual const char * GetDefaultFilename() = 0;
    virtual const char * GetFileDialogPattern() = 0;
    virtual bool CanWrite(BaseData::Pointer data) = 0;  
    virtual void DoWrite(BaseData::Pointer data) = 0;
};
}
#endif

