/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKINTERNALTRACKINGTOOL_H_HEADER_INCLUDED_
#define MITKINTERNALTRACKINGTOOL_H_HEADER_INCLUDED_

#include <mitkTrackingTool.h>
#include <mitkCommon.h>
#include <mitkVector.h>
#include <itkFastMutexLock.h>

namespace mitk {


  /**Documentation
  * \brief Subclass of TrackingTool that holds methods used by TrackingDevices but that should not be exposed
  *
  * This class is a complete TrackingTool implementation. It can either be used directly by
  * TrackingDevices, or be subclassed for more specific implementations.
  *
  * @ingroup Tracking
  */
  class InternalTrackingTool : public TrackingTool
  {
    friend class MicroBirdTrackingDevice; // Add all TrackingDevice subclasses that use InternalTrackingDevice directly
  public:
    mitkClassMacro(InternalTrackingTool, TrackingTool);
    itkNewMacro(Self);
    virtual void GetPosition(mitk::Point3D& position) const;    ///< returns the current position of the tool as an array of three floats
    virtual void GetQuaternion(mitk::Quaternion& orientation) const;  ///< returns the current orientation of the tool as a quaternion
    virtual bool Enable();                                      ///< enable the tool, so that it will be tracked
    virtual bool Disable();                                     ///< disable the tool, so that it will not be tracked anymore
    virtual bool IsEnabled() const;                             ///< returns whether the tool is enabled or disabled
    virtual bool IsDataValid() const;                           ///< returns true if the current position data is valid (no error during tracking, tracking error below threshold, ...)
    virtual float GetTrackingError() const;                     ///< return one value that corresponds to the overall tracking error.
    virtual void SetToolName(const std::string _arg);           ///< Set the name of the tool
    virtual void SetToolName(const char* _arg);                 ///< Set the name of the tool
    virtual void SetPosition(float x, float y, float z);        ///< set the position 
    virtual void SetQuaternion(float q0, float qx, float qy, float qz); ///< set the orientation as a quaternion
    virtual void SetTrackingError(float error);                 ///< set the tracking error
    virtual void SetDataValid(bool _arg);                       ///< set if the tracking data (position & Orientation) is valid
    virtual void SetErrorMessage(const char* _arg);             ///< set the error message

  protected:

    InternalTrackingTool();
    virtual ~InternalTrackingTool();
    
    float m_Position[3];      ///< holds the position of the tool
    float m_Orientation[4];   ///< holds the orientation of the tool 
    float m_TrackingError;    ///< holds the tracking error of the tool
    bool m_Enabled;           ///< if true, tool is enabled and should receive tracking updates from the tracking device
    bool m_DataValid;         ///< if true, data in m_Position and m_Orientation is valid, e.g. true tracking data
  };
} // namespace mitk
#endif /* MITKINTERNALTRACKINGTOOL_H_HEADER_INCLUDED_ */
