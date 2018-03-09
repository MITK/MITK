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

#ifndef __mitkDiffusionImageNrrdReaderService_h
#define __mitkDiffusionImageNrrdReaderService_h

#include "mitkCommon.h"

// MITK includes
#include "mitkImageSource.h"
#include "mitkFileReader.h"

#include <mitkDiffusionPropertyHelper.h>

// ITK includes
#include "itkVectorImage.h"
#include "mitkAbstractFileReader.h"



namespace mitk
{

  /** \brief
  */

  class DiffusionImageNrrdReaderService : public mitk::AbstractFileReader
  {
  public:

    DiffusionImageNrrdReaderService(const DiffusionImageNrrdReaderService & other);
    DiffusionImageNrrdReaderService();
    ~DiffusionImageNrrdReaderService() override;

    using AbstractFileReader::Read;
    std::vector<itk::SmartPointer<BaseData> > Read() override;

    typedef short                                                           DiffusionPixelType;

    typedef mitk::Image                                                     OutputType;
    typedef mitk::DiffusionPropertyHelper::ImageType                        VectorImageType;
    typedef mitk::DiffusionPropertyHelper::GradientDirectionType            GradientDirectionType;
    typedef mitk::DiffusionPropertyHelper::MeasurementFrameType             MeasurementFrameType;
    typedef mitk::DiffusionPropertyHelper::GradientDirectionsContainerType  GradientDirectionContainerType;

  protected:
    OutputType::Pointer m_OutputCache;
    itk::TimeStamp m_CacheTime;

    void InternalRead();

  private:

    DiffusionImageNrrdReaderService* Clone() const override;
    us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
  };

} //namespace MITK

#endif // __mitkDiffusionImageNrrdReaderService_h
