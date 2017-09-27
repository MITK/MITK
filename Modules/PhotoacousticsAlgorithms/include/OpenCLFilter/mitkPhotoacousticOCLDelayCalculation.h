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
#include "mitkPhotoacousticBeamformingSettings.h"

namespace mitk
{
  //##Documentation
  //## @brief
  //## @ingroup Process

  class OCLDelayCalculation : public OclDataSetToDataSetFilter, public itk::Object
  {

  public:
    mitkClassMacroItkParent(OCLDelayCalculation, itk::Object);
    itkNewMacro(Self);

    /**
    * @brief SetInput Set the input image. Only 3D images are supported for now.
    * @param image a 3D image.
    * @throw mitk::Exception if the dimesion is not 3.
    */

    /** Update the filter */
    void Update();

    void SetConfig(BeamformingSettings conf)
    {
      m_Conf = conf;
    }

    void SetInputs(cl_mem usedLines)
    {
      m_UsedLines = usedLines;
    }

  protected:

    /** Constructor */
    OCLDelayCalculation();

    /** Destructor */
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

    BeamformingSettings m_Conf;
    cl_mem m_UsedLines;
    unsigned int m_BufferSize;
    float m_DelayMultiplicatorRaw;
    char m_IsPAImage;
  };
}
#endif
#endif