/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFileSeriesReader_h
#define mitkFileSeriesReader_h

#include <MitkLegacyIOExports.h>
#include <mitkCommon.h>
#include <mitkFileReader.h>
#include <string>
#include <vector>

namespace mitk
{
  /**
   * \brief Base class for readers that load a series of numbered files.
   *
   * Provides functionality to generate a sorted list of files from a directory
   * based on a filename pattern. Given a sample filename, it extracts the numeric
   * portion, prefix, and extension, then scans the directory for all files matching
   * the same prefix and extension. The matched files are sorted by their numeric
   * index. Subclasses may use GenerateFileList() to obtain the sorted file list and
   * then load each file in sequence.
   *
   * \ingroup MitkLegacyIOModule
   * \deprecatedSince{2014_10}
   * \sa mitk::FileReader
   */
  class MITKLEGACYIO_EXPORT FileSeriesReader : public FileReader
  {
  public:
    mitkClassMacro(FileSeriesReader, FileReader);

    /** \brief Type for a vector of matched file names (absolute paths). */
    typedef std::vector<std::string> MatchedFileNames;

    /**
     * \brief Get the list of matched file names.
     *
     * Returns the list of file names that were found during the last call to
     * GenerateFileList(). The list is sorted by the numeric index extracted
     * from the filenames.
     *
     * \return A vector of absolute file paths matching the file series pattern.
     * \pre GenerateFileList() must have been called first.
     */
    virtual MatchedFileNames GetMatchedFileNames();

  protected:
    FileSeriesReader();

    ~FileSeriesReader() override;

    /**
     * \brief Generate a sorted list of files belonging to a series.
     *
     * Parses the filename set in m_FileName to extract a prefix, numeric index, and extension.
     * Then scans the directory for all files matching the same prefix and extension, sorts them
     * by their numeric index, and stores the result in m_MatchedFileNames.
     *
     * \return \c true if at least one matching file was found; \c false otherwise.
     * \throw itk::ImageFileReaderException If m_FileName is empty or the filename
     *        components have inconsistent lengths.
     */
    virtual bool GenerateFileList();

    std::string m_FileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;

    MatchedFileNames m_MatchedFileNames;
  };
}

#endif
