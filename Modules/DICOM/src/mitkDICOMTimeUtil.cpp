/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMTimeUtil.h>

#include <dcmtk/dcmdata/dcvrdt.h>

#include <cmath>

namespace mitk
{
  std::chrono::sys_time<std::chrono::milliseconds>
  ConvertOFDateTimeToTimePoint(const OFDateTime& time)
  {
    using namespace std::chrono;

    const year_month_day ymd{
      year{static_cast<int>(time.getDate().getYear())},
      month{static_cast<unsigned>(time.getDate().getMonth())},
      day{static_cast<unsigned>(time.getDate().getDay())}};

    auto tp = sys_days{ymd}
            + hours{time.getTime().getHour()}
            + minutes{time.getTime().getMinute()}
            + seconds{time.getTime().getIntSecond()}
            + milliseconds{time.getTime().getMilliSecond()};

    // A DICOM DT may carry an explicit UTC offset (&ZZXX). When present,
    // normalize the broken-down local time to UTC so a difference taken
    // against another OFDateTime is offset-consistent (e.g. an injection
    // (0018,1078) stamped "+0100" differenced against an offset-less
    // reference time). DCMTK reports an absent offset as "unspecified";
    // in that case leave the value as-is, which keeps the common
    // single-timezone path bit-identical to before (two offset-less
    // stamps still difference correctly).
    if (time.getTime().hasTimeZone())
    {
      const double tzHours = time.getTime().getTimeZone();
      tp -= milliseconds{std::llround(tzHours * 3600.0 * 1000.0)};
    }

    return tp;
  }

  double ComputeMiliSecDuration(const OFDateTime& start, const OFDateTime& stop)
  {
    using namespace std::chrono;

    const auto delta = ConvertOFDateTimeToTimePoint(stop)
                     - ConvertOFDateTimeToTimePoint(start);

    return duration_cast<duration<double, std::milli>>(delta).count();
  }
}
