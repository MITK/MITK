/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "mitkSurfaceVtkWriter.h"
#include "mitkSurfaceVtkWriter.txx"

#include <vtkSTLWriter.h>
#include <vtkPolyDataWriter.h>

namespace mitk {

template<>
void SurfaceVtkWriter<vtkSTLWriter>::SetDefaultExtension()
{
  m_Extension = ".stl";
}

template<>
void SurfaceVtkWriter<vtkPolyDataWriter>::SetDefaultExtension()
{
  m_Extension = ".vtk";
}

template class SurfaceVtkWriter<vtkSTLWriter>;

template class SurfaceVtkWriter<vtkPolyDataWriter>;

}

