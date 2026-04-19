/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFileWriter_h
#define mitkFileWriter_h

#include <MitkCoreExports.h>
#include <itkProcessObject.h>
#include <mitkDataNode.h>

namespace mitk
{
  /**
   * \brief Interface class of writers that write data to files.
   *
   * \ingroup DeprecatedIO
   *
   * \sa IFileWriter
   */
  class MITKCORE_EXPORT FileWriter : public itk::ProcessObject
  {
  public:
    mitkClassMacroItkParent(FileWriter, itk::ProcessObject);

    /**
     * \brief Get the specified file to write.
     *
     * Either the FileName or FilePrefix plus FilePattern are used to write.
     *
     * \return The file name as a C string.
     */
    virtual const char *GetFileName() const = 0;

    /**
     * \brief Specify the file to write.
     *
     * Either the FileName or FilePrefix plus FilePattern are used to write.
     *
     * \param aFileName The file name to write to.
     */
    virtual void SetFileName(const char *aFileName) = 0;

    /**
     * \brief Get the specified file prefix for the file(s) to write.
     *
     * You should specify either a FileName or FilePrefix. Use FilePrefix if
     * the data is stored in multiple files.
     *
     * \return The file prefix as a C string.
     */
    virtual const char *GetFilePrefix() const = 0;

    /**
     * \brief Specify file prefix for the file(s) to write.
     *
     * You should specify either a FileName or FilePrefix. Use FilePrefix if
     * the data is stored in multiple files.
     *
     * \param aFilePrefix The file prefix to use.
     */
    virtual void SetFilePrefix(const char *aFilePrefix) = 0;

    /**
     * \brief Get the specified file pattern for the file(s) to write.
     *
     * The sprintf format used to build filename from FilePrefix and number.
     * You should specify either a FileName or FilePrefix. Use FilePrefix if
     * the data is stored in multiple files.
     *
     * \return The file pattern as a C string.
     */
    virtual const char *GetFilePattern() const = 0;

    /**
     * \brief Specify file pattern for the file(s) to write.
     *
     * The sprintf format used to build filename from FilePrefix and number.
     * You should specify either a FileName or FilePrefix. Use FilePrefix if
     * the data is stored in multiple files.
     *
     * \param aFilePattern The file pattern to use.
     */
    virtual void SetFilePattern(const char *aFilePattern) = 0;

    /**
     * \brief Return the extension to be added to the filename.
     *
     * \return The default file extension as a string.
     */
    virtual std::string GetFileExtension();

    /**
     * \brief Check if the given extension is valid for this file writer.
     *
     * \param extension The file extension to check.
     * \return \c true if the extension is valid, \c false otherwise.
     */
    bool IsExtensionValid(std::string extension);

    /**
     * \brief Return the possible file extensions for the data type associated with the writer.
     *
     * \return A vector of valid file extension strings.
     */
    virtual std::vector<std::string> GetPossibleFileExtensions() = 0;

    /**
     * \brief Get the possible file extensions as a single formatted string.
     *
     * \return A space-separated string of possible file extensions prefixed with '*'.
     */
    virtual std::string GetPossibleFileExtensionsAsString();

    /**
     * \brief Check if the writer can write the data type of the DataNode.
     *
     * \return \c true if the writer supports the data type, \c false otherwise.
     */
    virtual bool CanWriteDataType(DataNode *);

    /**
     * \brief Return the MIME type of the saved file.
     *
     * \return The MIME type string, or an empty string if not available.
     */
    virtual std::string GetWritenMIMEType();

    /**
     * \brief Get the class name of the supported BaseData type.
     *
     * \return The class name of the data type this writer supports.
     */
    virtual std::string GetSupportedBaseData() const = 0;

    using ProcessObject::SetInput;

    /**
     * \brief Set the input BaseData to be written.
     *
     * \param data The BaseData object to write.
     */
    void SetInput(BaseData *data);

    /**
     * \brief Perform the write operation.
     */
    virtual void Write() = 0;

    /**
     * \brief Check whether the file writer can write to a memory buffer.
     *
     * \return \c true if memory writing is supported, \c false otherwise.
     */
    virtual bool CanWriteToMemory();

    /**
     * \brief Enable or disable writing to an internal memory buffer.
     *
     * \param write If \c true, the writer uses its internal memory array as the destination.
     */
    virtual void SetWriteToMemory(bool write);

    /**
     * \brief Get whether the writer is set to write to memory.
     *
     * \return \c true if writing to memory is enabled, \c false otherwise.
     */
    virtual bool GetWriteToMemory();

    /**
     * \brief Get the memory buffer where the file was written.
     *
     * To be used along with a call of SetWriteToMemory(true).
     *
     * \return Pointer to the internal memory buffer.
     */
    virtual const char *GetMemoryPointer();

    /**
     * \brief Get the size of the memory buffer where the file was written.
     *
     * To be used along with a call of SetWriteToMemory(true).
     *
     * \return Size of the memory buffer in bytes.
     */
    virtual unsigned int GetMemorySize();

    /**
     * \brief Release the internal memory buffer.
     *
     * \note It is the caller's responsibility to call this function to release the
     * memory buffer after use in case the file writer has written to its memory array.
     */
    virtual void ReleaseMemory();

  protected:
    FileWriter();
    ~FileWriter() override;

    bool m_CanWriteToMemory;
    bool m_WriteToMemory;
    char *m_MemoryBuffer;
    unsigned int m_MemoryBufferSize;
  };

#define mitkWriterMacro                                                                                                \
                                                                                                                       \
  virtual void Write() override                                                                                        \
                                                                                                                       \
  {                                                                                                                    \
    if (this->GetInput() == nullptr)                                                                                      \
                                                                                                                       \
    {                                                                                                                  \
      itkExceptionMacro(<< "Write:Please specify an input!");                                                          \
      return;                                                                                                          \
    }                                                                                                                  \
    /* Fill in image information.*/                                                                                    \
    this->UpdateOutputInformation();                                                                                   \
    (*(this->GetInputs().begin()))->SetRequestedRegionToLargestPossibleRegion();                                       \
    this->PropagateRequestedRegion(nullptr);                                                                              \
    this->UpdateOutputData(nullptr);                                                                                      \
  }                                                                                                                    \
                                                                                                                       \
  virtual void Update() override { Write(); }
} // namespace mitk
#endif
