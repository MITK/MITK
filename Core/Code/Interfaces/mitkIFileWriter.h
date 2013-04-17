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


#ifndef IFileWriter_H_HEADER_INCLUDED
#define IFileWriter_H_HEADER_INCLUDED

#include <MitkExports.h>
#include <itkProcessObject.h>
#include <mitkDataNode.h>

// Microservices
#include <usServiceInterface.h>
#include <usServiceRegistration.h>
#include <usServiceProperties.h>

namespace mitk {

  struct IOInformation {
  public:
    std::string m_HumanReadableInformation;
    std::string m_Extension;
    std::string m_Priority;
    std::map < std::string, std::string > m_Options;
  };

//##Documentation
//## @brief Interface class of writers that write data to files
//## @ingroup Process
class MITK_CORE_EXPORT IFileWriter : public itk::ProcessObject
{
  public:
    mitkClassMacro(IFileWriter,itk::ProcessObject);
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

    std::string GetInformation();

    //##Documentation
    //## @brief Checks if given extension is valid for file writer
    bool IsExtensionValid(std::string extension);

    //##Documentation
    //## @brief Check if the Writer can write this type of data of the
    //## DataTreenode.
    virtual bool CanWriteDataType( DataNode* );

    //##Documentation
    //## @brief Set the DataTreenode as Input. Important: A Writer
    //## always has a SetInput-Function.
    virtual void SetInput( DataNode* );

    virtual void Write() = 0;

protected:
  IFileWriter();
  virtual ~IFileWriter();

  bool   m_CanWriteToMemory;
  bool   m_WriteToMemory;
  char *          m_MemoryBuffer;
  unsigned int    m_MemoryBufferSize;
};

#define mitkWriterMacro                                                       \
virtual void Write()                                                          \
{                                                                             \
  if ( this->GetInput() == NULL )                                             \
{                                                                             \
  itkExceptionMacro(<<"Write:Please specify an input!");                      \
  return;                                                                     \
}                                                                             \
/* Fill in image information.*/                                               \
  this->UpdateOutputInformation();                                            \
  (*(this->GetInputs().begin()))->SetRequestedRegionToLargestPossibleRegion();\
  this->PropagateRequestedRegion(NULL);                                       \
  this->UpdateOutputData(NULL);                                               \
}                                                                             \
                                                                              \
virtual void Update()                                                         \
{                                                                             \
  Write();                                                                    \
}

} // namespace mitk

// This is the microservice declaration. Do not meddle!
US_DECLARE_SERVICE_INTERFACE(mitk::IFileWriter, "org.mitk.services.FileWriter")

#endif /* IFileWriter_H_HEADER_INCLUDED */
