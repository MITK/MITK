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

#ifndef MITKPolhemusTOOL_H_HEADER_INCLUDED_
#define MITKPolhemusTOOL_H_HEADER_INCLUDED_

#include <mitkPolhemusInterface.h>
#include <mitkTrackingTool.h>
#include <itkFastMutexLock.h>

namespace mitk
{
  class PolhemusTrackingDevice;
  /** Documentation:
  *   \brief  An object of this class represents a tool of a Polhemus tracking device.
  *           A tool has to be added to a tracking device which will then
  *           continuously update the tool coordinates.
  *   \ingroup IGT
  */
  class MITKIGT_EXPORT PolhemusTool : public TrackingTool
  {
  public:
    friend class PolhemusTrackingDevice;
    mitkClassMacro(PolhemusTool, TrackingTool);

    /**
    * \brief Sets the port of the tool. (e.g. 1 for port "SENS 1" etc.)
    */
    virtual void SetToolPort(int _ToolPort);

    /**
    * \brief Sets the port of the tool. (e.g. 1 for port "SENS 1" etc.)
    */
    virtual int GetToolPort();

  protected:
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    PolhemusTool();
    virtual ~PolhemusTool();

    //This is the port, on which the tool is connected. It is identical with the "ToolIdentifier" set on NavigationDataTools.
    //If tool is connected on port "SENS 2", the m_ToolPort is 2 etc.
    int m_ToolPort;
  };
}//mitk
#endif // MITKPolhemusTOOL_H_HEADER_INCLUDED_
