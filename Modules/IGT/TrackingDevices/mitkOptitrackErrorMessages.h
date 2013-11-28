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



namespace mitk
{
/**
  * \brief Helper function to get the error messages from Optitrack API
  * \author E. Marinetto (emarinetto@hggm.es) Instituto de Investigación Sanitaria Gregorio Marañón, Madrid, Spain. & M. Noll (matthias.noll@igd.fraunhofer.de) Cognitive Computing & Medical Imaging | Fraunhofer IGD
  */

  static std::string GetOptitrackErrorMessage(int result)
  {
      std::string message = "";
      switch(result)
      {
        case 0: message = "[Optitrack API] Successful Result"; break;
        case 1: message = "[Optitrack API] File Not Found"; break;
        case 2:  message = "[Optitrack API] Load Failed"; break;
        case 3:    message = "[Optitrack API] Failed"; break;
        case 8:  message = "[Optitrack API] Invalid File"; break;
        case 9:  message = "[Optitrack API] Invalid Calibration File"; break;
        case 10:  message = "[Optitrack API] Unable To Initialize"; break;
        case 11:  message = "[Optitrack API] Invalid License"; break;
        case 14:  message = "[Optitrack API] No Frames Available"; break;
        case 100:  message = "Optitrack Tracking Tools is not installed in the computer"; break;
        default: message = "[Optitrack API] Unknown error occured"; break;
      }
      return message;
    }
}


#endif
