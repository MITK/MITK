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

#ifndef MITKPolhemusINTERFACE_H_HEADER_INCLUDED_
#define MITKPolhemusINTERFACE_H_HEADER_INCLUDED_


#include <vector>
#include <string>

#include <MitkIGTExports.h>
#include "mitkCommon.h"

#include <itkObject.h>
#include <itkObjectFactory.h>

#include <windows.h>
#include <tchar.h>
#include <string>

#include <mitkNavigationData.h>


class CPDIdev;

namespace mitk
{

  /** Documentation:
  *   \brief An object of this class represents the interface to Polhemus trackers.
  *   All variables with the name "tool" start with index 1, which is the station number of Polhemus.
  *   Make sure to call functions in this class with parameter "1" if you want to loop over all tools.
  *   If you need to access an array (e.g. m_Hemisphere), you need to use "_tool -1" and adapt your index for loops...
  *   \ingroup IGT
  */
  class MITKIGT_EXPORT PolhemusInterface : public itk::Object
  {
  public:

    mitkClassMacroItkParent(PolhemusInterface,itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    struct trackingData
    {
      mitk::Point3D pos;
      mitk::Quaternion rot;
      BYTE id;
    };

    /**
    * \brief Opens the connection to the device and makes it ready to track tools.
    * \return Returns true if there is a connection to the device and the device is ready to track tools, false if not.
    */
    bool StartTracking();

    /**
    * \brief Clears all resources. After this method have been called the system isn't ready to track any longer.
    * \return Returns true if the operation was succesful, false if not.
    */
    bool StopTracking();

    bool Connect();

    bool Disconnect();

    /** @return Returns a single frame. Only works if the tracking device is not in continous tracking mode. Returns an empty vector in case of an error.*/
    std::vector<trackingData> GetSingleFrame();

    /** @return Returns the last frame when the tracking device is in continous tracking mode. Returns an empty vector in case of an error.*/
    std::vector<trackingData> GetLastFrame();

    /** @return Returns the number of tools. Returns 0 if no information is avialable.*/
    unsigned int GetNumberOfTools();

    /** Enables/disables hemisphere tracking for all stations/tools. */
    void SetHemisphereTrackingEnabled(bool _HeisphereTrackingEnabeled);

    /** Toggles the current hemisphere. Parameter _tool describes, for which tool the hemisphere should change. Default -1 toggles all tools.
        Index starts at "1" for the first tool (i.e. station number of Polhemus). Not 0!
    */
    void ToggleHemisphere(int _tool = -1);

    /** Convenient method to print the status of the tracking device (true/false) if connection is established. For debugging...*/
    void PrintStatus();

    /** Sets the Hemisphere of tool _tool to the vector _hemisphere. "-1" sets all tools.
        Index starts at "1" for the first tool (i.e. station number of Polhemus). Not 0!
    */
    void SetHemisphere(int _tool, mitk::Vector3D _hemisphere);

    /** Get the Hemisphere for _tool as mitk vector. -1 ("all tools") returns hemisphere of first tool.
        Index starts at "1" for the first tool (i.e. station number of Polhemus). Not 0!
    */
    mitk::Vector3D GetHemisphere(int _tool);

    /** Get the ports on which tools are connected. Returns empty vector if device is not connected!
    */
    std::vector<int> GetToolPorts();

    /** Is Hemisphere Tracking Enabled for this tool? */
    bool GetHemisphereTrackingEnabled(int _tool);

  protected:
    /**
    * \brief standard constructor
    */
    PolhemusInterface();
    /**
    * \brief standard destructor
    */
    ~PolhemusInterface();

	/** Polhemus liberty/patriot tracker object*/
    CPDIdev* m_pdiDev;
	
	/** Parses polhemus raw data to a collection of tracking data of single tools. */
    std::vector<mitk::PolhemusInterface::trackingData> ParsePolhemusRawData(PBYTE pBuf, DWORD dwSize);

    unsigned int m_numberOfTools;

    bool m_continousTracking;

    bool InitializeDevice();

    bool SetupDevice();

  private:
    std::vector<mitk::Vector3D> m_Hemispheres;

    //This vector stores the order of tools, which are available.
    //E.g. only Sensor 1 and 3 are attached, then this vector maps the first tool (0) to Polhemus identifier 1 and the second tool (1) to Polhemus 3.
    std::vector<int> m_ToolPorts;

  };
}//mitk
#endif
