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


#ifndef mitkSUVCalculationHelper_h
#define mitkSUVCalculationHelper_h

#include <map>
#include <string>
#include <vector>

#include <mitkException.h>
#include <mitkExceptionMacro.h>
#include <mitkSlicedData.h>

#include <MitkPETExports.h>

namespace mitk
{
  class IPropertyProvider;

  /**
   * \brief Base class for failures originating from the SUV calculation helpers.
   *
   * Callers can catch this base type to react to any helper failure, or one of
   * the specialized derived classes to differentiate by error category.
   */
  class MITKPET_EXPORT SUVHelperException : public mitk::Exception
  {
  public:
    mitkExceptionClassMacro(SUVHelperException, mitk::Exception);
  };

  /**
   * \brief A required DICOM property is missing on the input data.
   *
   * The exception message identifies the offending tag in human-readable form
   * (e.g. "(0054,1102) Decay Correction"). Callers that need to react to
   * "missing tag" specifically can catch this type rather than parsing message text.
   */
  class MITKPET_EXPORT MissingDICOMPropertyException : public SUVHelperException
  {
  public:
    mitkExceptionClassMacro(MissingDICOMPropertyException, SUVHelperException);
  };

  /**
   * \brief A DICOM property is present but holds a value the helper does not understand.
   *
   * Raised, e.g., when (0054,1102) Decay Correction is something other than
   * ADMIN, START, or NONE. The exception message contains the tag and the
   * actual unrecognized value.
   */
  class MITKPET_EXPORT InvalidDICOMPropertyValueException : public SUVHelperException
  {
  public:
    mitkExceptionClassMacro(InvalidDICOMPropertyValueException, SUVHelperException);
  };

  /**
   * \brief Acquisition / radiopharmaceutical-injection timing cannot be reconciled.
   *
   * Raised in two situations:
   *   - Only (0018,1072) Radiopharmaceutical Start Time is present (no DateTime),
   *     the resulting decay duration is negative, and even after a single 24 h
   *     rollover correction it remains outside [0, 24 h].
   *   - (0018,1078) Radiopharmaceutical Start DateTime is present but yields a
   *     negative decay duration. The DateTime is unambiguous, so no silent
   *     rollover correction is applied; the situation is surfaced as an error.
   *
   * The exception message recommends re-exporting with (0018,1078) where applicable.
   */
  class MITKPET_EXPORT AmbiguousDecayTimingException : public SUVHelperException
  {
  public:
    mitkExceptionClassMacro(AmbiguousDecayTimingException, SUVHelperException);
  };

  /**
   * \brief Strategy describing how (or whether) the input pixel data has been
   *        decay-corrected by the scanner.
   *
   * Mirrors the values defined for DICOM tag (0054,1102) Decay Correction.
   * Determines which reference time the helper must use when computing the
   * residual decay correction needed for SUV.
   */
  enum class DecayCorrectionStrategy
  {
    /** Pixel data is already decay-corrected to the radiopharmaceutical
     *  administration time. No further correction is needed. */
    Admin,
    /** Pixel data is decay-corrected to the Series Time (DICOM (0008,0031)). */
    Start,
    /** Pixel data is not decay-corrected. The residual correction uses the
     *  per-slice acquisition date and time. */
    None
  };

  /**
   * \brief Map storing the residual decay time in seconds per slice.
   *
   * The key is the slice index (z-index).
   */
  typedef std::map<mitk::SlicedData::IndexValueType, double> DecayTimeSliceMapType;

  /**
   * \brief Map storing per-time-step decay time slice maps.
   *
   * The outer key is the time step, the inner map stores decay times per slice index.
   */
  typedef std::map<mitk::TimeStepType, DecayTimeSliceMapType> DecayTimeMapType;

  /**
   * \brief Result of DeduceDecayCorrection().
   *
   * Bundles the detected DICOM decay-correction strategy together with the
   * decay-time map that the SUV functor should consume per (timestep, slice).
   *
   * The semantics of \c decayTimes depend on \c strategy:
   *   - Admin: every entry is 0.0; the SUV decay term reduces to 1 (2^0).
   *   - Start: every (timestep, slice) entry holds the same value
   *            (SeriesTime - InjectionDateTime in seconds).
   *   - None:  per-slice values (AcquisitionDateTime - InjectionDateTime).
   */
  struct MITKPET_EXPORT DecayCorrectionInfo
  {
    DecayCorrectionStrategy strategy;
    DecayTimeMapType        decayTimes;
  };

  /**
   * \brief Get the radionuclide half-life values from DICOM properties.
   *
   * Extracts the radionuclide half-life from the Radiopharmaceutical Information
   * Sequence stored in the DICOM properties of the passed provider. Reads from
   * DICOM path (0054,0016)[*](0018,1075).
   *
   * If the Radiopharmaceutical Information Sequence contains more than one item,
   * the half-life of all sequence items is returned. The order of results matches
   * the order of the sequence items.
   *
   * \param[in] provider Source of DICOM properties; typically the BaseData of a
   *            PET image.
   * \return A vector of half-life values in seconds. Empty if no appropriate
   *         DICOM element was found or if \p provider is \c nullptr.
   *
   * \sa GetRadionuclideTotalDose, GetRadionuclideNames
   */
  std::vector<double> MITKPET_EXPORT GetRadionuclideHalfLife(const mitk::IPropertyProvider* provider);

