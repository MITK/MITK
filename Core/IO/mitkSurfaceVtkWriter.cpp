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

#include "mitkSurfaceVtkWriter.h"
#include "mitkSurfaceVtkWriter.txx"

#include <vtkSTLWriter.h>
#include <vtkPolyDataWriter.h>
#include <vtkXMLPolyDataWriter.h>

namespace mitk {

template<>
void SurfaceVtkWriter<vtkSTLWriter>::SetDefaultExtension()
{
  m_Extension = ".stl";
  m_WriterWriteHasReturnValue = false;
}

template<>
void SurfaceVtkWriter<vtkPolyDataWriter>::SetDefaultExtension()
{
  m_Extension = ".vtk";
  m_WriterWriteHasReturnValue = false;
}

template<>
void SurfaceVtkWriter<vtkXMLPolyDataWriter>::SetDefaultExtension()
{
  m_Extension = ".vtp";
  m_WriterWriteHasReturnValue = true;
}

template<>
void SurfaceVtkWriter<vtkXMLPolyDataWriter>::ExecuteWrite( VtkWriterType* m_VtkWriter, vtkTransformPolyDataFilter* transformPolyData )
{
  if (!m_VtkWriter->Write())
  {
    transformPolyData->Delete();
    throw std::ios_base::failure("Error during surface writing.");
  }
}



template class MITK_CORE_EXPORT SurfaceVtkWriter<vtkSTLWriter>;

template class MITK_CORE_EXPORT SurfaceVtkWriter<vtkPolyDataWriter>;

template class MITK_CORE_EXPORT SurfaceVtkWriter<vtkXMLPolyDataWriter>;

}


