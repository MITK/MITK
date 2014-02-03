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

#ifndef __mitkLabelSetImageReader_h
#define __mitkLabelSetImageReader_h

#include "SegmentationExports.h"
#include "mitkFileReader.h"
#include "mitkLabelSetImage.h"
#include "mitkLabelSetImageSource.h"

#include "itkVectorImage.h"

namespace mitk
{

/**
 * Reads a segmentation session from file
 * @ingroup Process
 */
class Segmentation_EXPORT LabelSetImageReader : public LabelSetImageSource, public FileReader
{
  public:
    typedef LabelSetImage::PixelType            PixelType;
    typedef LabelSetImage::LabelSetImageType    ImageType;
    typedef LabelSetImage::VectorImageType      VectorImageType;
    typedef LabelSetImageSource::OutputType     OutputType;

    mitkClassMacro( LabelSetImageReader, FileReader );
    itkNewMacro(Self);

    const char* GetFileName() const;
    void SetFileName(const char* aFileName);
    const char* GetFilePrefix() const;
    void SetFilePrefix(const char* aFilePrefix);
    const char* GetFilePattern() const;
    void SetFilePattern(const char* aFilePattern);

    static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

  protected:

    virtual void GenerateData();
    virtual void GenerateOutputInformation();

    LabelSetImageReader();
    ~LabelSetImageReader();

    std::string m_FileName;
    std::string m_FilePrefix;
    std::string m_FilePattern;

    itk::TimeStamp m_CacheTime;

  private:
    void operator=(const Self&); //purposely not implemented
  };

} //namespace MITK

#endif // __mitkLabelSetImageReader_h
