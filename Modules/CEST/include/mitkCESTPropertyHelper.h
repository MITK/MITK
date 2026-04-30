/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCESTPropertyHelper_h
#define mitkCESTPropertyHelper_h

#include <mitkIPropertyProvider.h>
#include <mitkIPropertyOwner.h>

#include <MitkCESTExports.h>

namespace mitk
{
  /**
  \brief Return the property name for the CEST preparation type ("CEST.PreparationType").
  \return The property name string "CEST.PreparationType".
  */
  const std::string MITKCEST_EXPORT CEST_PROPERTY_NAME_PREPERATIONTYPE();

  /**
  \brief Return the property name for the CEST recovery mode ("CEST.RecoveryMode").
  \return The property name string "CEST.RecoveryMode".
  */
  const std::string MITKCEST_EXPORT CEST_PROPERTY_NAME_RECOVERYMODE();

  /**
  \brief Return the property name for the CEST spoiling type ("CEST.SpoilingType").
  \return The property name string "CEST.SpoilingType".
  */
  const std::string MITKCEST_EXPORT CEST_PROPERTY_NAME_SPOILINGTYPE();

  /**
  \brief Return the property name for the CEST frequency offsets ("CEST.Offsets").

  The offsets property stores space-separated frequency offset values in ppm.
  Offsets with absolute value > 299 indicate M0 normalization images.

  \return The property name string "CEST.Offsets".
  \sa ExtractCESTOffset, CESTImageNormalizationFilter
  */
  const std::string MITKCEST_EXPORT CEST_PROPERTY_NAME_OFFSETS();

  /**
  \brief Return the property name for the CEST T1 recovery times ("CEST.TREC").

  The TREC property stores space-separated T1 recovery time values in milliseconds.

  \return The property name string "CEST.TREC".
  \sa ExtractCESTT1Time
  */
  const std::string MITKCEST_EXPORT CEST_PROPERTY_NAME_TREC();

  /**
  \brief Return the property name for the CEST frequency ("CEST.FREQ").

  The frequency is stored in Hz in the property.

  \return The property name string "CEST.FREQ".
  \sa GetCESTFrequency, SetCESTFrequencyMHz
  */
  const std::string MITKCEST_EXPORT CEST_PROPERTY_NAME_FREQ();

  /**
  \brief Return the property name for the CEST pulse duration ("CEST.PulseDuration").

  The pulse duration is stored in microseconds in the property.

  \return The property name string "CEST.PulseDuration".
  \sa GetCESTPulseDuration
  */
  const std::string MITKCEST_EXPORT CEST_PROPERTY_NAME_PULSEDURATION();

  /**
  \brief Return the property name for the CEST B1 amplitude ("CEST.B1Amplitude").
  \return The property name string "CEST.B1Amplitude".
  \sa GetCESTB1Amplitude
  */
  const std::string MITKCEST_EXPORT CEST_PROPERTY_NAME_B1Amplitude();

  /**
  \brief Return the property name for the CEST duty cycle ("CEST.DutyCycle").

  The duty cycle is stored as a percentage value in the property (e.g., 56 for 56%).

  \return The property name string "CEST.DutyCycle".
  \sa GetCESTDutyCycle
  */
  const std::string MITKCEST_EXPORT CEST_PROPERTY_NAME_DutyCycle();

  /**
  \brief Get the CEST B1 amplitude from a property provider.

  Reads the "CEST.B1Amplitude" property and converts it to a double value.

  \param[in] provider Pointer to the property provider (e.g., an mitk::Image).
  \return The B1 amplitude value.
  \throw mitk::Exception if provider is nullptr or the property does not exist.
  \sa CEST_PROPERTY_NAME_B1Amplitude
  */
  double MITKCEST_EXPORT GetCESTB1Amplitude(const IPropertyProvider* provider);

  /**
  \brief Get the CEST frequency from a property provider in MHz.

  Reads the "CEST.FREQ" property (stored in Hz) and converts it to MHz by
  multiplying with 1e-6.

  \param[in] provider Pointer to the property provider (e.g., an mitk::Image).
  \return The CEST frequency in MHz.
  \throw mitk::Exception if provider is nullptr or the property does not exist.
  \sa CEST_PROPERTY_NAME_FREQ, SetCESTFrequencyMHz
  */
  double MITKCEST_EXPORT GetCESTFrequency(const IPropertyProvider* provider);

  /**
  \brief Set the CEST frequency on a property owner in MHz.

  Converts the value from MHz to Hz (by multiplying with 1e6) and stores it
  in the "CEST.FREQ" property.

  \param[in,out] owner Pointer to the property owner on which to set the property.
                 If nullptr, this function does nothing.
  \param[in] freqInMHz The CEST frequency value in MHz.
  \sa CEST_PROPERTY_NAME_FREQ, GetCESTFrequency
  */
  void MITKCEST_EXPORT SetCESTFrequencyMHz(IPropertyOwner* owner, double freqInMHz);

  /**
  \brief Get the CEST pulse duration from a property provider in seconds.

  Reads the "CEST.PulseDuration" property (stored in microseconds) and converts
  it to seconds by multiplying with 1e-6.

  \param[in] provider Pointer to the property provider (e.g., an mitk::Image).
  \return The CEST pulse duration in seconds.
  \throw mitk::Exception if provider is nullptr or the property does not exist.
  \sa CEST_PROPERTY_NAME_PULSEDURATION
  */
  double MITKCEST_EXPORT GetCESTPulseDuration(const IPropertyProvider* provider);

  /**
  \brief Get the CEST duty cycle from a property provider as a scaling factor.

  Reads the "CEST.DutyCycle" property (stored as a percentage, e.g. 56 for 56%)
  and converts it to a scaling factor by multiplying with 0.01 (e.g., returns 0.56).

  \param[in] provider Pointer to the property provider (e.g., an mitk::Image).
  \return The CEST duty cycle as a scaling factor in the range [0.0, 1.0].
  \throw mitk::Exception if provider is nullptr or the property does not exist.
  \sa CEST_PROPERTY_NAME_DutyCycle
  */
  double MITKCEST_EXPORT GetCESTDutyCycle(const IPropertyProvider* provider);
}

#endif
