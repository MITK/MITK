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

#ifndef MITKOpenIGTLinkTrackingTOOL_H_HEADER_INCLUDED_
#define MITKOpenIGTLinkTrackingTOOL_H_HEADER_INCLUDED_

#include <mitkTrackingTool.h>
#include <itkFastMutexLock.h>

namespace mitk
{
  class OpenIGTLinkTrackingDevice;
  /** Documentation:
  *   \brief  An object of this class represents a OpenIGTLink tracking tool.
  *           A tool has to be added to a tracking device which will then
  *           continuously update the tool coordinates.
  *   \ingroup IGT
  */
  class MITKIGT_EXPORT OpenIGTLinkTrackingTool : public TrackingTool
  {
  public:
    friend class OpenIGTLinkTrackingTrackingDevice;
    mitkClassMacro(OpenIGTLinkTrackingTool, TrackingTool);

    itkFactorylessNewMacro(Self)
  protected:

    itkCloneMacro(Self)
    OpenIGTLinkTrackingTool();
    ~OpenIGTLinkTrackingTool() override;
  };
}//mitk
#endif // MITKOpenIGTLinkTrackingTOOL_H_HEADER_INCLUDED_
