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

#ifndef _MITKPHOTOACOUSTICSDELAYCALC_H_
#define _MITKPHOTOACOUSTICSDELAYCALC_H_

#if defined(PHOTOACOUSTICS_USE_GPU) || DOXYGEN

#include "mitkOclDataSetToDataSetFilter.h"
#include <itkObject.h>
#include "mitkBeamformingSettings.h"

namespace mitk
{
  /*!
  * \brief Class implementing a mitk::OclDataSetToDataSetFilter to calculate the delays used for beamforming.
  *
  *  The class must be given a configuration class instance of mitk::BeamformingSettings for beamforming parameters through mitk::OCLDelayCalculation::SetConfig(BeamformingSettings conf)
  */

  class OCLDelayCalculation : public OclDataSetToDataSetFilter, public itk::Object
  {
  public:
    mitkClassMacroItkParent(OCLDelayCalculation, itk::Object);
    mitkNewMacro1Param(Self, mitk::BeamformingSettings::Pointer);

    void Update();

  protected:

    OCLDelayCalculation(mitk::BeamformingSettings::Pointer settings);
    virtual ~OCLDelayCalculation();

    /** Initialize the filter */
    bool Initialize();

    void Execute();

    mitk::PixelType GetOutputType()
    {
      return mitk::MakeScalarPixelType<unsigned short>();
    }

    int GetBytesPerElem()
    {
      return sizeof(unsigned short);
    }

    virtual us::Module* GetModule();

    int m_sizeThis;

  private:
    /** The OpenCL kernel for the filter */
    cl_kernel m_PixelCalculation;

    BeamformingSettings::Pointer m_Conf;
    unsigned int m_BufferSize;
    float m_DelayMultiplicatorRaw;
    char m_IsPAImage;
    size_t m_ChunkSize[3];
  };
}
#endif
#endif
