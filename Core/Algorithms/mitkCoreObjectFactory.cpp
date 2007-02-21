/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "mitkConfig.h"
#include "mitkCoreObjectFactory.h"

#include "mitkAffineInteractor.h"
#include "mitkColorProperty.h"
#include "mitkCone.h" 
#include "mitkContour.h" 
#include "mitkContourMapper2D.h"
#include "mitkContourSetMapper2D.h"
#include "mitkContourSetVtkMapper3D.h"
#include "mitkContourVtkMapper3D.h"
#include "mitkCuboid.h" 
#include "mitkCylinder.h" 
#include "mitkDataTreeNode.h"
#include "mitkEllipsoid.h" 
#include "mitkEnumerationProperty.h"
#include "mitkGeometry2DData.h"
#include "mitkGeometry2DDataMapper2D.h"
#include "mitkGeometry2DDataVtkMapper3D.h"
#include "mitkGeometry3D.h"
#include "mitkGeometryData.h"
#include "mitkImage.h"
#include "mitkImageMapper2D.h"
#include "mitkLevelWindowProperty.h"
#include "mitkLookupTable.h"
#include "mitkLookupTableProperty.h"
#include "mitkMaterialProperty.h"
#include "mitkMeshMapper2D.h"
#include "mitkMeshVtkMapper3D.h"
#include "mitkPlaneGeometry.h"
#include "mitkPointDataVtkMapper3D.h"
#include "mitkPointSet.h"
#include "mitkPointSetMapper2D.h"
#include "mitkPointSetVtkMapper3D.h"
#include "mitkPolyDataGLMapper2D.h"
#include "mitkProperties.h"
#include "mitkPropertyList.h"
#include "mitkSeedsImage.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkSmartPointerProperty.h"
#include "mitkStringProperty.h"
#include "mitkSurface.h"
#include "mitkSurface.h" 
#include "mitkSurfaceMapper2D.h"
#include "mitkSurfaceVtkMapper3D.h"
#include "mitkTimeSlicedGeometry.h"
#include "mitkTransferFunctionProperty.h"
#include "mitkVolumeDataVtkMapper3D.h"
#include "mitkVtkInterpolationProperty.h"
#include "mitkVtkRepresentationProperty.h"

#define CREATE_CPP( TYPE, NAME ) else if ( className == NAME ) {pointer = new TYPE(); pointer->Register();}
#define CREATE_ITK( TYPE, NAME ) else if ( className == NAME ) pointer = TYPE::New();

itk::Object::Pointer mitk::CoreObjectFactory::CreateSpecialCoreObject( const std::string& className ) 
{  
  itk::Object::Pointer pointer;

  if ( className == "" )
    return NULL;

    CREATE_CPP( BoolProperty, "BoolProperty" )
    CREATE_CPP( IntProperty, "IntProperty" )
    CREATE_CPP( FloatProperty, "FloatProperty" )
    CREATE_CPP( DoubleProperty, "DoubleProperty" )
    CREATE_CPP( Vector3DProperty, "Vector3DProperty" )
    CREATE_CPP( Point3dProperty, "Point3dProperty" )
    CREATE_CPP( Point3iProperty, "Point3iProperty" )
    CREATE_CPP( BoolProperty, "BoolProperty" )
    CREATE_CPP( ColorProperty, "ColorProperty" )
    CREATE_CPP( LevelWindowProperty, "LevelWindowProperty" )
    CREATE_CPP( LookupTableProperty, "LookupTableProperty" )
    CREATE_CPP( MaterialProperty, "MaterialProperty" )
    CREATE_CPP( StringProperty, "StringProperty" )
    CREATE_CPP( SmartPointerProperty, "SmartPointerProperty" )
    CREATE_CPP( TransferFunctionProperty, "TransferFunctionProperty" )
    CREATE_CPP( EnumerationProperty, "EnumerationProperty" )
    CREATE_CPP( VtkInterpolationProperty, "VtkInterpolationProperty" )
    CREATE_CPP( VtkRepresentationProperty, "VtkRepresentationProperty" )
    CREATE_ITK( GeometryData, "GeometryData" )
    CREATE_ITK( Contour, "Contour" )
    CREATE_ITK( Surface, "Surface" )
    CREATE_ITK( Ellipsoid, "Ellipsoid" ) 
    CREATE_ITK( Cylinder, "Cylinder" )
    CREATE_ITK( Cuboid, "Cuboid" )
    CREATE_ITK( Cone, "Cone" )
    CREATE_ITK( Image, "Image" )
    CREATE_ITK( SeedsImage, "SeedsImage" )
    CREATE_ITK( Geometry3D, "Geometry3D" )
    CREATE_ITK( TimeSlicedGeometry, "TimeSlicedGeometry" )
    CREATE_ITK( Surface, "Surface" )
    CREATE_ITK( PointSet, "PointSet" )
    CREATE_ITK( SlicedGeometry3D, "SlicedGeometry3D" )
    CREATE_ITK( PlaneGeometry, "PlaneGeometry" )
    CREATE_ITK( PropertyList, "PropertyList" )
    CREATE_ITK( SurfaceMapper2D, "SurfaceMapper2D" )
    CREATE_ITK( SurfaceVtkMapper3D, "SurfaceVtkMapper3D" )
    CREATE_ITK( ImageMapper2D, "ImageMapper2D" )
    CREATE_ITK( VolumeDataVtkMapper3D, "VolumeDataVtkMapper3D" )
    CREATE_ITK( LookupTable, "LookupTable" )
    CREATE_ITK( PointSetMapper2D, "PointSetMapper2D" )
    CREATE_ITK( PointSetVtkMapper3D, "PointSetVtkMapper3D" )

  else
    std::cout << "ObjectFactory::CreateObject: unknown class: " << className << std::endl;

  return pointer;
}  


