/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkSurfaceStlIO.h"

#include "mitkSurface.h"
#include "mitkIOMimeTypes.h"

#include <vtkSTLReader.h>
#include <vtkSTLWriter.h>
#include <vtkTriangleFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkCleanPolyData.h>
#include <vtkErrorCode.h>
#include <vtkSmartPointer.h>

namespace mitk {

std::string SurfaceStlIO::OPTION_MERGE_POINTS()
{
  static std::string s = "Merge points";
  return s;
}

std::string SurfaceStlIO::OPTION_TAG_SOLIDS()
{
  static std::string s = "Tag solids";
  return s;
}

std::string SurfaceStlIO::OPTION_CLEAN()
{
  static std::string s = "Clean poly data";
  return s;
}

SurfaceStlIO::SurfaceStlIO()
  : SurfaceVtkIO(Surface::GetStaticNameOfClass(), IOMimeTypes::STEREOLITHOGRAPHY_MIMETYPE(), "Stereolithography")
{
  Options defaultOptions;

  defaultOptions[OPTION_MERGE_POINTS()] = us::Any(true);
  defaultOptions[OPTION_TAG_SOLIDS()] = us::Any(false);
  defaultOptions[OPTION_CLEAN()] = us::Any(true);

  this->SetDefaultReaderOptions(defaultOptions);

  this->RegisterService();
}

std::vector<itk::SmartPointer<BaseData> > SurfaceStlIO::Read()
{
  Options options = this->GetReaderOptions();

  mitk::Surface::Pointer output = mitk::Surface::New();

  vtkSmartPointer<vtkSTLReader> stlReader = vtkSmartPointer<vtkSTLReader>::New();
  stlReader->SetFileName(this->GetLocalFileName().c_str());

  bool mergePoints = true;
  bool tagSolids = false;
  bool cleanData = true;

  try
  {
    mergePoints = us::any_cast<bool>(options[OPTION_MERGE_POINTS()]);
    tagSolids = us::any_cast<bool>(options[OPTION_TAG_SOLIDS()]);
    cleanData = us::any_cast<bool>(options[OPTION_CLEAN()]);
  }
  catch (const us::BadAnyCastException& e)
  {
    MITK_WARN << "Unexpected error: " << e.what();
  }

  stlReader->SetMerging(mergePoints);
  stlReader->SetScalarTags(tagSolids);

  vtkSmartPointer<vtkPolyDataNormals> normalsGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
  normalsGenerator->SetInputConnection(stlReader->GetOutputPort());

  vtkSmartPointer<vtkPolyDataAlgorithm> algo = normalsGenerator;
  if (cleanData)
  {
    vtkSmartPointer<vtkCleanPolyData> cleanPolyDataFilter = vtkSmartPointer<vtkCleanPolyData>::New();
    cleanPolyDataFilter->SetInputConnection(normalsGenerator->GetOutputPort());
    cleanPolyDataFilter->PieceInvariantOff();
    cleanPolyDataFilter->ConvertLinesToPointsOff();
    cleanPolyDataFilter->ConvertPolysToLinesOff();
    cleanPolyDataFilter->ConvertStripsToPolysOff();
    if (mergePoints)
    {
      cleanPolyDataFilter->PointMergingOn();
    }
    algo = cleanPolyDataFilter;
  }
  algo->Update();

  if (algo->GetOutput() != NULL)
  {
    vtkSmartPointer<vtkPolyData> surfaceWithNormals = algo->GetOutput();
    output->SetVtkPolyData(surfaceWithNormals);
  }

  std::vector<BaseData::Pointer> result;
  result.push_back(output.GetPointer());
  return result;
}

void SurfaceStlIO::Write()
{
  ValidateOutputLocation();

  const Surface* input = dynamic_cast<const Surface*>(this->GetInput());

  const unsigned int timesteps = input->GetTimeGeometry()->CountTimeSteps();
  for(unsigned int t = 0; t < timesteps; ++t)
  {
    std::string fileName;
    vtkSmartPointer<vtkPolyData> polyData = this->GetPolyData(t, fileName);
    vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
    triangleFilter->SetInputData(polyData);
    vtkSmartPointer<vtkSTLWriter> writer = vtkSmartPointer<vtkSTLWriter>::New();
    writer->SetInputConnection(triangleFilter->GetOutputPort());

    // The vtk stl writer cannot write to streams
    LocalFile localFile(this);
    writer->SetFileName(localFile.GetFileName().c_str());

    if (writer->Write() == 0 || writer->GetErrorCode() != 0 )
    {
      mitkThrow() << "Error during surface writing" << (writer->GetErrorCode() ?
                                                          std::string(": ") + vtkErrorCode::GetStringFromErrorCode(writer->GetErrorCode()) :
                                                          std::string());
    }

    if (this->GetOutputStream() && input->GetTimeGeometry()->CountTimeSteps() > 1)
    {
      MITK_WARN << "Writing multiple time-steps to output streams is not supported. "
                << "Only the first time-step will be written";
      break;
    }
  }
}

SurfaceStlIO* SurfaceStlIO::IOClone() const
{
  return new SurfaceStlIO(*this);
}

}
