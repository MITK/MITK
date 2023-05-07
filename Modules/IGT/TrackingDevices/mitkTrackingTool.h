/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkTrackingTool_h
#define mitkTrackingTool_h

#include <itkObject.h>
#include <MitkIGTExports.h>
#include <mitkCommon.h>
#include <mitkNumericTypes.h>
#include <mutex>

namespace mitk
{
  /**Documentation
  * \brief Interface for all Tracking Tools
  *
  * This class is a complete TrackingTool implementation. It can either be used directly by
  * TrackingDevices, or be subclassed for more specific implementations.
  * mitk::MicroBirdTrackingDevice uses this class to manage its tools. Other tracking devices
  * uses specialized versions of this class (e.g. mitk::NDITrackingTool)
  *
  * The TrackingTool class holds all coordinate transforms associated with tracking of a tool.
  * The sensor attached to the tool is localized in the global tracking coordinate system (m_Position, m_Orientation).
  * A tool tip (m_ToolTipPosition) can be defined in sensor coordinates.
  * The tool axis defines the main axis of the tool and is defined as the negative z-axis of the tool tip coordinate system
  * The main axis of the representation object of the tool (e.g. a surface) has to be defined along the negative z-axis
  * \imageMacro{TrackingTool.png,"Coordinate transforms associated to the tracking tool.",3}
  *
  * \ingroup IGT
  */
  class MITKIGT_EXPORT TrackingTool : public itk::Object
  {
  public:
    mitkClassMacroItkParent(TrackingTool, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    void PrintSelf(std::ostream& os, itk::Indent indent) const override;

    virtual const char* GetToolName() const; ///< every tool has a name thatgit  can be used to identify it.
    virtual void SetToolName(const std::string _arg); ///< Sets the name of the tool
    virtual void SetToolName(const char* _arg); ///< Sets the name of the tool

    Point3D GetToolTipPosition() const; ///< returns the tool tip in tool coordinates, which where set by SetToolTip
    Quaternion GetToolAxisOrientation() const; ///< returns the transformation of the tool axis with respect to the MITK-IGT main tool axis (0,0,-1)
    virtual void SetToolTipPosition(Point3D toolTipPosition, Quaternion orientation, ScalarType eps=0.0); ///< defines a tool tip for this tool in tool coordinates. GetPosition() and GetOrientation() return the data of the tool tip if it is defined. By default no tooltip is defined.
    virtual bool IsToolTipSet() const; ///< returns true if a tool tip is set, false if not

    virtual void GetPosition(Point3D& position) const; ///< returns the current position of the tool as an array of three floats (in the tracking device coordinate system)
    virtual void SetPosition(Point3D position); ///< sets the position

    virtual void GetOrientation(Quaternion& orientation) const; ///< returns the current orientation of the tool as a quaternion in a mitk::Point4D (in the tracking device coordinate system)
    virtual void SetOrientation(Quaternion orientation); ///< sets the orientation as a quaternion

    virtual bool Enable(); ///< enables the tool, so that it will be tracked
    virtual bool Disable(); ///< disables the tool, so that it will not be tracked anymore
    virtual bool IsEnabled() const; ///< returns whether the tool is enabled or disabled

    virtual void SetDataValid(bool isDataValid); ///< sets if the tracking data (position & orientation) is valid
    virtual bool IsDataValid() const; ///< returns true if the current position data is valid (no error during tracking, tracking error below threshold, ...)

    virtual float GetTrackingError() const; ///< returns one value that corresponds to the overall tracking error.
    virtual void SetTrackingError(float error); ///< sets the tracking error

    virtual const char* GetErrorMessage() const; ///< if the data is not valid, ErrorMessage should contain a string explaining why it is invalid (the Set-method should be implemented in subclasses, it should not be accessible by the user)
    virtual void SetErrorMessage(const char* _arg); ///< sets the error message

    itkSetMacro(IGTTimeStamp, double) ///< Sets the IGT timestamp of the tracking tool object (time in milliseconds)
    itkGetConstMacro(IGTTimeStamp, double) ///< Gets the IGT timestamp of the tracking tool object (time in milliseconds). Returns 0 if the timestamp was not set.

  protected:
    TrackingTool();
    ~TrackingTool() override;

    std::string m_ToolName;                          ///< every tool has a name that can be used to identify it.
    std::string m_ErrorMessage;                      ///< if a tool is invalid, this member should contain a human readable explanation of why it is invalid
    double m_IGTTimeStamp;                           ///< contains the time at which the tracking data was recorded
    mutable std::mutex m_MyMutex;           ///< mutex to control concurrent access to the tool

    Point3D m_Position;         ///< holds the position of the tool in global tracking coordinates
    Quaternion m_Orientation;   ///< holds the orientation of the tool´in global tracking coordinates
    float m_TrackingError;      ///< holds the tracking error of the tool
    bool m_Enabled;             ///< if true, tool is enabled and should receive tracking updates from the tracking device
    bool m_DataValid;           ///< if true, data in m_Position and m_Orientation is valid, e.g. true tracking data
    Point3D m_ToolTipPosition;  ///< holds the position of the tool tip in the coordinate system of the tracking sensor
    Quaternion m_ToolAxisOrientation; ///< holds the rotation of the sensor coordinate system such that the z-axis coincides with the main tool axis e.g. obtained by a tool calibration
    bool m_ToolTipSet;
  };
} // namespace mitk
#endif
