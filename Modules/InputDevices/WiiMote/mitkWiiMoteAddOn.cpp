/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
Version:   $Revision: 21975 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <mitkGlobalInteraction.h>
#include <mitkInteractionConst.h>
#include <mitkVector.h>

#include <mitkWiiMoteAddOn.h>

mitk::WiiMoteAddOn* mitk::WiiMoteAddOn::GetInstance()
{
  static WiiMoteAddOn instance;
  return &instance;
}

void mitk::WiiMoteAddOn::ForwardEvent(const mitk::StateEvent *e)
{
  mitk::GlobalInteraction::GetInstance()->HandleEvent(e);
}




