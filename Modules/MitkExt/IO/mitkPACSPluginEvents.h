/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 14120 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkPACSPluginEventshincluded
#define mitkPACSPluginEventshincluded

#include <itkObject.h>
#include <itkEventObject.h>

namespace mitk
{
  itkEventMacro( PluginEvent, itk::AnyEvent );
  itkEventMacro( PluginStudySelected, PluginEvent );
  itkEventMacro( PluginLightBoxCountChanged, PluginEvent );
  itkEventMacro( PluginAbortPACSImport, PluginEvent );
}

#endif

