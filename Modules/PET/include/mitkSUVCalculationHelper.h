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

#include <vector>
#include <string>

//MITK
#include <mitkSlicedData.h>

#include <MitkPETExports.h>

namespace mitk
{
  class BaseData;

  /**
   * \brief Get the radionuclide half-life values from DICOM properties.
   *
   * Extracts the radionuclide half-life from the Radiopharmaceutical Information
   * Sequence stored in the DICOM properties of the passed data. Reads from
   * DICOM path (0054,0016)[*](0018,1075).
   *
   * If the Radiopharmaceutical Information Sequence contains more than one item,
   * the half-life of all sequence items is returned. The order of results matches
   * the order of the sequence items.
   *
   * \param[in] data The BaseData object containing DICOM properties to query.
   * \return A vector of half-life values in seconds. Empty if no appropriate
   *         DICOM element was found or if \p data is \c nullptr.
   *
   * \sa GetRadionuclideTotalDose, GetRadionuclideNames
   */
  std::vector<double> MITKPET_EXPORT GetRadionuclideHalfLife(mitk::BaseData* data);

  /**
   * \brief Get the radionuclide names from DICOM properties.
   *
   * Extracts the radionuclide code meaning from the Radiopharmaceutical Information
   * Sequence stored in the DICOM properties of the passed data. Reads from
   * DICOM path (0054,0016)[*](0054,0300)[*](0008,0104).
   *
   * \param[in] data The BaseData object containing DICOM properties to query.
   * \return A space-separated string of radionuclide names. Empty if no appropriate
   *         DICOM element was found or if \p data is \c nullptr.
   *
   * \sa GetRadionuclideHalfLife
   */
  std::string MITKPET_EXPORT GetRadionuclideNames(mitk::BaseData* data);

  /**
   * \brief Get the radionuclide total dose (injected dose) from DICOM properties.
   *
   * Extracts the radionuclide total dose in [Bq] from the Radiopharmaceutical
   * Information Sequence stored in the DICOM properties of the passed data. Reads from
   * DICOM path (0054,0016)[*](0018,1074).
   *
   * If the Radiopharmaceutical Information Sequence contains more than one item,
   * the total dose of all sequence items is returned. The order of results matches
   * the order of the sequence items.
   *
   * \param[in] data The BaseData object containing DICOM properties to query.
   * \return A vector of total dose values in [Bq]. Empty if no appropriate
   *         DICOM element was found or if \p data is \c nullptr.
   *
   * \sa GetRadionuclideHalfLife
   */
  std::vector<double> MITKPET_EXPORT GetRadionuclideTotalDose(mitk::BaseData* data);

  /**
   * \brief Get the patient's weight from DICOM properties.
   *
   * Extracts the patient weight from DICOM tag (0010,1030) stored in the
   * properties of the passed data.
   *
   * \param[in] data The BaseData object containing DICOM properties to query.
   * \return The patient's weight in [kg].
   * \pre \p data must point to a valid instance.
   * \pre \p data must contain a DICOM patient weight property.
   * \throw mitk::Exception if \p data is \c nullptr or contains no weight property.
   */
  double MITKPET_EXPORT GetPatientsWeight(mitk::BaseData* data);

  /**
   * \brief Map storing the decay time in seconds per slice.
   *
   * The key of the map is the slice index (z-index).
   */
  typedef std::map<mitk::SlicedData::IndexValueType, double> DecayTimeSliceMapType;

  /**
   * \brief Map storing per-time-step decay time slice maps.
   *
   * The outer key is the time step, the inner map stores decay times per slice index.
   */
  typedef std::map<mitk::TimeStepType, DecayTimeSliceMapType> DecayTimeMapType;

  /**
   * \brief Deduce the radioactive decay time per slice from DICOM acquisition timestamps.
   *
   * Computes the decay time in seconds for each slice using the following strategy:
   *   -# Retrieve AcquisitionDate (0008,0022) and AcquisitionTime (0008,0032).
   *   -# Retrieve Radiopharmaceutical Start DateTime (0018,1078) or, if unavailable,
   *      Start Time (0018,1072). If only times are available, the acquisition date is assumed.
   *   -# DecayTime = AcquisitionTime - StartTime.
   *
   * \param[in] data The BaseData object containing the relevant DICOM time properties.
   * \return A nested map of decay times [s] indexed by time step and slice index.
   * \throw mitk::Exception if required DICOM time properties are missing or
   *        time string conversion fails.
   *
   * \sa computeSUVbwScaleFactor, SUVbwFunctorPolicy
   */
  DecayTimeMapType MITKPET_EXPORT DeduceDecayTime_AcquisitionMinusStartSliceResolved(mitk::BaseData* data);

}

#endif

