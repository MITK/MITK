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
#include "mitkPhotoacousticBeamformingSettings.h"

namespace mitk
{
  //##Documentation
  //## @brief
  //## @ingroup Process

  class OCLUsedLinesCalculation : public OclDataSetToDataSetFilter, public itk::Object
  {

  public:
    mitkClassMacroItkParent(OCLUsedLinesCalculation, itk::Object);
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

  protected:

    /** Constructor */
    OCLUsedLinesCalculation();

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

    BeamformingSettings m_Conf;
    float m_part;
  };
}
#endif
#endif
