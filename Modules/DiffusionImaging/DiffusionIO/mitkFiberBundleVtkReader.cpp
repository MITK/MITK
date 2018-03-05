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

#include "mitkFiberBundleVtkReader.h"
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
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLDataReader.h>
#include "mitkDiffusionIOMimeTypes.h"


mitk::FiberBundleVtkReader::FiberBundleVtkReader()
  : mitk::AbstractFileReader( mitk::DiffusionIOMimeTypes::FIBERBUNDLE_VTK_MIMETYPE_NAME(), "VTK Fiber Bundle Reader" )
{
  m_ServiceReg = this->RegisterService();
}

mitk::FiberBundleVtkReader::FiberBundleVtkReader(const FiberBundleVtkReader &other)
  :mitk::AbstractFileReader(other)
{
}

mitk::FiberBundleVtkReader * mitk::FiberBundleVtkReader::Clone() const
{
  return new FiberBundleVtkReader(*this);
}


std::vector<itk::SmartPointer<mitk::BaseData> > mitk::FiberBundleVtkReader::Read()
{

  std::vector<itk::SmartPointer<mitk::BaseData> > result;

  const std::string& locale = "C";
  const std::string& currLocale = setlocale( LC_ALL, nullptr );
  setlocale(LC_ALL, locale.c_str());

  std::string filename = this->GetInputLocation();

  std::string ext = itksys::SystemTools::GetFilenameLastExtension(filename);
  ext = itksys::SystemTools::LowerCase(ext);

  try
  {
    MITK_INFO << "Loading tractogram (VTK format): " << itksys::SystemTools::GetFilenameName(filename);
    vtkSmartPointer<vtkPolyDataReader> reader = vtkSmartPointer<vtkPolyDataReader>::New();
    reader->SetFileName( this->GetInputLocation().c_str() );

    if (reader->IsFilePolyData())
    {
      reader->Update();

      if ( reader->GetOutput() != nullptr )
      {
        vtkSmartPointer<vtkPolyData> fiberPolyData = reader->GetOutput();
        FiberBundle::Pointer fiberBundle = FiberBundle::New(fiberPolyData);

        vtkSmartPointer<vtkFloatArray> weights = vtkFloatArray::SafeDownCast(fiberPolyData->GetCellData()->GetArray("FIBER_WEIGHTS"));
        if (weights!=nullptr)
        {
          //                    float weight=0;
          for (int i=0; i<weights->GetNumberOfValues(); i++)
          {
            if (weights->GetValue(i)<0.0)
            {
              MITK_ERROR << "Fiber weight<0 detected! Setting value to 0.";
              weights->SetValue(i,0);
            }
          }
          //                        if (!mitk::Equal(weights->GetValue(i),weight,0.00001))
          //                        {
          //                            MITK_INFO << "Weight: " << weights->GetValue(i);
          //                            weight = weights->GetValue(i);
          //                        }
          fiberBundle->SetFiberWeights(weights);
        }

        vtkSmartPointer<vtkUnsignedCharArray> fiberColors = vtkUnsignedCharArray::SafeDownCast(fiberPolyData->GetPointData()->GetArray("FIBER_COLORS"));
        if (fiberColors!=nullptr)
          fiberBundle->SetFiberColors(fiberColors);

        result.push_back(fiberBundle.GetPointer());
        return result;
      }
    }
    else
      MITK_INFO << "File is not VTK format.";
  }
  catch(...)
  {
    throw;
  }

  try
  {
    MITK_INFO << "Trying to load fiber file as VTP format.";
    vtkSmartPointer<vtkXMLPolyDataReader> reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
    reader->SetFileName( this->GetInputLocation().c_str() );

    if ( reader->CanReadFile(this->GetInputLocation().c_str()) )
    {
      reader->Update();

      if ( reader->GetOutput() != nullptr )
      {
        vtkSmartPointer<vtkPolyData> fiberPolyData = reader->GetOutput();
        FiberBundle::Pointer fiberBundle = FiberBundle::New(fiberPolyData);

        vtkSmartPointer<vtkFloatArray> weights = vtkFloatArray::SafeDownCast(fiberPolyData->GetCellData()->GetArray("FIBER_WEIGHTS"));

        if (weights!=nullptr)
        {
          //                                float weight=0;
          //                                for (int i=0; i<weights->GetSize(); i++)
          //                                    if (!mitk::Equal(weights->GetValue(i),weight,0.00001))
          //                                    {
          //                                        MITK_INFO << "Weight: " << weights->GetValue(i);
          //                                        weight = weights->GetValue(i);
          //                                    }
          fiberBundle->SetFiberWeights(weights);
        }

        vtkSmartPointer<vtkUnsignedCharArray> fiberColors = vtkUnsignedCharArray::SafeDownCast(fiberPolyData->GetPointData()->GetArray("FIBER_COLORS"));
        if (fiberColors!=nullptr)
          fiberBundle->SetFiberColors(fiberColors);

        result.push_back(fiberBundle.GetPointer());
        return result;
      }
    }
    else
      MITK_INFO << "File is not VTP format.";
  }
  catch(...)
  {
    throw;
  }

  try
  {
    MITK_INFO << "Trying to load fiber file as deprecated XML format.";
    vtkSmartPointer<vtkPolyData> fiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkCellArray> cellArray = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkPoints>    points = vtkSmartPointer<vtkPoints>::New();
    TiXmlDocument doc( this->GetInputLocation().c_str() );
    if(doc.LoadFile())
    {
      TiXmlHandle hDoc(&doc);
      TiXmlElement* pElem;
      TiXmlHandle hRoot(nullptr);
      pElem = hDoc.FirstChildElement().Element();
      // save this for later
      hRoot = TiXmlHandle(pElem);
      pElem = hRoot.FirstChildElement("geometry").Element();
      // read geometry
      mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
      // read origin
      mitk::Point3D origin;
      double temp = 0;
      pElem->Attribute("origin_x", &temp);
      origin[0] = temp;
      pElem->Attribute("origin_y", &temp);
      origin[1] = temp;
      pElem->Attribute("origin_z", &temp);
      origin[2] = temp;
      geometry->SetOrigin(origin);
      // read spacing
      ScalarType spacing[3];
      pElem->Attribute("spacing_x", &temp);
      spacing[0] = temp;
      pElem->Attribute("spacing_y", &temp);
      spacing[1] = temp;
      pElem->Attribute("spacing_z", &temp);
      spacing[2] = temp;
      geometry->SetSpacing(spacing);
      // read transform
      vtkMatrix4x4* m = vtkMatrix4x4::New();
      pElem->Attribute("xx", &temp);
      m->SetElement(0,0,temp);
      pElem->Attribute("xy", &temp);
      m->SetElement(1,0,temp);
      pElem->Attribute("xz", &temp);
      m->SetElement(2,0,temp);
      pElem->Attribute("yx", &temp);
      m->SetElement(0,1,temp);
      pElem->Attribute("yy", &temp);
      m->SetElement(1,1,temp);
      pElem->Attribute("yz", &temp);
      m->SetElement(2,1,temp);
      pElem->Attribute("zx", &temp);
      m->SetElement(0,2,temp);
      pElem->Attribute("zy", &temp);
      m->SetElement(1,2,temp);
      pElem->Attribute("zz", &temp);
      m->SetElement(2,2,temp);
      m->SetElement(0,3,origin[0]);
      m->SetElement(1,3,origin[1]);
      m->SetElement(2,3,origin[2]);
      m->SetElement(3,3,1);
      geometry->SetIndexToWorldTransformByVtkMatrix(m);
      // read bounds
      float bounds[] = {0, 0, 0, 0, 0, 0};
      pElem->Attribute("size_x", &temp);
      bounds[1] = temp;
      pElem->Attribute("size_y", &temp);
      bounds[3] = temp;
      pElem->Attribute("size_z", &temp);
      bounds[5] = temp;
      geometry->SetFloatBounds(bounds);
      geometry->SetImageGeometry(true);
      pElem = hRoot.FirstChildElement("fiber_bundle").FirstChild().Element();
      for( ; pElem ; pElem=pElem->NextSiblingElement())
      {
        TiXmlElement* pElem2 = pElem->FirstChildElement();
        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
        for( ; pElem2; pElem2=pElem2->NextSiblingElement())
        {
          Point3D point;
          pElem2->Attribute("pos_x", &temp);
          point[0] = temp;
          pElem2->Attribute("pos_y", &temp);
          point[1] = temp;
          pElem2->Attribute("pos_z", &temp);
          point[2] = temp;
          geometry->IndexToWorld(point, point);
          vtkIdType id = points->InsertNextPoint(point.GetDataPointer());
          container->GetPointIds()->InsertNextId(id);
        }
        cellArray->InsertNextCell(container);
      }
      fiberPolyData->SetPoints(points);
      fiberPolyData->SetLines(cellArray);
      vtkSmartPointer<vtkCleanPolyData> cleaner = vtkSmartPointer<vtkCleanPolyData>::New();
      cleaner->SetInputData(fiberPolyData);
      cleaner->Update();
      fiberPolyData = cleaner->GetOutput();
      FiberBundle::Pointer image = FiberBundle::New(fiberPolyData);
      result.push_back(image.GetPointer());
      return result;
    }
    else
    {
      MITK_INFO << "File is not deprectaed XML format.";
    }

    setlocale(LC_ALL, currLocale.c_str());
    MITK_INFO << "Fiber bundle read";
  }
  catch(...)
  {
    throw;
  }

  throw "Selected file is no vtk readable fiber format (binary or ascii vtk or vtp file).";

  return result;
}
