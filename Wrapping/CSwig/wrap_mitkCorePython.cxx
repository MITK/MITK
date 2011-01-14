#define MITK_WRAP_PACKAGE "mitkCorePython"
//#include "wrap_MITKAlgorithms.cxx"
/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: wrap_ITKAlgorithms.cxx,v $
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifdef CABLE_CONFIGURATION

namespace _cable_
{
  const char* const package = MITK_WRAP_PACKAGE;
  const char* const groups[] =
  {
    "BaseController",
    "CallbackFromGUIThread",
    "CameraController",
    "CameraRotationController",
    "FocusManager",
    "LimitedLinearUndo",
    "OperationEvent",
    "ProgressBar",
    "ProgressBarImplementation",
    //"ReferenceCountWatcher",
    "RenderingManager",
    "RenderingManagerFactory",
    "SliceNavigationController",
    "SlicesCoordinator",
    "SlicesRotator",
    "SlicesSwiveller",
    "StatusBar",
    "StatusBarImplementation",
    "Stepper",
    //"TestingMacros",
    "TestManager",
    "UndoController",
    "UndoModel",
    "VerboseLimitedLinearUndo",
    "VtkInteractorCameraController",
    "VtkLayerController"
  };
}
#endif
