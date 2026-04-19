/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFileReader_h
#define mitkFileReader_h

#include <MitkCoreExports.h>
#include <mitkCommon.h>

namespace mitk
{
  /** \brief Interface class of readers that read from files.
   * \ingroup DeprecatedIO
   */
  class MITKCORE_EXPORT FileReader
  {
  public:
    mitkClassMacroNoParent(FileReader)

      /** \brief Get the specified file to load.
       *
       * Either the FileName or FilePrefix plus FilePattern are used to read.
       */
      virtual const char *GetFileName() const = 0;

    /** \brief Specify the file to load.
     *
     * Either the FileName or FilePrefix plus FilePattern are used to read.
     *
     * \param aFileName Path to the file.
     */
    virtual void SetFileName(const char *aFileName) = 0;

    /** \brief Get the specified file prefix for the file(s) to load.
     *
     * You should specify either a FileName or FilePrefix. Use FilePrefix if
     * the data is stored in multiple files.
     */
    virtual const char *GetFilePrefix() const = 0;

    /** \brief Specify file prefix for the file(s) to load.
     *
     * You should specify either a FileName or FilePrefix. Use FilePrefix if
     * the data is stored in multiple files.
     *
     * \param aFilePrefix The file prefix string.
     */
    virtual void SetFilePrefix(const char *aFilePrefix) = 0;

    /** \brief Get the specified file pattern for the file(s) to load.
     *
     * The sprintf format used to build filename from FilePrefix and number.
     * You should specify either a FileName or FilePrefix. Use FilePrefix if
     * the data is stored in multiple files.
     */
    virtual const char *GetFilePattern() const = 0;

    /** \brief Specify file pattern for the file(s) to load.
     *
     * The sprintf format used to build filename from FilePrefix and number.
     * You should specify either a FileName or FilePrefix. Use FilePrefix if
     * the data is stored in multiple files.
     *
     * \param aFilePattern The file pattern string.
     */
    virtual void SetFilePattern(const char *aFilePattern) = 0;

    /** \brief Check whether the file reader can read from a memory buffer.
     * \return True if reading from memory is supported.
     */
    virtual bool CanReadFromMemory();

    /** \brief Set whether the file reader should read from a memory buffer.
     * \param read If true, reading will use the memory buffer instead of a file.
     */
    virtual void SetReadFromMemory(bool read);

    /** \brief Get whether the file reader is set to read from a memory buffer.
     * \return True if reading from memory is enabled.
     */
    virtual bool GetReadFromMemory();

    /** \brief Set the memory buffer and its size for reading.
     *
     * To be used along with a call of SetReadFromMemory(true). This sets
     * the memory buffer and the size from which the reader will read.
     *
     * \param dataArray Pointer to the memory buffer containing the data.
     * \param size Size of the memory buffer in bytes.
     */
    virtual void SetMemoryBuffer(const char *dataArray, unsigned int size);

  protected:
    FileReader();
    virtual ~FileReader();

    bool m_CanReadFromMemory;
    bool m_ReadFromMemory;

    const char *m_MemoryBuffer;
    unsigned int m_MemorySize;

  public:
  protected:
  };
} // namespace mitk
#endif
