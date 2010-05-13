/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 16916 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkCoreExtObjectFactory.h"

#include "mitkProperties.h"
#include "mitkBaseRenderer.h"
#include "mitkDataNode.h"
#include "mitkCoreObjectFactory.h"

#include "mitkParRecFileIOFactory.h"
#include "mitkObjFileIOFactory.h"
#include "mitkVtkUnstructuredGridIOFactory.h"
#include "mitkStlVolumeTimeSeriesIOFactory.h"
#include "mitkVtkVolumeTimeSeriesIOFactory.h"

#include "mitkCone.h"
#include "mitkContour.h"
#include "mitkContourMapper2D.h"
#include "mitkContourSetMapper2D.h"
#include "mitkContourSetVtkMapper3D.h"
#include "mitkContourVtkMapper3D.h"
#include "mitkCuboid.h"
#include "mitkCylinder.h"
#include "mitkEllipsoid.h"
#include "mitkMeshMapper2D.h"
#include "mitkMeshVtkMapper3D.h"
#include "mitkUnstructuredGridMapper2D.h"
#include "mitkPointDataVtkMapper3D.h"
#include "mitkEnhancedPointSetVtkMapper3D.h"
#include "mitkSeedsImage.h"
#include "mitkUnstructuredGrid.h"
#include "mitkUnstructuredGridVtkMapper3D.h"
#include "mitkPolyDataGLMapper2D.h"
#include "mitkGPUVolumeMapper3D.h"

#include "mitkVolumeDataVtkMapper3D.h"

mitk::CoreExtObjectFactory::CoreExtObjectFactory() 
:CoreObjectFactoryBase()
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_INFO << "CoreExtObjectFactory c'tor" << std::endl;
    RegisterIOFactories();

    itk::ObjectFactoryBase::RegisterFactory( ParRecFileIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( ObjFileIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( VtkUnstructuredGridIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( StlVolumeTimeSeriesIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( VtkVolumeTimeSeriesIOFactory::New() );

    alreadyDone = true;
  }
}

mitk::Mapper::Pointer mitk::CoreExtObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId id) 
{
  mitk::Mapper::Pointer newMapper=NULL;
  mitk::BaseData *data = node->GetData();

  if ( id == mitk::BaseRenderer::Standard2D )
  {
    if((dynamic_cast<Mesh*>(data)!=NULL))
    {
      newMapper = mitk::MeshMapper2D::New();
      newMapper->SetDataNode(node);
    }
    else if((dynamic_cast<PointData*>(data)!=NULL))
    {
      newMapper = mitk::PolyDataGLMapper2D::New();
      newMapper->SetDataNode(node);
    }
    else if((dynamic_cast<Contour*>(data)!=NULL))
    {
      newMapper = mitk::ContourMapper2D::New();
      newMapper->SetDataNode(node);
    }
    else if((dynamic_cast<ContourSet*>(data)!=NULL))
    {
      newMapper = mitk::ContourSetMapper2D::New();
      newMapper->SetDataNode(node);
    }
    else if((dynamic_cast<UnstructuredGrid*>(data)!=NULL))
    {
      newMapper = mitk::UnstructuredGridMapper2D::New();
      newMapper->SetDataNode(node);
    }
  }
  else if ( id == mitk::BaseRenderer::Standard3D )
  {
    if((dynamic_cast<Image*>(data) != NULL))
    {
      newMapper = mitk::GPUVolumeMapper3D::New();
      newMapper->SetDataNode(node);
    }
    else if((dynamic_cast<Mesh*>(data)!=NULL))
    {
      newMapper = mitk::MeshVtkMapper3D::New();
      newMapper->SetDataNode(node);
    }
    else if((dynamic_cast<PointData*>(data)!=NULL))
    {
      newMapper = mitk::PointDataVtkMapper3D::New();
      newMapper->SetDataNode(node);
    }
    else if((dynamic_cast<Contour*>(data)!=NULL))
    {
      newMapper = mitk::ContourVtkMapper3D::New();
      newMapper->SetDataNode(node);
    }
    else if((dynamic_cast<ContourSet*>(data)!=NULL))
    {
      newMapper = mitk::ContourSetVtkMapper3D::New();
      newMapper->SetDataNode(node);
    }
    else if((dynamic_cast<UnstructuredGrid*>(data)!=NULL))
    {
      newMapper = mitk::UnstructuredGridVtkMapper3D::New();
      newMapper->SetDataNode(node);
    }
  }
  return newMapper;
}

void mitk::CoreExtObjectFactory::SetDefaultProperties(mitk::DataNode* node)
{
  
  if(node==NULL)
    return;

  mitk::DataNode::Pointer nodePointer = node;

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if(image.IsNotNull() && image->IsInitialized())
  {
    mitk::GPUVolumeMapper3D::SetDefaultProperties(node);
  }

  if (dynamic_cast<mitk::UnstructuredGrid*>(node->GetData()))
  {
    mitk::UnstructuredGridVtkMapper3D::SetDefaultProperties(node);
  }
  
}

const char* mitk::CoreExtObjectFactory::GetFileExtensions() 
{
  return "";
};

const char* mitk::CoreExtObjectFactory::GetSaveFileExtensions() 
{ 
  return "";
};

void mitk::CoreExtObjectFactory::RegisterIOFactories() 
{
}

void RegisterCoreExtObjectFactory() 
{
  static bool oneCoreExtObjectFactoryRegistered = false;
  if ( ! oneCoreExtObjectFactoryRegistered ) 
  {
    MITK_INFO << "Registering CoreExtObjectFactory..." << std::endl;
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(mitk::CoreExtObjectFactory::New());
    oneCoreExtObjectFactoryRegistered = true;
  }
}