  /**
   * \brief Get the radionuclide names from DICOM properties.
   *
   * Extracts the radionuclide code meaning from the Radiopharmaceutical Information
   * Sequence stored in the DICOM properties of the passed provider. Reads from
   * DICOM path (0054,0016)[*](0054,0300)[*](0008,0104).
   *
   * \param[in] provider Source of DICOM properties.
   * \return A space-separated string of radionuclide names. Empty if no appropriate
   *         DICOM element was found or if \p provider is \c nullptr.
   *
   * \sa GetRadionuclideHalfLife
   */
  std::string MITKPET_EXPORT GetRadionuclideNames(const mitk::IPropertyProvider* provider);

  /**
   * \brief Get the radionuclide total dose (injected dose) from DICOM properties.
   *
   * Extracts the radionuclide total dose in [Bq] from the Radiopharmaceutical
   * Information Sequence stored in the DICOM properties of the passed provider.
   * Reads from DICOM path (0054,0016)[*](0018,1074).
   *
   * If the Radiopharmaceutical Information Sequence contains more than one item,
   * the total dose of all sequence items is returned. The order of results matches
   * the order of the sequence items.
   *
   * \param[in] provider Source of DICOM properties.
   * \return A vector of total dose values in [Bq]. Empty if no appropriate
   *         DICOM element was found or if \p provider is \c nullptr.
   *
   * \sa GetRadionuclideHalfLife
   */
  std::vector<double> MITKPET_EXPORT GetRadionuclideTotalDose(const mitk::IPropertyProvider* provider);

  /**
   * \brief Get the patient's weight from DICOM properties.
   *
   * Extracts the patient weight from DICOM tag (0010,1030) stored in the
   * properties of the passed provider.
   *
   * \param[in] provider Source of DICOM properties.
   * \return The patient's weight in [kg].
   * \pre \p provider must point to a valid instance.
   * \pre \p provider must contain a DICOM patient weight property.
   * \throw MissingDICOMPropertyException if \p provider is \c nullptr or contains
   *        no patient-weight property.
   */
  double MITKPET_EXPORT GetPatientsWeight(const mitk::IPropertyProvider* provider);

  /**
   * \brief Detect the DICOM Decay Correction strategy of the input data.
   *
   * Reads DICOM tag (0054,1102) Decay Correction. The value is matched
   * case-insensitively after trimming surrounding whitespace.
   *
   * \param[in] provider Source of DICOM properties.
   * \return The detected DecayCorrectionStrategy.
   * \throw MissingDICOMPropertyException if (0054,1102) is not present.
   * \throw InvalidDICOMPropertyValueException if (0054,1102) is present but
   *        holds a value other than ADMIN, START, or NONE.
   *
   * \sa DeduceDecayCorrection
   */
  DecayCorrectionStrategy MITKPET_EXPORT GetDecayCorrectionStrategy(const mitk::IPropertyProvider* provider);

  /**
   * \brief Deduce the residual decay correction needed to compute SUV.
   *
   * Reads (0054,1102) Decay Correction and produces a DecayCorrectionInfo whose
   * \c decayTimes map can be plugged into the SUV functor unchanged. The
   * residual decay term then accounts only for what the scanner did NOT already
   * apply:
   *   - Admin: pixel data is already decay-corrected to the administration
   *     time; the helper fills the map with 0.0 for every (timestep, slice),
   *     so the SUV decay term collapses to 2^0 = 1.
   *   - Start: pixel data is decay-corrected to Series Time; the helper
   *     computes one decay duration (SeriesTime - InjectionDateTime) and
   *     uses that same value for every (timestep, slice).
   *   - None:  pixel data is not decay-corrected; the helper computes a
   *     per-slice (AcquisitionDateTime - InjectionDateTime) decay duration.
   *
   * The radiopharmaceutical injection time is read from
   * (0054,0016)[*](0018,1078) Radiopharmaceutical Start DateTime if present,
   * otherwise from (0054,0016)[*](0018,1072) Radiopharmaceutical Start Time
   * (in which case the acquisition date is used to assemble a complete
   * timestamp, with a 24 h rollover correction if the resulting decay would
   * be negative).
   *
   * \param[in] data The input data. Must be a valid SlicedData instance whose
   *            time geometry and DICOM properties are populated.
   * \return The strategy plus a fully populated decay-time map.
   * \throw mitk::Exception if \p data is \c nullptr.
   * \throw MissingDICOMPropertyException if a DICOM property required for the
   *        detected strategy is missing.
   * \throw InvalidDICOMPropertyValueException if (0054,1102) holds an
   *        unsupported value, or if a tag value cannot be parsed.
   * \throw AmbiguousDecayTimingException if the radiopharmaceutical injection
   *        time and the acquisition / series reference time cannot be
   *        reconciled into a non-negative decay duration within 24 h.
   *
   * \sa GetDecayCorrectionStrategy, computeSUVbwScaleFactor, SUVbwFunctorPolicy
   */
  DecayCorrectionInfo MITKPET_EXPORT DeduceDecayCorrection(const mitk::SlicedData* data);

}

#endif
