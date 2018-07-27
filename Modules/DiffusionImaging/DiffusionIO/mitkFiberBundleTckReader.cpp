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

#include "mitkFiberBundleTckReader.h"
#include <itkMetaDataObject.h>
#include <vtkPolyData.h>
#include <vtkDataReader.h>
#include <vtkPolyDataReader.h>
#include <vtkMatrix4x4.h>
#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <vtkDataArray.h>
#include <vtkFloatArray.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <itksys/SystemTools.hxx>
#include <tinyxml.h>
#include <vtkCleanPolyData.h>
#include <mitkTrackvis.h>
#include <mitkCustomMimeType.h>
#include "mitkDiffusionIOMimeTypes.h"
#include <vtkTransformPolyDataFilter.h>
#include <mitkLexicalCast.h>


mitk::FiberBundleTckReader::FiberBundleTckReader()
  : mitk::AbstractFileReader( mitk::DiffusionIOMimeTypes::FIBERBUNDLE_TCK_MIMETYPE_NAME(), "tck Fiber Bundle Reader (MRtrix format)" )
{
  m_ServiceReg = this->RegisterService();
}

mitk::FiberBundleTckReader::FiberBundleTckReader(const FiberBundleTckReader &other)
  :mitk::AbstractFileReader(other)
{
}

mitk::FiberBundleTckReader * mitk::FiberBundleTckReader::Clone() const
{
  return new FiberBundleTckReader(*this);
}

std::vector<itk::SmartPointer<mitk::BaseData> > mitk::FiberBundleTckReader::Read()
{

  std::vector<itk::SmartPointer<mitk::BaseData> > result;
  try
  {
    const std::string& locale = "C";
    const std::string& currLocale = setlocale( LC_ALL, nullptr );
    setlocale(LC_ALL, locale.c_str());

    std::string filename = this->GetInputLocation();

    std::string ext = itksys::SystemTools::GetFilenameLastExtension(filename);
    ext = itksys::SystemTools::LowerCase(ext);

    if (ext==".tck")
    {
      MITK_INFO << "Loading tractogram (MRtrix format): " << itksys::SystemTools::GetFilenameName(filename);
      std::FILE* filePointer = std::fopen(filename.c_str(),"r+b");
      std::string header = "";

      bool header_end = false;
      int count = 0;
      while (!header_end)
      {
        char c;
        count += std::fread(&c, 1, 1, filePointer);
        header += c;
        if (header.size() >= 3 && header.compare(header.size() - 3, 3, "END") == 0)
          header_end = true;
      }
      MITK_INFO << "TCK Header:";
      MITK_INFO << header;

      int header_size = -1;
      try
      {
        std::string delimiter = "file: . ";
        size_t pos = 0;
        pos = header.find(delimiter);
        header.erase(0, pos + delimiter.length());
        pos = header.find("\n");
        header_size = boost::lexical_cast<int>(header.substr(0, pos));
      }
      catch(...)
      {

      }

      if (header_size==-1)
        mitkThrow() << "Could not parse header size from " << filename;
      std::fseek ( filePointer , header_size , SEEK_SET );

      vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
      vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();
      vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();

      float tmp[3];
      count = 1;
      int fiber_length = 0;
      while (std::fread((char*)tmp, 1, 12, filePointer)==12)
      {
        if (std::isinf(tmp[0]) || std::isinf(tmp[1]) || std::isinf(tmp[1]))
          break;
        else if (std::isnan(tmp[0]) || std::isnan(tmp[1]) || std::isnan(tmp[2]))
        {
          count++;
          fiber_length = 0;
          vtkNewCells->InsertNextCell(container);
          container = vtkSmartPointer<vtkPolyLine>::New();
        }
        else
        {
          fiber_length++;
          vtkIdType id = vtkNewPoints->InsertNextPoint(tmp);
          container->GetPointIds()->InsertNextId(id);
        }
      }

      vtkSmartPointer<vtkPolyData> fiberPolyData = vtkSmartPointer<vtkPolyData>::New();
      fiberPolyData->SetPoints(vtkNewPoints);
      fiberPolyData->SetLines(vtkNewCells);

      // transform polydata from RAS (MRtrix) to LPS (MITK)
      mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
      vtkSmartPointer< vtkMatrix4x4 > matrix = vtkSmartPointer< vtkMatrix4x4 >::New();
      matrix->Identity();
      matrix->SetElement(0,0,-matrix->GetElement(0,0));
      matrix->SetElement(1,1,-matrix->GetElement(1,1));
      geometry->SetIndexToWorldTransformByVtkMatrix(matrix);

      vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      transformFilter->SetInputData(fiberPolyData);
      transformFilter->SetTransform(geometry->GetVtkTransform());
      transformFilter->Update();

      FiberBundle::Pointer fib = FiberBundle::New(transformFilter->GetOutput());
      result.push_back(fib.GetPointer());
    }

    setlocale(LC_ALL, currLocale.c_str());
    MITK_INFO << "Fiber bundle read";
    return result;
  }
  catch(...)
  {
    throw;
  }
  return result;
}
