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
  };
}
#endif
#endif
