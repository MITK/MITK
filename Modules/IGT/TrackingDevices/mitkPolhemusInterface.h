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
#include <PDI.h>

#include <mitkNavigationData.h>



namespace mitk
{

  /** Documentation:
  *   \brief An object of this class represents the interface to Polhemus trackers.
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


    std::vector<trackingData> GetLastFrame();

    unsigned int GetNumberOfTools();

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
    CPDIdev m_pdiDev;

    unsigned int m_numberOfTools;


  };
}//mitk
#endif
