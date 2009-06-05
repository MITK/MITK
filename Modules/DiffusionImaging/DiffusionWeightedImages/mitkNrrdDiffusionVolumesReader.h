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
#ifndef __mitkNrrdDiffusionVolumesReader_h
#define __mitkNrrdDiffusionVolumesReader_h

#include "mitkCommon.h"
#include "itkImageSource.h"
#include "itkVectorContainer.h"

namespace mitk
{

  /** \brief 
  */

  template < class TImageType >
  class NrrdDiffusionVolumesReader : public itk::ImageSource< TImageType >
  {
  public:

    typedef itk::ImageSource< TImageType > ImageSourceType;

    typedef vnl_vector_fixed< double, 3 >            GradientDirectionType;
    typedef itk::VectorContainer< unsigned int, 
      GradientDirectionType >                  GradientDirectionContainerType;

    mitkClassMacro( NrrdDiffusionVolumesReader, ImageSourceType );
    itkNewMacro(Self);

    itkGetMacro(FileName, std::string);
    itkSetMacro(FileName, std::string);

    GradientDirectionContainerType::Pointer GetDiffusionVectors()
    { return m_DiffusionVectors; }

    itkGetMacro(B_Value, float);
    itkSetMacro(B_Value, float);

  protected:

    /** Does the real work. */
    virtual void GenerateData();

    std::string m_FileName;

    GradientDirectionContainerType::Pointer m_DiffusionVectors;

    float m_B_Value;

  private:
    void operator=(const Self&); //purposely not implemented
  };

} //namespace MITK

#include "mitkNrrdDiffusionVolumesReader.cpp"

#endif // __mitkNrrdDiffusionVolumesReader_h
