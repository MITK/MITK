/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
