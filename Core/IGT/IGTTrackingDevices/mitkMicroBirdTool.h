/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-07-10 17:24:20 +0200 (Di, 10 Jul 2007) $
Version:   $Revision: 11220 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKMICROBIRDTOOL_H_HEADER_INCLUDED_
#define MITKMICROBIRDTOOL_H_HEADER_INCLUDED_

#include <mitkTrackingTool.h>
#include "mitkTrackingTypes.h"

namespace mitk {
  namespace tracking {

    //##Documentation
    //## \brief Implementation of a daVinci tool - either one of the instrument arms or the camera arm
    //##
    //## 
    //## @ingroup Tracking

    class MicroBirdTool : public TrackingTool
    {
    public:
      mitkClassMacro(MicroBirdTool, TrackingTool);
      itkNewMacro(Self);

      virtual void GetPosition(mitk::Point3D& position) const;          // returns the current position of the tool as an array of three floats
      virtual void GetQuaternion(mitk::Quaternion& orientation) const;  // returns the current orientation of the tool as a quaternion in an array of four floats

      virtual bool Enable();                       // enable the tool, so that it will be tracked. NOTE: This tool class can not communicate with the tracking device directly. The tracking device class has to check this status itself and update the tracking device accordingly.
      virtual bool Disable();                      // disable the tool, so that it will not be tracked anymore. NOTE: This tool class can not communicate with the tracking device directly. The tracking device class has to check this status itself and update the tracking device accordingly.
      virtual bool IsEnabled() const;              // returns wether the tool is enabled or disabled
      virtual bool IsDataValid() const;            // returns true if the current position data is valid (no error during tracking, tracking error below threshold, ...)
      virtual float GetTrackingError() const;      // return one value that corresponds to the overall tracking error.

      virtual void SetPosition(float x, float y, float z);  // set the position 
      virtual void SetQuaternion(float q0, float qx, float qy, float qz); // set the orientation as a quaternion
      virtual void SetTrackingError(float error);           // set the tracking error
      virtual void SetDataValid(bool _arg);

      virtual void SetErrorMessage(const char* _arg);

    protected:

      MicroBirdTool();
      virtual ~MicroBirdTool();

      float m_Position[3];
      float m_Quaternion[4];
      float m_TrackingError;
      bool m_Enabled;
      bool m_DataValid;

    };
  } // namespace tracking
} // namespace mitk

#endif /* MITKMICROBIRDTOOL_H_HEADER_INCLUDED_ */
