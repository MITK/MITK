/*=========================================================================

Program:   Insight Segmentation & Registration Toolkit
Module:    $RCSfile: itkImageSeriesReader.h,v $
Language:  C++
Date:      $Date: 2007-08-24 13:35:59 $
Version:   $Revision: 1.14 $

Copyright (c) Insight Software Consortium. All rights reserved.
See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __mitkNrrdDiffusionImageReader_h
#define __mitkNrrdDiffusionImageReader_h

#include "mitkCommon.h"
#include "itkVectorContainer.h"
#include "mitkFileReader.h"
#include "vnl/vnl_vector_fixed.h"
#include "mitkDiffusionImageSource.h"
#include "itkVectorImage.h"

namespace mitk
{

  /** \brief 
  */

  template < class TPixelType >
  class NrrdDiffusionImageReader : public mitk::DiffusionImageSource<TPixelType>, public FileReader
  {
  public:

    typedef mitk::DiffusionImage<TPixelType> OutputType;
    typedef itk::VectorImage<TPixelType,3>     ImageType;
    typedef DiffusionImageSource<TPixelType> DiffVolSourceType;
    typedef vnl_vector_fixed< double, 3 >      GradientDirectionType;
    typedef itk::VectorContainer< unsigned int, 
      GradientDirectionType >                  GradientDirectionContainerType;

    mitkClassMacro( NrrdDiffusionImageReader, DiffVolSourceType );
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

    typename OutputType::Pointer m_OutputCache;
    itk::TimeStamp m_CacheTime;

    GradientDirectionContainerType::Pointer m_DiffusionVectors;
    float m_B_Value;

  private:
    void operator=(const Self&); //purposely not implemented
  };

} //namespace MITK

#include "mitkNrrdDiffusionImageReader.cpp"

#endif // __mitkNrrdDiffusionImageReader_h
