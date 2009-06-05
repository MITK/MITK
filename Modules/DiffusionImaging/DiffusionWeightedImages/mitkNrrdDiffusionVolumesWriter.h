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
#ifndef __mitkNrrdDiffusionVolumesWriter_h
#define __mitkNrrdDiffusionVolumesWriter_h

// itk includes
#include "itkObject.h"
#include "itkVectorContainer.h"

// mitk includes
#include "mitkCommon.h"

// vnl includes
#include "vnl/vnl_vector_fixed.h"

namespace mitk
{

  /** \brief 
  */

  template <class TImageType>
  class NrrdDiffusionVolumesWriter : public itk::Object
  {
  public:

    mitkClassMacro( NrrdDiffusionVolumesWriter, itk::Object );
    itkNewMacro(Self);

    typedef vnl_vector_fixed< double, 3 >            GradientDirectionType;
    typedef itk::VectorContainer< unsigned int, 
      GradientDirectionType >                  GradientDirectionContainerType;

    virtual void Update(void);

    typename TImageType::Pointer GetInput()
    { return m_Input; }
    void SetInput( typename TImageType::Pointer input )
    { m_Input = input; }

    GradientDirectionContainerType::Pointer GetDirections()
    { return m_Directions; }
    void SetDirections( GradientDirectionContainerType::Pointer directions )
    { m_Directions = directions; }

    itkGetMacro(B_Value, float);
    itkSetMacro(B_Value, float);

    itkGetMacro(FileName, std::string);
    itkSetMacro(FileName, std::string);

  protected:

    typename TImageType::Pointer m_Input;

    std::string m_FileName;

    /** A list of gradient directions. */
    GradientDirectionContainerType::Pointer m_Directions;

    /** b-value */
    float m_B_Value;

  private:
    void operator=(const Self&); //purposely not implemented
  };

} //namespace MITK

#include "mitkNrrdDiffusionVolumesWriter.cpp"

#endif // __mitkNrrdDiffusionVolumesWriter_h
