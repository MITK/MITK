/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkMicroBirdTool_h
#define mitkMicroBirdTool_h

#include <mitkTrackingTool.h>

namespace mitk {
    //##Documentation
    //## \brief Implementation of a MicroBird tool
    //##
    //##
    //## \ingroup IGT

    class MITKIGT_EXPORT MicroBirdTool : public TrackingTool
    {
    public:
      mitkClassMacro(MicroBirdTool, TrackingTool);
      itkFactorylessNewMacro(Self);
      itkCloneMacro(Self)
    protected:
      MicroBirdTool();
      virtual ~MicroBirdTool();
    };
} // namespace mitk

#endif
