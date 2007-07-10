/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkChiliPluginEventshincluded
#define mitkChiliPluginEventshincluded

#include <itkObject.h>
#include <itkEventObject.h>

namespace mitk
{

itkEventMacro( PluginEvent, itk::AnyEvent );
itkEventMacro( PluginStudySelected, PluginEvent );
itkEventMacro( PluginLightBoxCountChanged, PluginEvent );

}

#endif

