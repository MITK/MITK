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
#include "mitkPointSetSliceMapper2D.h"
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
#include "mitkVtkResliceInterpolationProperty.h"

#include "mitkPicFileIOFactory.h"
#include "mitkPointSetIOFactory.h"
#include "mitkItkImageFileIOFactory.h"
#include "mitkParRecFileIOFactory.h"
#include "mitkSTLFileIOFactory.h"
#include "mitkObjFileIOFactory.h"
#include "mitkVtkSurfaceIOFactory.h"
#include "mitkVtkUnstructuredGridIOFactory.h"
#include "mitkVtkImageIOFactory.h"
#include "mitkVtiFileIOFactory.h"
#include "mitkPicVolumeTimeSeriesIOFactory.h"
#include "mitkStlVolumeTimeSeriesIOFactory.h"
#include "mitkVtkVolumeTimeSeriesIOFactory.h"

#include "mitkImageWriterFactory.h"
#include "mitkPointSetWriterFactory.h"
#include "mitkSurfaceVtkWriterFactory.h"

#include "mitkAddContourTool.h"
#include "mitkSubstractContourTool.h"
#include "mitkRegionGrowingTool.h"
#include "mitkCorrectorTool2D.h"
#include "mitkFillRegionTool.h"
#include "mitkEraseRegionTool.h"


#include "mitkUnstructuredGrid.h"
#include "mitkUnstructuredGridVtkMapper3D.h"

#define CREATE_CPP( TYPE, NAME ) else if ( className == NAME ) {pointer = new TYPE(); pointer->Register();}
#define CREATE_ITK( TYPE, NAME ) else if ( className == NAME ) pointer = TYPE::New();

itk::Object::Pointer mitk::CoreObjectFactory::CreateCoreObject( const std::string& className ) 
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
    CREATE_CPP( Point4dProperty, "Point4dProperty" )
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
    CREATE_CPP( VtkResliceInterpolationProperty, "VtkResliceInterpolationProperty" )
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
    CREATE_ITK( UnstructuredGrid, "UnstructuredGrid" )
    CREATE_ITK( PointSet, "PointSet" )
    CREATE_ITK( SlicedGeometry3D, "SlicedGeometry3D" )
    CREATE_ITK( PlaneGeometry, "PlaneGeometry" )
    CREATE_ITK( PropertyList, "PropertyList" )
    CREATE_ITK( SurfaceMapper2D, "SurfaceMapper2D" )
    CREATE_ITK( SurfaceVtkMapper3D, "SurfaceVtkMapper3D" )
    CREATE_ITK( ImageMapper2D, "ImageMapper2D" )
    CREATE_ITK( VolumeDataVtkMapper3D, "VolumeDataVtkMapper3D" )
    CREATE_ITK( UnstructuredGridVtkMapper3D, "UnstructuredGridVtkMapper3D" )
    CREATE_ITK( LookupTable, "LookupTable" )
    CREATE_ITK( PointSetMapper2D, "PointSetMapper2D" )
    CREATE_ITK( PointSetVtkMapper3D, "PointSetVtkMapper3D" )

  else
    std::cout << "ObjectFactory::CreateObject: unknown class: " << className << std::endl;

  return pointer;
}  

mitk::CoreObjectFactory::Pointer mitk::CoreObjectFactory::GetInstance() {
  static mitk::CoreObjectFactory::Pointer instance;
  if (instance.IsNull()) {
    std::list<itk::LightObject::Pointer> allobjects = itk::ObjectFactoryBase::CreateAllInstance("mitkCoreObjectFactoryBase");
    std::list<itk::LightObject::Pointer>::iterator factoryIt = allobjects.begin();
    while (instance.IsNull() && factoryIt != allobjects.end() ) {
      if (std::string("SBCoreObjectFactory") == (*factoryIt)->GetNameOfClass() ) {
        instance = dynamic_cast<mitk::CoreObjectFactory*>(factoryIt->GetPointer());
      }
      ++factoryIt;
    } 
    factoryIt = allobjects.begin(); 
    while (instance.IsNull() && factoryIt != allobjects.end() ) {
      if ( std::string("QMCoreObjectFactory") == (*factoryIt)->GetNameOfClass() ) {
        instance = dynamic_cast<mitk::CoreObjectFactory*>(factoryIt->GetPointer());
      }
      ++factoryIt;
    } 
    if (instance.IsNull()) {
      instance = mitk::CoreObjectFactory::New();
    }
    std::cout << "CoreObjectFactory: created instance of " << instance->GetNameOfClass() << std::endl;
  } 
  return instance;
}

void mitk::CoreObjectFactory::SetDefaultProperties(mitk::DataTreeNode* /*node*/) 
{
  // don't do anything. Implementation is in subclasses.
}

