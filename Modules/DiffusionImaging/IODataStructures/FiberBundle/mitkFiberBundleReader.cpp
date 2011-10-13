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

#include "mitkFiberBundleReader.h"
#include <itkMetaDataObject.h>
#include <vtkMatrix4x4.h>
#include <tinyxml.h>
#include <itkAffineGeometryFrame.h>
#include <mitkSlicedGeometry3D.h>
#include <mitkPlaneGeometry.h>
#include <mitkGeometry2D.h>
#include <vtkPolyData.h>
#include <vtkDataReader.h>
#include <vtkPolyDataReader.h>
#include <vtkCellArray.h>
#include <mitkSurface.h>

const char* mitk::FiberBundleReader::XML_GEOMETRY = "geometry";

const char* mitk::FiberBundleReader::XML_MATRIX_XX = "xx";

const char* mitk::FiberBundleReader::XML_MATRIX_XY = "xy";

const char* mitk::FiberBundleReader::XML_MATRIX_XZ = "xz";

const char* mitk::FiberBundleReader::XML_MATRIX_YX = "yx";

const char* mitk::FiberBundleReader::XML_MATRIX_YY = "yy";

const char* mitk::FiberBundleReader::XML_MATRIX_YZ = "yz";

const char* mitk::FiberBundleReader::XML_MATRIX_ZX = "zx";

const char* mitk::FiberBundleReader::XML_MATRIX_ZY = "zy";

const char* mitk::FiberBundleReader::XML_MATRIX_ZZ = "zz";

const char* mitk::FiberBundleReader::XML_ORIGIN_X = "origin_x";

const char* mitk::FiberBundleReader::XML_ORIGIN_Y = "origin_y";

const char* mitk::FiberBundleReader::XML_ORIGIN_Z = "origin_z";

const char* mitk::FiberBundleReader::XML_SPACING_X = "spacing_x";

const char* mitk::FiberBundleReader::XML_SPACING_Y = "spacing_y";

const char* mitk::FiberBundleReader::XML_SPACING_Z = "spacing_z";

const char* mitk::FiberBundleReader::XML_SIZE_X = "size_x";

const char* mitk::FiberBundleReader::XML_SIZE_Y = "size_y";

const char* mitk::FiberBundleReader::XML_SIZE_Z = "size_z";

const char* mitk::FiberBundleReader::XML_FIBER_BUNDLE = "fiber_bundle";

const char* mitk::FiberBundleReader::XML_FIBER = "fiber";

const char* mitk::FiberBundleReader::XML_PARTICLE = "particle";

const char* mitk::FiberBundleReader::XML_ID = "id";

const char* mitk::FiberBundleReader::XML_POS_X = "pos_x";

const char* mitk::FiberBundleReader::XML_POS_Y = "pos_y";

const char* mitk::FiberBundleReader::XML_POS_Z = "pos_z";

const char* mitk::FiberBundleReader::XML_NUM_FIBERS = "num_fibers";

const char* mitk::FiberBundleReader::XML_NUM_PARTICLES = "num_particles";

const char* mitk::FiberBundleReader::XML_FIBER_BUNDLE_FILE = "fiber_bundle_file" ;

const char* mitk::FiberBundleReader::XML_FILE_VERSION = "file_version" ;

const char* mitk::FiberBundleReader::VERSION_STRING = "0.1" ;

namespace mitk
{

  void FiberBundleReader
    ::GenerateData()
  {
    MITK_INFO << "Reading fiber bundle";
    if ( ( ! m_OutputCache ) )
    {
      Superclass::SetNumberOfRequiredOutputs(0);
      this->GenerateOutputInformation();
    }

    if (!m_OutputCache)
    {
      itkWarningMacro("Tree cache is empty!");
    }


    Superclass::SetNumberOfRequiredOutputs(1);
    Superclass::SetNthOutput(0, m_OutputCache.GetPointer());
  }

