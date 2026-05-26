/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMTimeUtil.h>

#include <dcmtk/dcmdata/dcvrdt.h>

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

    return sys_days{ymd}
         + hours{time.getTime().getHour()}
         + minutes{time.getTime().getMinute()}
         + seconds{time.getTime().getIntSecond()}
         + milliseconds{time.getTime().getMilliSecond()};
  }

  double ComputeMiliSecDuration(const OFDateTime& start, const OFDateTime& stop)
  {
    using namespace std::chrono;

    const auto delta = ConvertOFDateTimeToTimePoint(stop)
                     - ConvertOFDateTimeToTimePoint(start);

    return duration_cast<duration<double, std::milli>>(delta).count();
  }
}
