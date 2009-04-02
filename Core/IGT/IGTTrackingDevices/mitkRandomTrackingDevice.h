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


namespace mitk
{
  /** Documentation
  * \brief Class representing a tracking device which generates random positions / orientations. No hardware is needed for tracking device.
  *
  * 
  * \ingroup IGT
  */
  class RandomTrackingDevice : public TrackingDevice
  {
  public:

    mitkClassMacro(RandomTrackingDevice, TrackingDevice);
    itkNewMacro(Self);

    /**
    * \brief Starts the tracking.
    * \return Sets the refresh rate of the pseudo trackingdevice in ms
    */
    itkSetMacro(RefreshRate,unsigned int)

    /**
    * \brief Starts the tracking.
    * \return Returns the refresh rate in ms.
    */
    itkGetMacro(RefreshRate,unsigned int)

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
    * \return Returns all tools of the tracking device.
    */
    std::vector<InternalTrackingTool::Pointer> GetAllTools();

    /**
    * \brief Opens the connection to the device. This have to be done before the tracking is startet.
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
    TrackingTool* GetTool(unsigned int toolNumber);

    /**
    * \brief Adds a tool to the tracking device.
    * \param tool  The tool which will be added.
    * \return Returns true if the tool has been added, false otherwise.
    */
    bool AddTool(InternalTrackingTool::Pointer tool);

  
  protected:
    RandomTrackingDevice();
    ~RandomTrackingDevice();
    /**
    * \brief This method tracks tools as long as the variable m_Mode is set to "Tracking".
    * Tracking tools means generating random numbers for the tool position and orientation.
    */
    void TrackTools();

    static ITK_THREAD_RETURN_TYPE ThreadStartTracking(void* data);
  
    std::vector<InternalTrackingTool::Pointer> m_AllTools;

    itk::MultiThreader::Pointer m_MultiThreader;
    int m_ThreadID;
    
    unsigned int m_RefreshRate;
  };   
}//mitk
#endif /* MITKRandomTrackingDevice_H_HEADER_INCLUDED_ */