  void FiberBundleReader::GenerateOutputInformation()
  {
    m_OutputCache = OutputType::New();

    std::string ext = itksys::SystemTools::GetFilenameLastExtension(m_FileName);
    ext = itksys::SystemTools::LowerCase(ext);

    const std::string& locale = "C";
    const std::string& currLocale = setlocale( LC_ALL, NULL );

    if ( locale.compare(currLocale)!=0 )
    {
      try
      {
        setlocale(LC_ALL, locale.c_str());
      }
      catch(...)
      {
        MITK_INFO << "Could not set locale " << locale;
      }
    }

    if ( m_FileName == "")
    {

    }
    else if (ext == ".fib")
    {
      try
      {
        TiXmlDocument doc( m_FileName );
        doc.LoadFile();

        TiXmlHandle hDoc(&doc);
        TiXmlElement* pElem;
        TiXmlHandle hRoot(0);

        pElem = hDoc.FirstChildElement().Element();

        // save this for later
        hRoot = TiXmlHandle(pElem);

        pElem = hRoot.FirstChildElement(FiberBundleReader::XML_GEOMETRY).Element();

        // read geometry
        mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();

        // read origin
        mitk::Point3D origin;
        double temp = 0;
        pElem->Attribute(FiberBundleReader::XML_ORIGIN_X, &temp);
        origin[0] = temp;
        pElem->Attribute(FiberBundleReader::XML_ORIGIN_Y, &temp);
        origin[1] = temp;
        pElem->Attribute(FiberBundleReader::XML_ORIGIN_Z, &temp);
        origin[2] = temp;
        geometry->SetOrigin(origin);

        // read spacing
        float spacing[3];
        pElem->Attribute(FiberBundleReader::XML_SPACING_X, &temp);
        spacing[0] = temp;
        pElem->Attribute(FiberBundleReader::XML_SPACING_Y, &temp);
        spacing[1] = temp;
        pElem->Attribute(FiberBundleReader::XML_SPACING_Z, &temp);
        spacing[2] = temp;
        geometry->SetSpacing(spacing);

        // read transform
        vtkMatrix4x4* m = vtkMatrix4x4::New();
        pElem->Attribute(FiberBundleReader::XML_MATRIX_XX, &temp);
        m->SetElement(0,0,temp);
        pElem->Attribute(FiberBundleReader::XML_MATRIX_XY, &temp);
        m->SetElement(1,0,temp);
        pElem->Attribute(FiberBundleReader::XML_MATRIX_XZ, &temp);
        m->SetElement(2,0,temp);
        pElem->Attribute(FiberBundleReader::XML_MATRIX_YX, &temp);
        m->SetElement(0,1,temp);
        pElem->Attribute(FiberBundleReader::XML_MATRIX_YY, &temp);
        m->SetElement(1,1,temp);
        pElem->Attribute(FiberBundleReader::XML_MATRIX_YZ, &temp);
        m->SetElement(2,1,temp);
        pElem->Attribute(FiberBundleReader::XML_MATRIX_ZX, &temp);
        m->SetElement(0,2,temp);
        pElem->Attribute(FiberBundleReader::XML_MATRIX_ZY, &temp);
        m->SetElement(1,2,temp);
        pElem->Attribute(FiberBundleReader::XML_MATRIX_ZZ, &temp);
        m->SetElement(2,2,temp);

        m->SetElement(0,3,origin[0]);
        m->SetElement(1,3,origin[1]);
        m->SetElement(2,3,origin[2]);
        m->SetElement(3,3,1);
        geometry->SetIndexToWorldTransformByVtkMatrix(m);

        // read bounds
        float bounds[] = {0, 0, 0, 0, 0, 0};
        pElem->Attribute(FiberBundleReader::XML_SIZE_X, &temp);
        bounds[1] = temp;
        pElem->Attribute(FiberBundleReader::XML_SIZE_Y, &temp);
        bounds[3] = temp;
        pElem->Attribute(FiberBundleReader::XML_SIZE_Z, &temp);
        bounds[5] = temp;
        geometry->SetFloatBounds(bounds);

        // read bounds
        float bounds2[] = {0, 0, 0};
        bounds2[0] = bounds[1];
        bounds2[1] = bounds[3];
        bounds2[2] = bounds[5];
        m_OutputCache->SetBounds(bounds2);

        geometry->SetImageGeometry(true);
        m_OutputCache->SetGeometry(geometry);

        // generate tract container
        ContainerType::Pointer tractContainer = ContainerType::New();

        int fiberID = 0;
        pElem = hRoot.FirstChildElement(FiberBundleReader::XML_FIBER_BUNDLE).FirstChild().Element();
        for( pElem; pElem; pElem=pElem->NextSiblingElement())
        {
          TiXmlElement* pElem2 = pElem->FirstChildElement();
          ContainerTractType::Pointer tract = ContainerTractType::New();
          for( pElem2; pElem2; pElem2=pElem2->NextSiblingElement())
          {
            ContainerPointType point;
            pElem2->Attribute(FiberBundleReader::XML_POS_X, &temp);
            point[0] = temp;
            pElem2->Attribute(FiberBundleReader::XML_POS_Y, &temp);
            point[1] = temp;
            pElem2->Attribute(FiberBundleReader::XML_POS_Z, &temp);
            point[2] = temp;

            tract->InsertElement(tract->Size(), point);

          }
          pElem->Attribute(FiberBundleReader::XML_ID, &fiberID);
          tractContainer->CreateIndex(fiberID);
          tractContainer->SetElement(fiberID, tract);

        }
        m_OutputCache->addTractContainer(tractContainer);
        m_OutputCache->initFiberGroup();

        MITK_INFO << "Fiber bundle read";
      }
      catch(...)
      {
        MITK_INFO << "Could not read file ";
      }
    }
    else if (ext == ".vfib")
    {
      // generate tract container
      ContainerType::Pointer tractContainer = ContainerType::New();
      mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();

      ///We create a Generic Reader to test de .vtk/
      vtkDataReader *chooser=vtkDataReader::New();
      chooser->SetFileName(m_FileName.c_str() );
      if( chooser->IsFilePolyData())
      {
        vtkPolyDataReader *reader = vtkPolyDataReader::New();
        reader->SetFileName( m_FileName.c_str() );
        reader->Update();

        if ( reader->GetOutput() != NULL )
        {
          vtkPolyData* output = reader->GetOutput();
          output->ComputeBounds();
          double bounds[3];
          output->GetBounds(bounds);
          double center[3];
          output->GetCenter(center);
          Point3D origin;
          origin.SetElement(0, -center[0]);
          origin.SetElement(1, -center[1]);
          origin.SetElement(2, -center[2]);

          mitk::Surface::Pointer surf = mitk::Surface::New();
          surf->SetVtkPolyData(output);
          mitk::Geometry3D* geom  = surf->GetGeometry();
          //geom->SetOrigin(origin);
          geom->SetImageGeometry(true);
          m_OutputCache->SetBounds(bounds);
          m_OutputCache->SetGeometry(geom);
          vtkCellArray* cells = output->GetLines();

          cells->InitTraversal();

          for (int i=0; i<output->GetNumberOfCells(); i++)
          {
            ContainerTractType::Pointer tract = ContainerTractType::New();
            vtkCell* cell = output->GetCell(i);
            int p = cell->GetNumberOfPoints();
            vtkPoints* points = cell->GetPoints();
            for (int j=0; j<p; j++)
            {
              double p[3];
              points->GetPoint(j, p);
              ContainerPointType point;
              point[0] = p[0];
              point[1] = p[1];
              point[2] = p[2];
              tract->InsertElement(tract->Size(), point);
            }
            tractContainer->InsertElement(i, tract);
          }
        }
        reader->Delete();
      }
      chooser->Delete();

      m_OutputCache->addTractContainer(tractContainer);
      m_OutputCache->initFiberGroup();
      MITK_INFO << "Fiber bundle read";
    }

    try
    {
      setlocale(LC_ALL, currLocale.c_str());
    }
    catch(...)
    {
      MITK_INFO << "Could not reset locale " << currLocale;
    }
  }

