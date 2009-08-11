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

#ifndef MITKCLARONINTERFACE_H_HEADER_INCLUDED_
#define MITKCLARONINTERFACE_H_HEADER_INCLUDED_

#include <vector>
#include <string>

#include <MitkIGTExports.h>

#include <itkObject.h>
#include <itkObjectFactory.h>
#include "mitkCommon.h"

typedef int mtHandle;

namespace mitk
{
  typedef int claronToolHandle;
  
  /** Documentation:
  *   \brief An object of this class represents the interface to the MicronTracker. Normally the methods of this class
  *          are calling the c-functions which are provided by the MTC-library. But in this case the MicronTracker is
  *       not installed and so we only have stubs here. These stubs send error messages when you try to call the
  *       methods.
  *   \ingroup IGT
  */
  class MitkIGT_EXPORT ClaronInterface : public itk::Object
  {
  public:

    mitkClassMacro(ClaronInterface, itk::Object);
    itkNewMacro(Self);
    /**
    * \brief Initialization of claroninterface.
    * \param calibrationDir   The directory where the device can find the camera calibration file.
    * \param toolFilesDir     The directory for the tool files.
    */
    void Initialize(std::string calibrationDir, std::string toolFilesDir);
    
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
    const char* GetName(claronToolHandle c);

    /**
    * \brief This is only a stub, please switch the cmake variable USE_MICRON_TRACKER to on if you want to use the Microntracker.
    */
    void GrabFrame();

    /**
    * \return Returns wether the MicronTracker is installed (means wether the C-Make-Variable "MITK_USE_MICRON_TRACKER" is set),
    *         so returns false in this case.
    */
    bool IsMicronTrackerInstalled();

  protected:
    /**
    * \brief standard constructor
    */
    ClaronInterface();
    /**
    * \brief standard destructor
    */
    ~ClaronInterface();

  };
}//mitk
#endif