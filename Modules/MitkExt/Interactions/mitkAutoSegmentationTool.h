/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitk.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.0 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkAutoSegmentationTool_h_Included
#define mitkAutoSegmentationTool_h_Included

#include "mitkCommon.h"
#include "mitkTool.h"

namespace mitk
{

/**
  \brief Superclass for tool that create a new segmentation without user interaction in render windows

  This class is undocumented. Ask the creator ($Author$) to supply useful comments.
*/
class MITKEXT_CORE_EXPORT AutoSegmentationTool : public Tool
{
  public:
    
    mitkClassMacro(AutoSegmentationTool, Tool);

  protected:

    AutoSegmentationTool(); // purposely hidden
    virtual ~AutoSegmentationTool();
    
    virtual const char* GetGroup() const;
 };

} // namespace

#endif

