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


#ifndef __MITK_FILE_SERIES_READER__H_
#define __MITK_FILE_SERIES_READER__H_

#include <MitkLegacyIOExports.h>
#include <mitkCommon.h>
#include <mitkFileReader.h>
#include <vector>
#include <string>

namespace mitk
{

/**
 * Provides a function which generates a list of files from
 * a given prefix and pattern.
 * Subclasses may use this function to load a series of files.
 *
 * @deprecatedSince{2014_10}
 */
class MitkLegacyIO_EXPORT FileSeriesReader : public FileReader
{
public:
    mitkClassMacro( FileSeriesReader, FileReader );

    typedef std::vector< std::string > MatchedFileNames;

    virtual MatchedFileNames GetMatchedFileNames( );

protected:

    FileSeriesReader();

    virtual ~FileSeriesReader();

    virtual bool GenerateFileList();

    std::string m_FileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;

    MatchedFileNames m_MatchedFileNames;
};

}

#endif
