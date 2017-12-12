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

#include "mitkFiberBundleVtkWriter.h"
#include <vtkSmartPointer.h>
#include <vtkCleanPolyData.h>
#include <itksys/SystemTools.hxx>
#include <mitkTrackvis.h>
#include <itkSize.h>
#include <vtkFloatArray.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <mitkAbstractFileWriter.h>
#include <mitkCustomMimeType.h>
#include <vtkXMLPolyDataWriter.h>
#include "mitkDiffusionIOMimeTypes.h"

mitk::FiberBundleVtkWriter::FiberBundleVtkWriter()
  : mitk::AbstractFileWriter(mitk::FiberBundle::GetStaticNameOfClass(), mitk::DiffusionIOMimeTypes::FIBERBUNDLE_VTK_MIMETYPE_NAME(), "VTK Fiber Bundle Writer")
{
  Options defaultOptions;
  defaultOptions["Save as binary file"] = true;
  defaultOptions["Save as xml file (vtp style)"] = false;
  defaultOptions["Save color information"] = true;
  defaultOptions["Save fiber weights"] = true;
  this->SetDefaultOptions(defaultOptions);
  RegisterService();
}

mitk::FiberBundleVtkWriter::FiberBundleVtkWriter(const mitk::FiberBundleVtkWriter & other)
  :mitk::AbstractFileWriter(other)
{}

mitk::FiberBundleVtkWriter::~FiberBundleVtkWriter()
{}

mitk::FiberBundleVtkWriter * mitk::FiberBundleVtkWriter::Clone() const
{
  return new mitk::FiberBundleVtkWriter(*this);
}

void mitk::FiberBundleVtkWriter::Write()
{

  std::ostream* out;
  std::ofstream outStream;

  if( this->GetOutputStream() )
  {
    out = this->GetOutputStream();
  }else{
    outStream.open( this->GetOutputLocation().c_str() );
    out = &outStream;
  }

  if ( !out->good() )
  {
    mitkThrow() << "Stream not good.";
  }

  try
  {
    const std::string& locale = "C";
    const std::string& currLocale = setlocale( LC_ALL, nullptr );
    setlocale(LC_ALL, locale.c_str());

    std::locale I("C");
    out->imbue(I);

    std::string filename = this->GetOutputLocation().c_str();

    mitk::FiberBundle::ConstPointer input = dynamic_cast<const mitk::FiberBundle*>(this->GetInput());
    std::string ext = itksys::SystemTools::GetFilenameLastExtension(this->GetOutputLocation().c_str());
    Options options = this->GetOptions();

    vtkSmartPointer<vtkFloatArray> weights = nullptr;
    vtkSmartPointer<vtkUnsignedCharArray> fiberColors = nullptr;

    vtkSmartPointer<vtkPolyData> fibPoly = input->GetFiberPolyData();
    if (us::any_cast<bool>(options["Save fiber weights"]))
    {
      MITK_INFO << "Adding fiber weight information";
      fibPoly->GetCellData()->AddArray(input->GetFiberWeights());
    }
    else if (fibPoly->GetCellData()->HasArray("FIBER_WEIGHTS"))
    {
      weights = input->GetFiberWeights();
      fibPoly->GetCellData()->RemoveArray("FIBER_WEIGHTS");
    }

    if (us::any_cast<bool>(options["Save color information"]))
    {
      MITK_INFO << "Adding color information";
      fibPoly->GetPointData()->AddArray(input->GetFiberColors());
    }
    else if (fibPoly->GetPointData()->HasArray("FIBER_COLORS"))
    {
      fiberColors = input->GetFiberColors();
      fibPoly->GetPointData()->RemoveArray("FIBER_COLORS");
    }

    // default extension is .fib
    if(ext == "")
    {
      ext = ".fib";
      this->SetOutputLocation(this->GetOutputLocation() + ext);
    }

    if (us::any_cast<bool>(options["Save as xml file (vtp style)"]))
    {
      vtkSmartPointer<vtkXMLPolyDataWriter> writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
      writer->SetInputData(fibPoly);
      writer->SetFileName(filename.c_str());
      if (us::any_cast<bool>(options["Save as binary file"]))
      {
        MITK_INFO << "Writing fiber bundle as vtk binary file";
        writer->SetDataModeToBinary();
      }
      else
      {
        MITK_INFO << "Writing fiber bundle as vtk ascii file";
        writer->SetDataModeToAscii();
      }
      writer->Write();
    }
    else
    {
      vtkSmartPointer<vtkPolyDataWriter> writer = vtkSmartPointer<vtkPolyDataWriter>::New();
      writer->SetInputData(fibPoly);
      writer->SetFileName(filename.c_str());
      if (us::any_cast<bool>(options["Save as binary file"]))
      {
        MITK_INFO << "Writing fiber bundle as vtk binary file";
        writer->SetFileTypeToBinary();
      }
      else
      {
        MITK_INFO << "Writing fiber bundle as vtk ascii file";
        writer->SetFileTypeToASCII();
      }
      writer->Write();
    }

    // restore arrays
    if ( weights!=nullptr )
      fibPoly->GetPointData()->AddArray(weights);

    if (fiberColors != nullptr)
      fibPoly->GetPointData()->AddArray(fiberColors);

    setlocale(LC_ALL, currLocale.c_str());
    MITK_INFO << "VTK Fiber bundle written to " << filename;
  }
  catch(...)
  {
    throw;
  }
}