void mitk::CoreObjectFactory::RegisterSelfOnce() {
  static bool registeredFactory = false;
  if (! registeredFactory) {
    std::cout << "Registering standard CoreObjectFactory factory ..." << std::endl;
    itk::ObjectFactoryBase::RegisterFactory(mitk::CoreObjectFactory::New());
    registeredFactory = true; 
  }
}

itk::Object::Pointer mitk::CoreObjectFactory::CreateCoreObject( const std::string& className )  {
  RegisterSelfOnce();
  std::list<itk::LightObject::Pointer> allobjects = itk::ObjectFactoryBase::CreateAllInstance("mitkCoreObjectFactoryBase");
  for(std::list<itk::LightObject::Pointer>::iterator i = allobjects.begin();
      i != allobjects.end(); ++i)
  {
    mitk::CoreObjectFactoryBase::Pointer mf = dynamic_cast<mitk::CoreObjectFactoryBase*>(i->GetPointer());
    if(mf)
    {
      itk::Object::Pointer object = mf->CreateSpecialCoreObject(className);
      if(object.IsNotNull()) {
        return object;
      }
    }
    else
    {
      std::cerr << "Error ObjectFactory did not return an CoreObjectFactoryBase "
        << (*i)->GetNameOfClass() 
        << std::endl;
    }
  }
  return NULL;
}

mitk::Mapper::Pointer mitk::CoreObjectFactory::CreateMapper(mitk::DataTreeNode* node, MapperSlotId slotId) {
  RegisterSelfOnce();
  std::list<itk::LightObject::Pointer> allobjects = itk::ObjectFactoryBase::CreateAllInstance("mitkCoreObjectFactoryBase");
  for(std::list<itk::LightObject::Pointer>::iterator i = allobjects.begin();
      i != allobjects.end(); ++i)
  {
    mitk::CoreObjectFactoryBase::Pointer mf = dynamic_cast<mitk::CoreObjectFactoryBase*>(i->GetPointer());
    if(mf)
    {
      mitk::Mapper::Pointer mapper = mf->CreateSpecialMapper(node,slotId);
      if(mapper.IsNotNull()) {
        return mapper;
      }
    }
    else
    {
      std::cerr << "Error ObjectFactory did not return an CoreObjectFactoryBase "
        << (*i)->GetNameOfClass() 
        << std::endl;
    }
  }
  return NULL;
}

mitk::CoreObjectFactory::CoreObjectFactory() {
  this->RegisterOverride("mitkCoreObjectFactoryBase",
      "mitkCoreObjectFactory",
      "mitk Mapper Creation",
      1,
      itk::CreateObjectFunction<mitk::CoreObjectFactory>::New());
};

