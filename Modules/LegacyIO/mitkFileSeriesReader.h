/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __MITK_FILE_SERIES_READER__H_
#define __MITK_FILE_SERIES_READER__H_

#include <MitkLegacyIOExports.h>
#include <mitkCommon.h>
#include <mitkFileReader.h>
#include <string>
#include <vector>

namespace mitk
{
  /**
   * Provides a function which generates a list of files from
   * a given prefix and pattern.
   * Subclasses may use this function to load a series of files.
   *
   * @deprecatedSince{2014_10}
   */
  class MITKLEGACYIO_EXPORT FileSeriesReader : public FileReader
  {
  public:
    mitkClassMacro(FileSeriesReader, FileReader);

      typedef std::vector<std::string> MatchedFileNames;

    virtual MatchedFileNames GetMatchedFileNames();

  protected:
    FileSeriesReader();

    ~FileSeriesReader() override;

    virtual bool GenerateFileList();

    std::string m_FileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;

    MatchedFileNames m_MatchedFileNames;
  };
}

#endif
