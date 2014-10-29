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


#ifndef FILEWRITER_H_HEADER_INCLUDED
#define FILEWRITER_H_HEADER_INCLUDED

#include <MitkCoreExports.h>
#include <itkProcessObject.h>
#include <mitkDataNode.h>

namespace mitk {

//##Documentation
//## @brief Interface class of writers that write data to files
//## @ingroup Process
//## @deprecatedSince{2014_10} Use mitk::IFileWriter instead.
class MITK_CORE_EXPORT FileWriter : public itk::ProcessObject
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
    //## @brief Checks if given extension is valid for file writer
    bool IsExtensionValid(std::string extension);

    //##Documentation
    //## @brief Return the possible file extensions for the data type associated with the writer
    virtual std::vector<std::string> GetPossibleFileExtensions() = 0;

    //##Documentation
    //## @brief possible file extensions for the data type associated with the writer as string
     virtual std::string GetPossibleFileExtensionsAsString();

    //##Documentation
    //## @brief Check if the Writer can write this type of data of the
    //## DataTreenode.
    virtual bool CanWriteDataType( DataNode* );

    //##Documentation
    //## @brief Return the MimeType of the saved File.
    virtual std::string GetWritenMIMEType();

    virtual std::string GetSupportedBaseData() const = 0;

    using ProcessObject::SetInput;
    void SetInput( BaseData* data );

    virtual void Write() = 0;

    /**
    @brief Specifies, whether the file writer also can
    write a file to a memory buffer */
    virtual bool CanWriteToMemory(  );

    /**
    @brief Set/Get functions to advise the file writer to
    use tis internal memory array as file writing destination*/
    virtual void SetWriteToMemory( bool write );
    virtual bool GetWriteToMemory(  );

    /**
    @brief To be used along with a call of SetWriteToMemory(true). This returns
    the memory buffer where the file was written.*/
    virtual const char*  GetMemoryPointer();

    /**
    @brief To be used along with a call of SetWriteToMemory(true). This returns
    the size of the  memory buffer where the file was written.*/
    virtual unsigned int GetMemorySize();

    /**
    @brief CAUTION: It's up to the user to call this function to release the
    memory buffer after use in case the file writer has written to its memory array.*/
    virtual void         ReleaseMemory();

protected:
  FileWriter();
  virtual ~FileWriter();

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
#endif /* FILEWRITER_H_HEADER_INCLUDED */
