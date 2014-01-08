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


#ifndef MITKMICROBIRDTOOL_H_HEADER_INCLUDED_
#define MITKMICROBIRDTOOL_H_HEADER_INCLUDED_

#include <mitkInternalTrackingTool.h>
//#include "mitkTrackingTypes.h"

namespace mitk {
    //##Documentation
    //## \brief Implementation of a MicroBird tool
    //##
    //##
    //## \ingroup IGT

    class MitkIGT_EXPORT MicroBirdTool : public InternalTrackingTool
    {
    public:
      mitkClassMacro(MicroBirdTool, InternalTrackingTool);
      itkNewMacro(Self);
    protected:
      MicroBirdTool();
      virtual ~MicroBirdTool();
    };
} // namespace mitk

#endif /* MITKMICROBIRDTOOL_H_HEADER_INCLUDED_ */
