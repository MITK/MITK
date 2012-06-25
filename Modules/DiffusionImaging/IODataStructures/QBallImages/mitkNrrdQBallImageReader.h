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

#ifndef __mitkNrrdQBallImageReader_h
#define __mitkNrrdQBallImageReader_h

#include "mitkCommon.h"
#include "itkVectorContainer.h"
#include "mitkFileReader.h"
#include "vnl/vnl_vector_fixed.h"
#include "mitkQBallImage.h"
#include "mitkQBallImageSource.h"
#include "itkVectorImage.h"
#include <MitkDiffusionImagingExports.h>

namespace mitk
{

  /** \brief
  */

  class MitkDiffusionImaging_EXPORT NrrdQBallImageReader : public mitk::QBallImageSource, public FileReader
  {
  public:

    typedef mitk::QBallImage OutputType;
    typedef mitk::QBallImageSource QBImgSourceType;

    mitkClassMacro( NrrdQBallImageReader, QBImgSourceType );
    itkNewMacro(Self);

    const char* GetFileName() const;
    void SetFileName(const char* aFileName);
    const char* GetFilePrefix() const;
    void SetFilePrefix(const char* aFilePrefix);
    const char* GetFilePattern() const;
    void SetFilePattern(const char* aFilePattern);

    static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

  protected:

    /** Does the real work. */
    virtual void GenerateData();
    virtual void GenerateOutputInformation();

    std::string m_FileName;
    std::string m_FilePrefix;
    std::string m_FilePattern;

  private:
    void operator=(const Self&); //purposely not implemented
  };

} //namespace MITK

#endif // __mitkNrrdQBallImageReader_h
