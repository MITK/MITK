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
#ifndef __mitkToFCameraMESASR4000Controller_h
#define __mitkToFCameraMESASR4000Controller_h

#include "mitkMESASR4000ModuleExports.h"
#include "mitkCommon.h"
#include "mitkToFCameraMESAController.h"

#include "itkObject.h"
#include "itkObjectFactory.h"

namespace mitk
{
  /**
  * @brief Interface to the Time-of-Flight (ToF) camera MESA Swissranger 4000
  *
  *
  * @ingroup ToFHardware
  */
  class MITK_MESASR4000MODULE_EXPORT ToFCameraMESASR4000Controller : public mitk::ToFCameraMESAController
  {
  public:

    mitkClassMacro( ToFCameraMESASR4000Controller , mitk::ToFCameraMESAController );

    itkNewMacro( Self );

    /*!
    \brief opens a connection to the ToF camera and initializes the hardware specific members
    \return returns whether the connection was successful (true) or not (false)
    */
    virtual bool OpenCameraConnection();
    /*!
    \brief Returns the currently set modulation frequency.
    \return modulation frequency
    */
    virtual int GetModulationFrequency();
    /*!
    \brief Sets the modulation frequency of the ToF device.
    The method automatically calculates a valid value from the given frequency to
    make sure that only valid frequencies are used.
    \param modulationFrequency modulation frequency
    \return frequency set after validation step
    */
    virtual int SetModulationFrequency(unsigned int modulationFrequency);
    /*!
    \brief Returns the currently set integration time.
    \return integration time
    */
    virtual int GetIntegrationTime();
    /*!
    \brief Sets the integration time of the ToF device.
    The method automatically calculates a valid value from the given integration time to
    make sure that only valid times are used.
    \param integrationTime integration time
    \return integration time set after validation step
    */
    virtual int SetIntegrationTime(unsigned int integrationTime);
    /*!
    \brief Sets the acquire mode of the MESA SR4000 camera.
    The method sets some parameter supported by the MESA SR4000, see MESA API for more details.
    \param fpn turns on fix pattern noise correction this should always be enabled for good distance measurement
    */
    virtual void SetFPN( bool fpn );
    /*!
    \brief Sets the acquire mode of the MESA SR4000 camera.
    The method sets some parameter supported by the MESA SR4000, see MESA API for more details.
    \param convGray Converts the amplitude image by multiplying by the square of the distance, resulting in image more like a conventional camera
    */
    virtual void SetConvGray( bool convGray );
    /*!
    \brief Sets the acquire mode of the MESA SR4000 camera.
    The method sets some parameter supported by the MESA SR4000, see MESA API for more details.
    \param median turns on a 3x3 median filter
    */
    virtual void SetMedian( bool median );
    /*!
    \brief Sets the acquire mode of the MESA SR4000 camera.
    The method sets some parameter supported by the MESA SR4000, see MESA API for more details.
    \param anf Turns on the 5x5 hardware adaptive neighborhood filter
    */
    virtual void SetANF( bool anf );

  protected:

    ToFCameraMESASR4000Controller();

    ~ToFCameraMESASR4000Controller();

  private:

    float *m_Lambda;

    float *m_Frequency;

  };
} //END mitk namespace
#endif
