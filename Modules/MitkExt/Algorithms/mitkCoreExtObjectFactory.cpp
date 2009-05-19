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
#include "mitkDataTreeNode.h"

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

#include "mitkVolumeDataVtkMapper3D.h"

#include "mitkAddContourToolFactory.h"
#include "mitkSubtractContourToolFactory.h"
#include "mitkRegionGrowingToolFactory.h"
#include "mitkCorrectorTool2DFactory.h"
#include "mitkFillRegionToolFactory.h"
#include "mitkEraseRegionToolFactory.h"
#include "mitkAutoCropToolFactory.h"
#include "mitkCalculateVolumetryToolFactory.h"
#include "mitkCreateSurfaceToolFactory.h"
#include "mitkBinaryThresholdToolFactory.h"
#include "mitkCalculateGrayValueStatisticsToolFactory.h"
#include "mitkDrawPaintbrushToolFactory.h"
#include "mitkErasePaintbrushToolFactory.h"

mitk::CoreExtObjectFactory::CoreExtObjectFactory(bool registerSelf) 
:CoreObjectFactory()
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    std::cout << "CoreExtObjectFactory c'tor" << std::endl;
    RegisterIOFactories();

    itk::ObjectFactoryBase::RegisterFactory( ParRecFileIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( ObjFileIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( VtkUnstructuredGridIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( StlVolumeTimeSeriesIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( VtkVolumeTimeSeriesIOFactory::New() );

    itk::ObjectFactoryBase::RegisterFactory( AddContourToolFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( SubtractContourToolFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( DrawPaintbrushToolFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( ErasePaintbrushToolFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( RegionGrowingToolFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( CorrectorTool2DFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( FillRegionToolFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( EraseRegionToolFactory::New() );

    itk::ObjectFactoryBase::RegisterFactory( AutoCropToolFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( CalculateVolumetryToolFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( CreateSurfaceToolFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( BinaryThresholdToolFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( CalculateGrayValueStatisticsToolFactory::New() );

    alreadyDone = true;
  }

  if (registerSelf) 
  {
    this->RegisterOverride("mitkCoreObjectFactoryBase",
      "mitkCoreExtObjectFactory",
      "mitk Mapper Creation",
      1,
      itk::CreateObjectFunction<mitk::CoreExtObjectFactory>::New());
  }
}

#define CREATE_CPP( TYPE, NAME ) else if ( className == NAME ) {pointer = new TYPE(); pointer->Register();}
#define CREATE_ITK( TYPE, NAME ) else if ( className == NAME ) pointer = TYPE::New();

itk::Object::Pointer mitk::CoreExtObjectFactory::CreateCoreObject( const std::string& className )
{
  itk::Object::Pointer pointer;

  if ( className == "" )
    return NULL;
  CREATE_ITK( Contour, "Contour" )
  CREATE_ITK( Ellipsoid, "Ellipsoid" )
  CREATE_ITK( Cylinder, "Cylinder" )
  CREATE_ITK( Cuboid, "Cuboid" )
  CREATE_ITK( Cone, "Cone" )
  CREATE_ITK( SeedsImage, "SeedsImage" )
  CREATE_ITK( UnstructuredGrid, "UnstructuredGrid" )
  CREATE_ITK( UnstructuredGridVtkMapper3D, "UnstructuredGridVtkMapper3D" )
  else
    pointer = Superclass::CreateCoreObject( className );

  return pointer;
}


mitk::Mapper::Pointer mitk::CoreExtObjectFactory::CreateMapper(mitk::DataTreeNode* node, MapperSlotId id) 
{
  mitk::Mapper::Pointer newMapper=NULL;
  mitk::BaseData *data = node->GetData();

  if ( id == mitk::BaseRenderer::Standard2D )
  {
    if((dynamic_cast<Mesh*>(data)!=NULL))
    {
      newMapper = mitk::MeshMapper2D::New();
      newMapper->SetDataTreeNode(node);
    }
    else if((dynamic_cast<PointData*>(data)!=NULL))
    {
      newMapper = mitk::PolyDataGLMapper2D::New();
      newMapper->SetDataTreeNode(node);
    }
    else if((dynamic_cast<Contour*>(data)!=NULL))
    {
      newMapper = mitk::ContourMapper2D::New();
      newMapper->SetDataTreeNode(node);
    }
    else if((dynamic_cast<ContourSet*>(data)!=NULL))
    {
      newMapper = mitk::ContourSetMapper2D::New();
      newMapper->SetDataTreeNode(node);
    }
    else if((dynamic_cast<UnstructuredGrid*>(data)!=NULL))
    {
      newMapper = mitk::UnstructuredGridMapper2D::New();
      newMapper->SetDataTreeNode(node);
    }
  }
  else if ( id == mitk::BaseRenderer::Standard3D )
  {
    if((dynamic_cast<Mesh*>(data)!=NULL))
    {
      newMapper = mitk::MeshVtkMapper3D::New();
      newMapper->SetDataTreeNode(node);
    }
    else if((dynamic_cast<PointData*>(data)!=NULL))
    {
      newMapper = mitk::PointDataVtkMapper3D::New();
      newMapper->SetDataTreeNode(node);
    }
    else if((dynamic_cast<Contour*>(data)!=NULL))
    {
      newMapper = mitk::ContourVtkMapper3D::New();
      newMapper->SetDataTreeNode(node);
    }
    else if((dynamic_cast<ContourSet*>(data)!=NULL))
    {
      newMapper = mitk::ContourSetVtkMapper3D::New();
      newMapper->SetDataTreeNode(node);
    }
    else if((dynamic_cast<UnstructuredGrid*>(data)!=NULL))
    {
      newMapper = mitk::UnstructuredGridVtkMapper3D::New();
      newMapper->SetDataTreeNode(node);
    }
  }

  if (newMapper.IsNull()) {
    newMapper = Superclass::CreateMapper(node,id);
  }
  return newMapper;
}

void mitk::CoreExtObjectFactory::SetDefaultProperties(mitk::DataTreeNode* node)
{
  Superclass::SetDefaultProperties(node);
}

const char* mitk::CoreExtObjectFactory::GetFileExtensions() 
{
  return Superclass::GetFileExtensions();
};

const char* mitk::CoreExtObjectFactory::GetSaveFileExtensions() 
{ 
  return Superclass::GetSaveFileExtensions();
};

void mitk::CoreExtObjectFactory::RegisterIOFactories() 
{
}

void RegisterCoreExtObjectFactory() 
{
  bool oneCoreExtObjectFactoryRegistered = false;
  if ( ! oneCoreExtObjectFactoryRegistered ) {
    std::cout << "Registering CoreExtObjectFactory..." << std::endl;
    itk::ObjectFactoryBase::RegisterFactory(mitk::CoreExtObjectFactory::New());
    oneCoreExtObjectFactoryRegistered = true;
  }
}
