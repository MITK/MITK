/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkVirtualTrackingDevice.h,v $
Language:  C++
Date:      $Date: 2009-02-11 18:22:32 +0100 (Mi, 11 Feb 2009) $
Version:   $Revision: 16250 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKVIRTUALTRACKINGDEVICE_H_HEADER_INCLUDED_
#define MITKVIRTUALTRACKINGDEVICE_H_HEADER_INCLUDED_

#include <vector>
#include <mitkConfig.h>
#include <mitkTrackingDevice.h>
#include <mitkInternalTrackingTool.h>
#include <itkMultiThreader.h>
#include <itkNonUniformBSpline.h>


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
  class MITK_IGT_EXPORT VirtualTrackingDevice : public TrackingDevice
  {
  public:

    mitkClassMacro(VirtualTrackingDevice, TrackingDevice);
    itkNewMacro(Self);

    typedef itk::NonUniformBSpline<3> SplineType; ///< spline type used for tool path interpolation

    /**
    * \brief Sets the refresh rate of the virtual tracking device in ms
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
    * \return Returns true if the tracking is started. Returns false if there was an error.
    */
    virtual bool StartTracking();

    /**
    * \brief Stops the tracking.
    * \return Returns true if the tracking is stopped. Returns false if there was an error.
    */
    virtual bool StopTracking();

    /**
    * \brief Opens the connection to the device. This have to be done before the tracking is started.
    *
    * This method will initialize a closed spline path for each tool. After StartTracking() is called,
    * the tools will move on their spline paths with a constant velocity that can be set with 
    * SetToolSpeed(). The standard velocity is 10 seconds for one complete cycle along the spline path.
    */
    virtual bool OpenConnection();

    /**
    * \brief Closes the connection and clears all resources.
    */
    virtual bool CloseConnection();

    /**
    * \return Returns the number of tools which have been added to the device.
    */
    virtual unsigned int GetToolCount() const;

    /**
    * \param toolNumber The number of the tool which should be given back.
    * \return Returns the tool which the number "toolNumber". Returns NULL, if there is
    * no tool with this number.
    */
    TrackingTool* GetTool(unsigned int toolNumber) const;

    /**
    * \brief Adds a tool to the tracking device.
    * \param tool  The tool which will be added.
    * \return Returns true if the tool has been added, false otherwise.
    */
    TrackingTool* AddTool(const char* toolName);


    /**
    * \brief Set the tracking volume bounds
    *
    * This will set the tracking volume as an axis aligned bounding box
    * defined by the six bounds values xMin, xMax, yMin, yMax, zMin, zMax
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
    * The call to OpenConnection() will initialize a spline path for each tool.
    * If GetSplineChordLength() is called before OpenConnection() or if the idx is out of range of 
    * valid tool indices, a std::invalid_argument exception is thrown. 
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
    */
    void TrackTools();

    static ITK_THREAD_RETURN_TYPE ThreadStartTracking(void* data); ///< static start method for tracking thread

    SplineType::ControlPointType GetRandomPoint(); ///< returns a random position inside the tracking volume (defined by m_Bounds)

    typedef std::vector<InternalTrackingTool::Pointer> ToolContainer; ///< container type for tracking tools
    ToolContainer m_AllTools;                       ///< container for all tracking tools

    itk::MultiThreader::Pointer m_MultiThreader;    ///< MultiThreader that starts continuous tracking update
    int m_ThreadID;
    
    unsigned int m_RefreshRate;                     ///< refresh rate of the internal tracking thread in milliseconds
    unsigned int m_NumberOfControlPoints;           ///< number of control points for the random path generation

    typedef std::vector<SplineType::Pointer> SplineVectorType; ///< storage container type for splines. 
    SplineVectorType m_Interpolators;               ///< storage container for splines. \TODO: each tool should manage its own spline
    std::vector<mitk::ScalarType> m_SplineLengths;  ///< storage container for splines lengths. \TODO: each tool should manage its own spline length
    std::vector<mitk::ScalarType> m_ToolVelocities; ///< storage container for splines velocities. \TODO: each tool should manage its own spline velocity
    mitk::ScalarType m_Bounds[6];                   ///< bounding box of the tracking volume stored as {xMin, xMax, yMin, yMax, zMin, zMax}
  };   
}//mitk
#endif /* MITKVIRTUALTRACKINGDEVICE_H_HEADER_INCLUDED_ */