mitk::CoreObjectFactory::CoreObjectFactory() 
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    itk::ObjectFactoryBase::RegisterFactory( PicFileIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( PointSetIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( ParRecFileIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( STLFileIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( ObjFileIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( VtkSurfaceIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( VtkUnstructuredGridIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( VtkImageIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( VtiFileIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( ItkImageFileIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( PicVolumeTimeSeriesIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( StlVolumeTimeSeriesIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( VtkVolumeTimeSeriesIOFactory::New() );

    mitk::SurfaceVtkWriterFactory::RegisterOneFactory();
    mitk::PointSetWriterFactory::RegisterOneFactory();
    mitk::ImageWriterFactory::RegisterOneFactory();

    itk::ObjectFactoryBase::RegisterFactory( AddContourToolFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( SubstractContourToolFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( RegionGrowingToolFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( CorrectorTool2DFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( FillRegionToolFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( EraseRegionToolFactory::New() );


    alreadyDone = true;
  }
}

mitk::Mapper::Pointer mitk::CoreObjectFactory::CreateMapper(mitk::DataTreeNode* node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper = NULL;
  mitk::BaseData *data = node->GetData();

  if ( id == mitk::BaseRenderer::Standard2D )
  {
    if((dynamic_cast<Image*>(data)!=NULL))
    {
      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(data);
      newMapper = mitk::ImageMapper2D::New();
      newMapper->SetDataTreeNode(node);
    }
    else if((dynamic_cast<Geometry2DData*>(data)!=NULL))
    {
      newMapper = mitk::Geometry2DDataMapper2D::New();
      newMapper->SetDataTreeNode(node);
    }
    else if((dynamic_cast<Surface*>(data)!=NULL))
    {
      newMapper = mitk::SurfaceMapper2D::New();
      // cast because SetDataTreeNode is not virtual
      mitk::SurfaceMapper2D *castedMapper = (mitk::SurfaceMapper2D*)newMapper.GetPointer();
      castedMapper->SetDataTreeNode(node);
    }
    else if((dynamic_cast<Mesh*>(data)!=NULL))
    {
      newMapper = mitk::MeshMapper2D::New();
      newMapper->SetDataTreeNode(node);
    }
    else if((dynamic_cast<PointSet*>(data)!=NULL))
    {
      newMapper = mitk::PointSetMapper2D::New();
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
      newMapper = mitk::PointSetSliceMapper2D::New();
      newMapper->SetDataTreeNode(node);
    }
  }
  else if ( id == mitk::BaseRenderer::Standard3D )
  {
    if((dynamic_cast<Image*>(data) != NULL))
    {
      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(data);
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
    else if((dynamic_cast<Geometry2DData*>(data)!=NULL))
    {
      newMapper = mitk::Geometry2DDataVtkMapper3D::New();
      newMapper->SetDataTreeNode(node);
    }
    else if((dynamic_cast<Surface*>(data)!=NULL))
    {
      newMapper = mitk::SurfaceVtkMapper3D::New();
      newMapper->SetDataTreeNode(node);
    }
    else if((dynamic_cast<Mesh*>(data)!=NULL))
    {
      newMapper = mitk::MeshVtkMapper3D::New();
      newMapper->SetDataTreeNode(node);
    }
    else if((dynamic_cast<PointSet*>(data)!=NULL))
    {
      newMapper = mitk::PointSetVtkMapper3D::New();
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
  return newMapper;
}

#define EXTERNAL_FILE_EXTENSIONS \
    "All known formats(*.dcm *.DCM *.gdcm *.ima *.mps *.pic *.pic.gz *.bmp *.png *.jpg *.tiff *.pvtk *.stl *.vtk *.vtp *.vtu *.obj *.vti *.hdr);;" \
    "DICOM files(*.dcm *.DCM *.gdcm);;" \
    "DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;" \
    "Point sets (*.mps);;" \
    "Sets of 2D slices (*.pic *.pic.gz *.bmp *.png *.dcm *.gdcm *.ima *.tiff);;" \
    "Surface files (*.stl *.vtk *.vtp *.obj)"

#define INTERNAL_FILE_EXTENSIONS \
    "all (*.seq *.mps *.pic *.pic.gz *.seq.gz *.pvtk *.stl *.vtk *.vtp *.vtu *.obj *.vti *.ves " \
         "*.uvg *.dvg *.par *.dcm *.gdcm *.ima *.mhd *.hdr hpsonos.db HPSONOS.DB *.ssm *msm *.bmp *.png *.jpg *.tiff);;" \
    "DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;" \
    "Point sets (*.mps);;" \
    "surface files (*.stl *.vtk *.vtp *.obj);;" \
    "stl files (*.stl);;" \
    "vtk surface files (*.vtk);;" \
    "vtk image files (*.pvtk);;" \
    "vessel files (*.ves *.uvg *.dvg);;" \
    "par/rec files (*.par);;" \
    "DSR files (hpsonos.db HPSONOS.DB);;" \
    "DICOM files (*.dcm *.gdcm *.ima)"

#define SAVE_FILE_EXTENSIONS "all (*.pic *.mhd *.vtk *.vti *.hdr *.png *.tiff *.jpg)"

const char* mitk::CoreObjectFactory::GetFileExtensions() 
{
  return EXTERNAL_FILE_EXTENSIONS;
};

const char* mitk::CoreObjectFactory::GetSaveFileExtensions() {
  return SAVE_FILE_EXTENSIONS; 
};

mitk::CoreObjectFactory::FileWriterList mitk::CoreObjectFactory::GetFileWriters() {
  return m_FileWriters;
}
void mitk::CoreObjectFactory::MapEvent(const mitk::Event*, const int) {

}
