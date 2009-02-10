#ifndef MITKCLARONINTERFACE_H_HEADER_INCLUDED_
#define MITKCLARONINTERFACE_H_HEADER_INCLUDED_

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
    * \brief This is only a stub, please switch the cmake variable USE_MICRON_TRACKER to on if you want to use the Microntracker.
    */
    ClaronInterface(std::string calibrationDir, std::string toolFilesDir);

    /**
    * \brief This is only a stub, please switch the cmake variable USE_MICRON_TRACKER to on if you want to use the Microntracker.
    */
    bool StartTracking();

    /**
    * \brief This is only a stub, please switch the cmake variable USE_MICRON_TRACKER to on if you want to use the Microntracker.
    */
    bool StopTracking();

    /**
    * \brief This is only a stub, please switch the cmake variable USE_MICRON_TRACKER to on if you want to use the Microntracker.
    */
    std::vector<claronToolHandle> GetAllActiveTools();

    /**
    * \brief This is only a stub, please switch the cmake variable USE_MICRON_TRACKER to on if you want to use the Microntracker.
    */
    std::vector<double> GetTipPosition(claronToolHandle c);

    /**
    * \brief This is only a stub, please switch the cmake variable USE_MICRON_TRACKER to on if you want to use the Microntracker.
    */
    std::vector<double> GetTipQuaternions(claronToolHandle c);

    /**
    * \brief This is only a stub, please switch the cmake variable USE_MICRON_TRACKER to on if you want to use the Microntracker.
    */
    std::vector<double> GetPosition(claronToolHandle c);

    /**
    * \brief This is only a stub, please switch the cmake variable USE_MICRON_TRACKER to on if you want to use the Microntracker.
    */
    std::vector<double> GetQuaternions(claronToolHandle c);

    /**
    * \brief This is only a stub, please switch the cmake variable USE_MICRON_TRACKER to on if you want to use the Microntracker.
    */
    std::string GetName(claronToolHandle c);

    /**
    * \brief This is only a stub, please switch the cmake variable USE_MICRON_TRACKER to on if you want to use the Microntracker.
    */
    void GrabFrame();
  
    /**
    * @return Returns wether the MicronTracker is installed (means wether the C-Make-Variable "MITK_USE_MICRON_TRACKER" is set),
    *         so returns false in this case.
    */
    bool IsMicronTrackerInstalled();

  };
}//mitk
#endif
