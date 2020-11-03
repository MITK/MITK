/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkOptitrackErrorMessages.h"

std::string mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(int result)
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
