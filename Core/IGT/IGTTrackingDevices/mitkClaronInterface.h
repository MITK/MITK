#ifndef MITKCLARONINTERFACE_H_HEADER_INCLUDED_
#define MITKCLARONINTERFACE_H_HEADER_INCLUDED_
#define MTC(func) {int r = func; if (r!=mtOK) printf("MTC error: %s\n",MTLastErrorString()); };

#include <vector>
#include <string>

typedef int mtHandle;

namespace mitk
{
  typedef int claronToolHandle;

  class ClaronInterface
  {
  public:

    /**
    * \brief Constructor for a claroninterface.
    * @param calibrationDir The directory where the device can find the camera calibration file.
    * @param toolFilesDir The directory for the tool files.
    */
    ClaronInterface(std::string calibrationDir, std::string toolFilesDir);

    /**
    * \brief Opens the connection to the device and makes it ready to track tools.
    * @return Returns true if theres a connection to the device and the device is ready to track tools, false if not.
    */
    bool StartTracking();

    /**
    * \brief Clears all resources. After this method have been called the system isn't any longer ready to track.
    * @return Returns true if the operation was succesful, false if not.
    */
    bool StopTracking();

    /**
    * @return Returns all tools which have been detected at the last frame grab.
    */
    std::vector<claronToolHandle> GetAllActiveTools();

    /**
    * @return Returns the position of the tooltip. If no tooltip is defined the Method returns the position of the tool.
    */
    std::vector<double> GetTipPosition(claronToolHandle c);

    /**
    * @return Returns the quarternions of the tooltip. If no tooltip is defined the Method returns the quarternions of the tool.
    */
    std::vector<double> GetTipQuaternions(claronToolHandle c);

    /**
    * @return Returns the position of the tool
    */
    std::vector<double> GetPosition(claronToolHandle c);

    /**
    * @return Returns the quaternion of the tool.
    */
    std::vector<double> GetQuaternions(claronToolHandle c);

    /**
    * @return Returns the name of the tool. This name is given by the calibration file.
    * @param c The handle of the tool, which name should be given back.
    */
    std::string GetName(claronToolHandle c);

    /**
    * \brief Grabs a frame from the camera.
    */
    void GrabFrame();

    /**
    * @return Returns wether the tracking device is tracking or not.
    */
    bool IsTracking();
    
	/**
    * @return Returns wether the MicronTracker is installed (means wether the C-Make-Variable "MITK_USE_MICRON_TRACKER" is set),
    *         so returns true in this case. This is because the class mitkClaronInterfaceStub, in which the same Method returns false 
    *		  is used otherways.
    */
    bool IsMicronTrackerInstalled();


  protected:
    bool isTracking;

    /** \brief gets the MTHome variable from the system. The path is set manually, so this function is not in use. */
    int getMTHome (  char *sMTHome, int size );

    /** \brief Variable is not in use. Activate the code in the function StartTracking if you need it.*/
    char MTHome[512];

    char calibrationDir[512];
    char markerDir[512];
    mtHandle IdentifiedMarkers;
    mtHandle PoseXf;
    mtHandle CurrCamera;
    mtHandle IdentifyingCamera;
  };
}//mitk
#endif