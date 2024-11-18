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

#include "MitkPETExports.h"

namespace mitk
{
  class BaseData;

  /** Gets the radio nuclide half life stored in the properties of the passed data (DICOM path (0054,0016)[*](0018,1075) in seconds. If no apropriate DICOM element was found the vector is empty.
   Radiopharmaceutical Information Sequence (0x0054,0x0016) containes more then one item, the half life of all sequence items will be returned. The order of the results is
   the same like the order of the sequence items.*/
  std::vector<double> MITKPET_EXPORT GetRadionuclideHalfLife(mitk::BaseData* data);

  /** Gets the name of the used radio nuclides stored in the properties of the passed data (DICOM path (0054,0016)[*](0054,0300)[*](0008,0104). If no apropriate DICOM element was found the string is empty.*/
  std::string MITKPET_EXPORT GetRadionuclideNames(mitk::BaseData* data);

  /** Gets the radio nuclide total dose (injected does) in [Bq] stored in the properties of the passed data (DICOM path (0054,0016)[*](0018,1074) in seconds. If no apropriate DICOM element was found the vector is empty.
  Radiopharmaceutical Information Sequence (0x0054,0x0016) containes more then one item, the total dose of all sequence items will be returned. The order of the results is
  the same like the order of the sequence items.*/
  std::vector<double> MITKPET_EXPORT GetRadionuclideTotalDose(mitk::BaseData* data);

  /** Gets the patient's weight in [kg]. If data is invalid or containes no weight property an exception will be thrown.
   * @pre data must point to a valid instance.
   * @pre data must contain a DICOM patient weight property.*/
  double MITKPET_EXPORT GetPatientsWeight(mitk::BaseData* data);

  /**Map that stores the decay time in [sec] per slice. Key of the map is the slice index (z-index).*/
  typedef std::map<mitk::SlicedData::IndexValueType, double> DecayTimeSliceMapType;
  typedef std::map<mitk::TimeStepType, DecayTimeSliceMapType> DecayTimeMapType;

  /** Helper function that deduces the decay time in [sec] for each slice with the following strategy:\n
   1. Get AcquisitionDate and AcquisitionTime.
   2. Get StartDateTime (if not available StartTime; if just times are available it is assumed that it has the same date)
   3. DecayTime is AcquesitionTime - StartTime
   */
  DecayTimeMapType MITKPET_EXPORT DeduceDecayTime_AcquisitionMinusStartSliceResolved(mitk::BaseData* data);

}

#endif

