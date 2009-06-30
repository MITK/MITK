/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkRandomTrackingDevice.h,v $
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

#ifndef MITKRandomTrackingDevice_H_HEADER_INCLUDED_
#define MITKRandomTrackingDevice_H_HEADER_INCLUDED_

#include <vector>
#include <mitkConfig.h>
#include <mitkTrackingDevice.h>
#include <mitkInternalTrackingTool.h>
#include <itkMultiThreader.h>
#include <itkNonUniformBSpline.h>


namespace mitk
{
  /** Documentation
  * \brief Class representing a tracking device which generates random positions / orientations. No hardware is needed for tracking device.
  *
  * 
  * \ingroup IGT
  */
  class MITK_IGT_EXPORT RandomTrackingDevice : public TrackingDevice
  {
  public:

    mitkClassMacro(RandomTrackingDevice, TrackingDevice);
    itkNewMacro(Self);

    /**
    * \brief Sets the refresh rate of the virtual tracking device in ms
    * \return Sets the refresh rate of the virtual tracking device in ms
    */
    itkSetMacro(RefreshRate,unsigned int)

    /**
    * \brief Returns the refresh rate in ms.
    * \return Returns the refresh rate in ms.
    */
    itkGetConstMacro(RefreshRate,unsigned int)

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
    * \brief return the approximate length of the spline for tool with index idx in milimeter
    *
    * The call to OpenConnection() will initialize a spline path for each tool.
    * If GetSplineChordLength() is called before OpenConnection() or if the idx is out of range of 
    * valid tool indices, a std::invalid_argument exception is thrown. 
    * GetSplineChordLength() returns the distance between all control points of the 
    * spline in milimeter. This can be used as an approximation for the length of the spline path.
    */
    mitk::ScalarType GetSplineChordLength(unsigned int idx);
  
    /**
    * \brief sets the speed of the tool idx in rounds per second
    *
    * The virtual tools will travel along a closed spline path. 
    * This method sets the speed of a tool as a factor of how many rounds per second
    * the tool should move. A setting of 1.0 will indicate one complete round per second.
    * Together with GetSplineChordLength(), the speed in milimeter per second can be estimated. 
    * roundsPerSecond must be positive and larger than 0.0001. 
    */
    void SetToolSpeed(unsigned int idx, mitk::ScalarType roundsPerSecond);

typedef itk::NonUniformBSpline<3> SplineType;
SplineType::Pointer GetSpline(unsigned int index)
    {
      if (index >= m_Interpolators.size())
        throw std::invalid_argument("index out of range");
      return m_Interpolators.at(index);
    };

  protected:
    RandomTrackingDevice();
    ~RandomTrackingDevice();
    /**
    * \brief This method tracks tools as long as the variable m_Mode is set to "Tracking".
    * Tracking tools means generating random numbers for the tool position and orientation.
    */
    void TrackTools();

    static ITK_THREAD_RETURN_TYPE ThreadStartTracking(void* data);

    mitk::Point3D GetRandomPoint(); ///< returns a random position inside the tracking volume (defined by m_Bounds)

    typedef std::vector<InternalTrackingTool::Pointer> ToolContainer;
    ToolContainer m_AllTools;

    itk::MultiThreader::Pointer m_MultiThreader;
    int m_ThreadID;
    
    unsigned int m_RefreshRate;
    unsigned int m_NumberOfControlPoints;

    
    typedef std::vector<SplineType::Pointer> SplineVectorType;
    SplineVectorType m_Interpolators;
    std::vector<mitk::ScalarType> m_SplineLengths;
    std::vector<mitk::ScalarType> m_ToolSpeeds;
    mitk::ScalarType m_Bounds[6];
  };   
}//mitk
#endif /* MITKRandomTrackingDevice_H_HEADER_INCLUDED_ */
