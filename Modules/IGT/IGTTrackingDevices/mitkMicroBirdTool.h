/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-07-10 17:24:20 +0200 (Di, 10 Jul 2007) $
Version:   $Revision: 11220 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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
