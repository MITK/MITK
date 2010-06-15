/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Do, 28 Mai 2009) $
Version:   $Revision $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNavigationTool.h"

mitk::NavigationTool::NavigationTool()
  {
    m_Type = mitk::NavigationTool::Unknown;
    m_Identifier = "None";
    m_TrackingDeviceType = mitk::TrackingSystemNotSpecified;
  }
 
mitk::NavigationTool::~NavigationTool()
  {

  }