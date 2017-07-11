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
#include <mitkInternalTrackingTool.h>
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
  class MITKIGT_EXPORT PolhemusTool : public InternalTrackingTool
  {
  public:
    friend class PolhemusTrackingDevice;
    mitkClassMacro(PolhemusTool, InternalTrackingTool);

  protected:
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    PolhemusTool();
    virtual ~PolhemusTool();
  };
}//mitk
#endif // MITKPolhemusTOOL_H_HEADER_INCLUDED_
