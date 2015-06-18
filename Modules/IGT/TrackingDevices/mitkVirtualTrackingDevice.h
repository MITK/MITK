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

#ifndef MITKVIRTUALTRACKINGDEVICE_H_HEADER_INCLUDED_
#define MITKVIRTUALTRACKINGDEVICE_H_HEADER_INCLUDED_

#include <MitkIGTExports.h>
#include <mitkTrackingDevice.h>
#include <mitkVirtualTrackingTool.h>
#include <itkMultiThreader.h>

#include "itkFastMutexLock.h"
#include <vector>

namespace mitk
{
  /** Documentation
  * \brief Class representing a tracking device which generates random positions / orientations.
  *        No hardware is needed for tracking device.
  *
  * This TrackingDevice class does not interface with a physical tracking device. It simulates
  * a tracking device by moving the tools on a randomly generated spline path.
  *
  * \ingroup IGT
  */
  class MITKIGT_EXPORT VirtualTrackingDevice : public TrackingDevice
  {
  public:

    mitkClassMacro(VirtualTrackingDevice, TrackingDevice);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)


    /**
    * \brief Sets the refresh rate of the virtual tracking device in ms
    * \warning This refresh rate is not guaranteed. A thread is used to refresh the positions
    * of the virtual tools. However, this thread may not run at all during this refresh time period.
    * \return Sets the refresh rate of the virtual tracking device in ms
    */
    itkSetMacro(RefreshRate, unsigned int)

    /**
    * \brief Returns the refresh rate in ms.
    * \return Returns the refresh rate in ms.
    */
    itkGetConstMacro(RefreshRate, unsigned int)

    /**
    * \brief Starts the tracking.
    *
    * After StartTracking() is called,
    * the tools will move on their spline paths with a constant velocity that can be set with
    * SetToolSpeed(). The standard velocity is 10 seconds for one complete cycle along the spline path.
    * \warning tool speed is not yet used in the current version
    * \return Returns true if the tracking is started. Returns false if there was an error.
    */
    virtual bool StartTracking() override;

    /**
    * \brief Stops the tracking.
    * \return Returns true if the tracking is stopped. Returns false if there was an error.
    */
    virtual bool StopTracking() override;

    /**
    * \brief Opens the connection to the device. This have to be done before the tracking is started.
    * @throw mitk::IGTException Throws an exception if there are two less control points to start the the virtual device.
    */
    virtual bool OpenConnection() override;

    /**
    * \brief Closes the connection and clears all resources.
    */
    virtual bool CloseConnection() override;

    /**
    * \return Returns the number of tools which have been added to the device.
    */
    virtual unsigned int GetToolCount() const override;

    /**
    * \param toolNumber The number of the tool which should be given back.
    * \return Returns the tool which the number "toolNumber". Returns NULL, if there is
    * no tool with this number.
    */
    TrackingTool* GetTool(unsigned int toolNumber) const override;

    /**
    * \brief Adds a tool to the tracking device.
    *
    * The tool will have a random path on which it will move around. The path is created with a
    * spline function and random control points inside the tracking volume.
    *
    * \param tool  The tool which will be added.
    * \return Returns true if the tool has been added, false otherwise.
    */
    TrackingTool* AddTool(const char* toolName);


    /**
    * \brief Set the tracking volume bounds
    *
    * This will set the tracking volume as an axis aligned bounding box
    * defined by the six bounds values xMin, xMax, yMin, yMax, zMin, zMax.
    * Note that the random path of existing tools will not be updated with the new
    * tracking volume. Tools that are created after calling SetBounds() will use the
    * new tracking volume
    */
    itkSetVectorMacro(Bounds, mitk::ScalarType, 6);

    /**
    * \brief return the tracking volume bounds
    *
    * This will return the tracking volume as an axis aligned bounding box
    * defined by the six bounds values xMin, xMax, yMin, yMax, zMin, zMax
    */
    const mitk::ScalarType* GetBounds() const
    {
      return m_Bounds;
    };

    /**
    * \brief return the approximate length of the spline for tool with index idx in millimeter
    *
    * if the index idx is not a
    * valid tool index, a std::invalid_argument exception is thrown.
    * GetSplineChordLength() returns the distance between all control points of the
    * spline in millimeter. This can be used as an approximation for the length of the spline path.
    */
    mitk::ScalarType GetSplineChordLength(unsigned int idx);

    /**
    * \brief sets the speed of the tool idx in rounds per second
    *
    * The virtual tools will travel along a closed spline path.
    * This method sets the speed of a tool as a factor of how many rounds per second
    * the tool should move. A setting of 1.0 will indicate one complete round per second.
    * Together with GetSplineChordLength(), the speed in millimeter per second can be estimated.
    * roundsPerSecond must be positive and larger than 0.0001.
    * \warning Tool speed is currently not used.
    * \TODO: use tool speed
    */
    void SetToolSpeed(unsigned int idx, mitk::ScalarType roundsPerSecond);

  protected:
    VirtualTrackingDevice();
    ~VirtualTrackingDevice();
    /**
    * \brief This method tracks tools as long as the variable m_Mode is set to "Tracking".
    * Tracking tools means generating random numbers for the tool position and orientation.
    * @throw mitk::IGTException Throws an mitk::IGTException if there is an error during virtual tool tracking.
    */
    void TrackTools();

    void InitializeSpline(mitk::VirtualTrackingTool* t);  ///< initializes the spline path of the tool t with random control points inside the current tracking volume

    static ITK_THREAD_RETURN_TYPE ThreadStartTracking(void* data); ///< static start method for tracking thread

    typedef mitk::VirtualTrackingTool::SplineType::ControlPointType ControlPointType;

    ControlPointType GetRandomPoint(); ///< returns a random position inside the tracking volume (defined by m_Bounds)
    mitk::VirtualTrackingTool* GetInternalTool(unsigned int idx);

    typedef std::vector<VirtualTrackingTool::Pointer> ToolContainer; ///< container type for tracking tools
    ToolContainer m_AllTools;                       ///< container for all tracking tools
    itk::FastMutexLock::Pointer m_ToolsMutex; ///< mutex for coordinated access of tool container

    itk::MultiThreader::Pointer m_MultiThreader;    ///< MultiThreader that starts continuous tracking update
    int m_ThreadID;

    unsigned int m_RefreshRate;                     ///< refresh rate of the internal tracking thread in milliseconds (NOT refreshs per second!)
    unsigned int m_NumberOfControlPoints;           ///< number of control points for the random path generation

    mitk::ScalarType m_Bounds[6];                   ///< bounding box of the tracking volume stored as {xMin, xMax, yMin, yMax, zMin, zMax}
  };
}//mitk
#endif /* MITKVIRTUALTRACKINGDEVICE_H_HEADER_INCLUDED_ */
