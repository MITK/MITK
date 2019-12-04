/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkUnstructuredGridVtkWriter.h"
#include "mitkUnstructuredGridVtkWriter.txx"

namespace mitk
{
  template <>
  std::vector<std::string> UnstructuredGridVtkWriter<vtkUnstructuredGridWriter>::GetPossibleFileExtensions()
  {
    std::vector<std::string> possibleFileExtensions;
    possibleFileExtensions.push_back(".vtk");
    return possibleFileExtensions;
  }

  template <>
  std::vector<std::string> UnstructuredGridVtkWriter<vtkXMLUnstructuredGridWriter>::GetPossibleFileExtensions()
  {
    std::vector<std::string> possibleFileExtensions;
    possibleFileExtensions.push_back(".vtu");
    return possibleFileExtensions;
  }

  template <>
  std::vector<std::string> UnstructuredGridVtkWriter<vtkXMLPUnstructuredGridWriter>::GetPossibleFileExtensions()
  {
    std::vector<std::string> possibleFileExtensions;
    possibleFileExtensions.push_back(".pvtu");
    return possibleFileExtensions;
  }

  template <>
  const char *UnstructuredGridVtkWriter<vtkUnstructuredGridWriter>::GetDefaultFilename()
  {
    return "vtkUnstructuredGrid.vtk";
  }

  template <>
  const char *UnstructuredGridVtkWriter<vtkXMLUnstructuredGridWriter>::GetDefaultFilename()
  {
    return "vtkUnstructuredGrid.vtu";
  }

  template <>
  const char *UnstructuredGridVtkWriter<vtkXMLPUnstructuredGridWriter>::GetDefaultFilename()
  {
    return "vtkUnstructuredGrid.pvtu";
  }

  template <>
  const char *UnstructuredGridVtkWriter<vtkUnstructuredGridWriter>::GetFileDialogPattern()
  {
    return "VTK Legacy Unstructured Grid (*.vtk)";
  }

  template <>
  const char *UnstructuredGridVtkWriter<vtkXMLUnstructuredGridWriter>::GetFileDialogPattern()
  {
    return "VTK XML Unstructured Grid (*.vtu)";
  }

  template <>
  const char *UnstructuredGridVtkWriter<vtkXMLPUnstructuredGridWriter>::GetFileDialogPattern()
  {
    return "VTK Parallel XML Unstructured Grid (*.pvtu)";
  }

  template <>
  const char *UnstructuredGridVtkWriter<vtkUnstructuredGridWriter>::GetDefaultExtension()
  {
    return ".vtk";
  }

  template <>
  const char *UnstructuredGridVtkWriter<vtkXMLUnstructuredGridWriter>::GetDefaultExtension()
  {
    return ".vtu";
  }

  template <>
  const char *UnstructuredGridVtkWriter<vtkXMLPUnstructuredGridWriter>::GetDefaultExtension()
  {
    return ".pvtu";
  }

  template class UnstructuredGridVtkWriter<vtkUnstructuredGridWriter>;
  template class UnstructuredGridVtkWriter<vtkXMLUnstructuredGridWriter>;
  template class UnstructuredGridVtkWriter<vtkXMLPUnstructuredGridWriter>;
}