  void FiberBundleReader::Update()
  {
    this->GenerateData();
  }

  const char* FiberBundleReader
    ::GetFileName() const
  {
    return m_FileName.c_str();
  }


  void FiberBundleReader
    ::SetFileName(const char* aFileName)
  {
    m_FileName = aFileName;
  }


  const char* FiberBundleReader
    ::GetFilePrefix() const
  {
    return m_FilePrefix.c_str();
  }


  void FiberBundleReader
    ::SetFilePrefix(const char* aFilePrefix)
  {
    m_FilePrefix = aFilePrefix;
  }


  const char* FiberBundleReader
    ::GetFilePattern() const
  {
    return m_FilePattern.c_str();
  }


  void FiberBundleReader
    ::SetFilePattern(const char* aFilePattern)
  {
    m_FilePattern = aFilePattern;
  }


  bool FiberBundleReader
    ::CanReadFile(const std::string filename, const std::string /*filePrefix*/, const std::string /*filePattern*/)
  {
    // First check the extension
    if(  filename == "" )
    {
      return false;
    }
    std::string ext = itksys::SystemTools::GetFilenameLastExtension(filename);
    ext = itksys::SystemTools::LowerCase(ext);

    if (ext == ".fib" || ext == ".vfib")
    {
      return true;
    }

    return false;
  }

} //namespace MITK
