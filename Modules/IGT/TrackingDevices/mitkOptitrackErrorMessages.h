/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef OptiTrackErrorMessages_H_HEADER_INCLUDED_
#define OptiTrackErrorMessages_H_HEADER_INCLUDED_

/**
* \brief Maximum number of attempts for Initialization, Shutdown and CleanUp
*/
#ifndef OPTITRACK_ATTEMPTS
#define OPTITRACK_ATTEMPTS 10
#endif

/**
* \brief Time to refresh the tools location (ms)
*/
#ifndef OPTITRACK_FRAME_RATE
#define OPTITRACK_FRAME_RATE 10
#endif

#include <MitkIGTExports.h>
#include <string>

namespace mitk
{
  class MITKIGT_EXPORT OptitrackErrorMessages
  {
  public:
  /**
   * \brief Helper function to get the error messages from Optitrack API
   * \author E. Marinetto (emarinetto@hggm.es) Instituto de Investigación Sanitaria Gregorio Marañón, Madrid, Spain. & M. Noll (matthias.noll@igd.fraunhofer.de) Cognitive Computing & Medical Imaging | Fraunhofer IGD
   */
  static std::string GetOptitrackErrorMessage(int result);
  };
}
#endif
