/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __mitkNrrdTensorImageReader_h
#define __mitkNrrdTensorImageReader_h

#include "mitkCommon.h"
#include "itkVectorContainer.h"
#include "mitkFileReader.h"
#include "vnl/vnl_vector_fixed.h"
#include "mitkTensorImage.h"
#include "mitkTensorImageSource.h"
#include "itkVectorImage.h"

namespace mitk
{

  /** \brief 
  */

  class NrrdTensorImageReader : public mitk::TensorImageSource, public FileReader
  {
  public:

    typedef mitk::TensorImage OutputType;
    typedef mitk::TensorImageSource DTImgSourceType;
 
    mitkClassMacro( NrrdTensorImageReader, DTImgSourceType );
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

#endif // __mitkNrrdTensorImageReader_h
