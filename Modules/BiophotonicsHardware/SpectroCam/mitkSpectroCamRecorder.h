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

#ifndef __mitkSpectroCamRecorder_h
#define __mitkSpectroCamRecorder_h


using namespace std;
using namespace cv;

namespace mitk
{
  /**
  * @brief Interface to the SpectroCam camera
  *
  *
  * @ingroup BiophotonicsHardware
  */

    class SpectroCamController : public itk::Object
      {
      private:

        void SpectroCamRecorder::DisplayCamerastream(SpectroCamImage in);

        namedWindow( "Display window", WINDOW_NORMAL ); // Create a window for display.

        bool SpectroCamController::DisplayCamerastream();

        static int imagesRecoreded=0;   //counter for recorded images


      public:

      }

}
//END mitk namespace
#endif

