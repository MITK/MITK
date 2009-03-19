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
  * \brief implements TrackingTool interface
  *
  * This class is a complete TrackingTool implementation. It can either be used directly by
  * TrackingDevices, or be subclassed for more specific implementations.
  * mitk::MicroBirdTrackingDevice uses this class to manage its tools. Other tracking devices
  * uses specialized versions of this class (e.g. mitk::NDITrackingTool)
  *
  * \ingroup IGT
  */
  class InternalTrackingTool : public TrackingTool
  {
    friend class MicroBirdTrackingDevice; // Add all TrackingDevice subclasses that use InternalTrackingDevice directly
  public:
    mitkClassMacro(InternalTrackingTool, TrackingTool);
    itkNewMacro(Self);
    virtual void GetPosition(mitk::Point3D& position) const;    ///< returns the current position of the tool as an array of three floats (in the tracking device coordinate system)
    virtual void GetOrientation(mitk::Quaternion& orientation) const;  ///< returns the current orientation of the tool as a quaternion (in the tracking device coordinate system)
    virtual bool Enable();                                      ///< enablea the tool, so that it will be tracked. Returns true if enabling was successfull
    virtual bool Disable();                                     ///< disables the tool, so that it will not be tracked anymore. Returns true if disabling was successfull
    virtual bool IsEnabled() const;                             ///< returns whether the tool is enabled or disabled
    virtual bool IsDataValid() const;                           ///< returns true if the current position data is valid (no error during tracking, tracking error below threshold, ...)
    virtual float GetTrackingError() const;                     ///< return one value that corresponds to the overall tracking error. The dimension of this value is specific to each tracking device
    virtual void SetToolName(const std::string _arg);           ///< Sets the name of the tool
    virtual void SetToolName(const char* _arg);                 ///< Sets the name of the tool
    virtual void SetPosition(mitk::Point3D position);           ///< sets the position
    virtual void SetOrientation(mitk::Quaternion orientation);  ///< sets the orientation as a quaternion
    virtual void SetTrackingError(float error);                 ///< sets the tracking error
    virtual void SetDataValid(bool _arg);                       ///< sets if the tracking data (position & Orientation) is valid
    virtual void SetErrorMessage(const char* _arg);             ///< sets the error message

  protected:

    InternalTrackingTool();
    virtual ~InternalTrackingTool();
    
    Point3D m_Position;      ///< holds the position of the tool
    Quaternion m_Orientation;   ///< holds the orientation of the tool 
    float m_TrackingError;    ///< holds the tracking error of the tool
    bool m_Enabled;           ///< if true, tool is enabled and should receive tracking updates from the tracking device
    bool m_DataValid;         ///< if true, data in m_Position and m_Orientation is valid, e.g. true tracking data
  };
} // namespace mitk
#endif /* MITKINTERNALTRACKINGTOOL_H_HEADER_INCLUDED_ */
