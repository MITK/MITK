/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkFiberBundleXReader.h"
#include <itkMetaDataObject.h>
#include <vtkPolyData.h>
#include <vtkDataReader.h>
#include <vtkPolyDataReader.h>
#include <vtkMatrix4x4.h>
#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <itksys/SystemTools.hxx>
#include <tinyxml.h>

namespace mitk
{

  void FiberBundleXReader
    ::GenerateData()
  {
    if ( ( ! m_OutputCache ) )
    {
      Superclass::SetNumberOfRequiredOutputs(0);
      this->GenerateOutputInformation();
    }

    if (!m_OutputCache)
    {
      itkWarningMacro("Output cache is empty!");
    }


    Superclass::SetNumberOfRequiredOutputs(1);
    Superclass::SetNthOutput(0, m_OutputCache.GetPointer());
  }

  void FiberBundleXReader::GenerateOutputInformation()
  {
    try
    {
      const std::string& locale = "C";
      const std::string& currLocale = setlocale( LC_ALL, NULL );
      setlocale(LC_ALL, locale.c_str());

      std::string ext = itksys::SystemTools::GetFilenameLastExtension(m_FileName);
      ext = itksys::SystemTools::LowerCase(ext);

      vtkSmartPointer<vtkDataReader> chooser=vtkDataReader::New();
      chooser->SetFileName(m_FileName.c_str() );
      if( chooser->IsFilePolyData())
      {
        MITK_INFO << "Reading vtk fiber bundle";
        vtkSmartPointer<vtkPolyDataReader> reader = vtkPolyDataReader::New();
        reader->SetFileName( m_FileName.c_str() );
        reader->Update();

        if ( reader->GetOutput() != NULL )
        {
          vtkSmartPointer<vtkPolyData> fiberPolyData = reader->GetOutput();
          m_OutputCache = OutputType::New(fiberPolyData);
        }
      }
      else // try to read deprecated fiber bundle file format
      {
        MITK_INFO << "Reading xml fiber bundle";

        vtkSmartPointer<vtkPolyData> fiberPolyData = vtkSmartPointer<vtkPolyData>::New();
        vtkSmartPointer<vtkCellArray> cellArray = vtkSmartPointer<vtkCellArray>::New();
        vtkSmartPointer<vtkPoints>    points = vtkSmartPointer<vtkPoints>::New();
        TiXmlDocument doc( m_FileName );
        if(doc.LoadFile())
        {
          TiXmlHandle hDoc(&doc);
          TiXmlElement* pElem;
          TiXmlHandle hRoot(0);

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
          float spacing[3];
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
          for( pElem; pElem; pElem=pElem->NextSiblingElement())
          {
            TiXmlElement* pElem2 = pElem->FirstChildElement();

            vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();

            for( pElem2; pElem2; pElem2=pElem2->NextSiblingElement())
            {
              itk::Point<float> point;
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
          m_OutputCache = OutputType::New(fiberPolyData);
        }
        else
        {
          MITK_INFO << "could not open xml file";
          throw "could not open xml file";
        }
      }
      setlocale(LC_ALL, currLocale.c_str());
      MITK_INFO << "Fiber bundle read";
    }
    catch(...)
    {
      throw;
    }
  }

  void FiberBundleXReader::Update()
  {
    this->GenerateData();
  }

  const char* FiberBundleXReader
    ::GetFileName() const
  {
    return m_FileName.c_str();
  }


  void FiberBundleXReader
    ::SetFileName(const char* aFileName)
  {
    m_FileName = aFileName;
  }


  const char* FiberBundleXReader
    ::GetFilePrefix() const
  {
    return m_FilePrefix.c_str();
  }


  void FiberBundleXReader
    ::SetFilePrefix(const char* aFilePrefix)
  {
    m_FilePrefix = aFilePrefix;
  }


  const char* FiberBundleXReader
    ::GetFilePattern() const
  {
    return m_FilePattern.c_str();
  }


  void FiberBundleXReader
    ::SetFilePattern(const char* aFilePattern)
  {
    m_FilePattern = aFilePattern;
  }


  bool FiberBundleXReader
    ::CanReadFile(const std::string filename, const std::string /*filePrefix*/, const std::string /*filePattern*/)
  {
    // First check the extension
    if(  filename == "" )
    {
      return false;
    }
    std::string ext = itksys::SystemTools::GetFilenameLastExtension(filename);
    ext = itksys::SystemTools::LowerCase(ext);

    if (ext == ".fib")
    {
      return true;
    }

    return false;
  }

} //namespace MITK
