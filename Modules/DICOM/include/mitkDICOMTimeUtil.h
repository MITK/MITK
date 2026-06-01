/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMTimeUtil_h
#define mitkDICOMTimeUtil_h

#include <chrono>

#include <MitkDICOMExports.h>

/* Forward declaration of a DCMTK class. The full definition is only needed in
   the implementation. */
class OFDateTime;

namespace mitk
{
  /**
   * \brief Convert a DCMTK OFDateTime to a std::chrono time_point.
   *
   * The returned time_point has millisecond precision, matching DCMTK's native
   * granularity for OFTime. No range clamping or year adjustment is performed;
   * any year accepted by std::chrono::year is forwarded as-is.
   */
  MITKDICOM_EXPORT std::chrono::sys_time<std::chrono::milliseconds>
    ConvertOFDateTimeToTimePoint(const OFDateTime& time);

  /**
   * \brief Compute the duration from \a start to \a stop in milliseconds.
   *
   * Returned as a double so callers can capture the fractional part below
   * the integer-millisecond boundary. For DCMTK's ms-granular OFDateTime
   * input this is bit-equivalent to the previous Boost.DateTime
   * implementation.
   */
  MITKDICOM_EXPORT double
    ComputeMiliSecDuration(const OFDateTime& start, const OFDateTime& stop);
}

#endif