mitk::Mapper::Pointer mitk::CoreObjectFactory::CreateSpecialMapper(mitk::DataTreeNode* node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper=NULL;

  if(id==mitk::BaseRenderer::Standard2D)
  {
    if((dynamic_cast<Image*>(node->GetData()) != NULL))
    {
      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
      newMapper = mitk::ImageMapper2D::New();
      newMapper->SetDataTreeNode(node);
    }
    else
      if((dynamic_cast<Geometry2DData*>(node->GetData())!=NULL))
      {
        newMapper = mitk::Geometry2DDataMapper2D::New();
        newMapper->SetDataTreeNode(node);
      }
      else
        if((dynamic_cast<Surface*>(node->GetData())!=NULL))
        {
          newMapper = mitk::SurfaceMapper2D::New();
          // cast because SetDataTreeNode is not virtual
          mitk::SurfaceMapper2D *castedMapper = (mitk::SurfaceMapper2D*)newMapper.GetPointer();
          castedMapper->SetDataTreeNode(node);
        }
        else
          if((dynamic_cast<Mesh*>(node->GetData())!=NULL))
          {
            newMapper = mitk::MeshMapper2D::New();
            newMapper->SetDataTreeNode(node);
          }
          else
            if((dynamic_cast<PointSet*>(node->GetData())!=NULL))
            {
              newMapper = mitk::PointSetMapper2D::New();
              newMapper->SetDataTreeNode(node);
            }
    if((dynamic_cast<PointData*>(node->GetData())!=NULL))
    {
      newMapper = mitk::PolyDataGLMapper2D::New();
      newMapper->SetDataTreeNode(node);
    }
    else
      if((dynamic_cast<Contour*>(node->GetData())!=NULL))
      {
        newMapper = mitk::ContourMapper2D::New();
        newMapper->SetDataTreeNode(node);
      }
      else
        if((dynamic_cast<ContourSet*>(node->GetData())!=NULL))
        {
          newMapper = mitk::ContourSetMapper2D::New();
          newMapper->SetDataTreeNode(node);
        }
  }
  else
    if(id==mitk::BaseRenderer::Standard3D)
    {
      if((dynamic_cast<Image*>(node->GetData()) != NULL))
      {
        mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
        if (image->GetPixelType().GetNumberOfComponents() <= 1)
        {
          newMapper = mitk::VolumeDataVtkMapper3D::New();
          newMapper->SetDataTreeNode(node);
        }
        else
        {
          //newMapper = mitk::VectorImageVtkGlyphMapper3D::New();
          //newMapper->SetDataTreeNode(node);
        }
      }
      else
        if((dynamic_cast<Geometry2DData*>(node->GetData())!=NULL))
        {
          newMapper = mitk::Geometry2DDataVtkMapper3D::New();
          newMapper->SetDataTreeNode(node);
        }
        else
          if((dynamic_cast<Surface*>(node->GetData())!=NULL))
          {
            newMapper = mitk::SurfaceVtkMapper3D::New();
            newMapper->SetDataTreeNode(node);
          }
          else
            if((dynamic_cast<Mesh*>(node->GetData())!=NULL))
            {
              newMapper = mitk::MeshVtkMapper3D::New();
              newMapper->SetDataTreeNode(node);
            }
            else
              if((dynamic_cast<PointSet*>(node->GetData())!=NULL))
              {
                newMapper = mitk::PointSetVtkMapper3D::New();
                newMapper->SetDataTreeNode(node);
              }
      if((dynamic_cast<PointData*>(node->GetData())!=NULL))
      {
        newMapper = mitk::PointDataVtkMapper3D::New();
        newMapper->SetDataTreeNode(node);
      }
      else
        if((dynamic_cast<Contour*>(node->GetData())!=NULL))
        {
          newMapper = mitk::ContourVtkMapper3D::New();
          newMapper->SetDataTreeNode(node);
        }
        else
          if((dynamic_cast<ContourSet*>(node->GetData())!=NULL))
          {
            newMapper = mitk::ContourSetVtkMapper3D::New();
            newMapper->SetDataTreeNode(node);
          }
    }
  return newMapper;
}


