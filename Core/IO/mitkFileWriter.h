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


#ifndef FILEWRITER_H_HEADER_INCLUDED
#define FILEWRITER_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include <itkProcessObject.h>
#include <mitkDataTreeNode.h>

namespace mitk {

//##Documentation
//## @brief Interface class of writers that write data to files
//## @ingroup Process
class FileWriter : public itk::ProcessObject
{
  public:
    mitkClassMacro(FileWriter,itk::ProcessObject);
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

    //##Documentation
    //## @brief Return the extension to be added to the filename.
    virtual std::string GetFileExtension();

    //##Documentation
    //## @brief Check if the Writer can write the Content of the 
    //## DataTreenode.
    virtual bool CanWrite( DataTreeNode* );

    //##Documentation
    //## @brief Return the MimeType of the saved File.
    virtual std::string GetWritenMIMEType();

    //##Documentation
    //## @brief Set the DataTreenode as Input. Important: The Writer
    //## always have a SetInput-Function.
    virtual void SetInput( DataTreeNode* );

    virtual void Write() = 0;

protected:

    FileWriter();

    virtual ~FileWriter();
};

#define mitkWriterMacro                                                       \
virtual void Write()                                                          \
{                                                                             \
  if ( this->GetInput() == NULL )                                             \
    {                                                                         \
    itkExceptionMacro(<<"Write:Please specify an input!");                    \
    return;                                                                   \
    }                                                                         \
  /* Fill in image information.*/                                             \
  this->UpdateOutputInformation();                                            \
  (*(this->GetInputs().begin()))->SetRequestedRegionToLargestPossibleRegion();\
  this->PropagateRequestedRegion(NULL);                                       \
  this->UpdateOutputData(NULL);                                               \
}                                                                             \
virtual void Update()                                                         \
{                                                                             \
  Write();                                                                    \
}                                                                           

} // namespace mitk
#endif /* FILEWRITER_H_HEADER_INCLUDED */
