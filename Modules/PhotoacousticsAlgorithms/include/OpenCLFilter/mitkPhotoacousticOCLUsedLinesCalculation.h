/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _MITKPHOTOACOUSTICSOCLUSEDLINESCALCULATION_H_
#define _MITKPHOTOACOUSTICSOCLUSEDLINESCALCULATION_H_

#if defined(PHOTOACOUSTICS_USE_GPU) || DOXYGEN

#include "mitkOclDataSetToDataSetFilter.h"
#include <itkObject.h>
#include "mitkBeamformingSettings.h"

namespace mitk
{
  /*!
  * \brief Class implementing a mitk::OclDataSetToDataSetFilter to calculate which lines each sample should use when beamforming.
  *
  *  The class must be given a configuration class instance of mitk::BeamformingSettings for beamforming parameters through mitk::OCLDelayCalculation::SetConfig(BeamformingSettings conf)
  */

  class OCLUsedLinesCalculation : public OclDataSetToDataSetFilter, public itk::Object
  {
  public:
    mitkClassMacroItkParent(OCLUsedLinesCalculation, itk::Object);
    mitkNewMacro1Param(Self, mitk::BeamformingSettings::Pointer);

    void Update();

    void SetElementHeightsBuffer(cl_mem elementHeightsBuffer);
    void SetElementPositionsBuffer(cl_mem elementPositionsBuffer);

  protected:

    /** Constructor */
    OCLUsedLinesCalculation(mitk::BeamformingSettings::Pointer settings);

    /** Destructor */
    virtual ~OCLUsedLinesCalculation();

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
    float m_part;
    size_t m_ChunkSize[3];
    cl_mem m_ElementHeightsBuffer;
    cl_mem m_ElementPositionsBuffer;
  };
}
#endif
#endif
