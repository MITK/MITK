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

#ifndef __mitkNrrdDiffusionImageReader_h
#define __mitkNrrdDiffusionImageReader_h

#include "mitkCommon.h"
#include "itkVectorContainer.h"
#include "mitkFileReader.h"
#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix_fixed.h"

#include "itkVectorImage.h"
#include "mitkAbstractFileReader.h"
#include "mitkDiffusionImage.h"

namespace mitk
{

  /** \brief
  */

  class NrrdDiffusionImageReader : public mitk::AbstractFileReader
  {
  public:

    NrrdDiffusionImageReader(const NrrdDiffusionImageReader & other);
    NrrdDiffusionImageReader();
    virtual ~NrrdDiffusionImageReader();

    using AbstractFileReader::Read;
    virtual std::vector<itk::SmartPointer<BaseData> > Read();

    typedef mitk::DiffusionImage<short>                                 OutputType;
    typedef itk::VectorImage<short,3>                                   ImageType;
    typedef vnl_vector_fixed< double, 3 >                               GradientDirectionType;
    typedef vnl_matrix_fixed< double, 3, 3 >                            MeasurementFrameType;
    typedef itk::VectorContainer< unsigned int,GradientDirectionType >  GradientDirectionContainerType;

  protected:
    OutputType::Pointer m_OutputCache;
    itk::TimeStamp m_CacheTime;
    GradientDirectionContainerType::Pointer m_OriginalDiffusionVectors;
    GradientDirectionContainerType::Pointer m_DiffusionVectors;
    float m_B_Value;
    MeasurementFrameType m_MeasurementFrame;

    void InternalRead();

  private:

    NrrdDiffusionImageReader* Clone() const;
    us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
  };

} //namespace MITK


#endif // __mitkNrrdDiffusionImageReader_h
