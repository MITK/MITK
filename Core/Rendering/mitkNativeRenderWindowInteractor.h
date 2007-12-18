/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#ifndef MITKNATIVERENDERWINDOWINTERACTOR_H_HEADER_INCLUDED_C1C53722
#define MITKNATIVERENDERWINDOWINTERACTOR_H_HEADER_INCLUDED_C1C53722

#include "mitkCommon.h"
#include "itkObject.h"
#include "itkObjectFactory.h"

class vtkRenderWindow;
class vtkRenderWindowInteractor;

namespace mitk
{

class NativeRenderWindowInteractor : public itk::Object
{
public:
  mitkClassMacro(NativeRenderWindowInteractor, itk::Object);
  
  itkNewMacro(Self);

  virtual void Start();

  void SetMitkRenderWindow(vtkRenderWindow * renderwindow);
  itkGetMacro(MitkRenderWindow, vtkRenderWindow*);

protected:
  NativeRenderWindowInteractor();
  virtual ~NativeRenderWindowInteractor();

  vtkRenderWindow* m_MitkRenderWindow;

  vtkRenderWindowInteractor* m_NativeVtkRenderWindowInteractor;
};

}
#endif /* MITKNATIVERENDERWINDOWINTERACTOR_H_HEADER_INCLUDED_C1C53722 */

