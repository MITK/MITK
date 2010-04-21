/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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
#include "mitkDataNode.h"
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
//#include "mitkMaterialProperty.h"
#include "mitkPlaneGeometry.h"
#include "mitkPointSet.h"
#include "mitkPointSetMapper2D.h"
#include "mitkPointSetVtkMapper3D.h"
#include "mitkPolyDataGLMapper2D.h"
#include "mitkProperties.h"
#include "mitkPropertyList.h"
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
#include "mitkSTLFileIOFactory.h"
#include "mitkVtkSurfaceIOFactory.h"
#include "mitkVtkImageIOFactory.h"
#include "mitkVtiFileIOFactory.h"
#include "mitkPicVolumeTimeSeriesIOFactory.h"

#include "mitkImageWriterFactory.h"
#include "mitkPointSetWriterFactory.h"
#include "mitkSurfaceVtkWriterFactory.h"

mitk::CoreObjectFactory::FileWriterList mitk::CoreObjectFactory::m_FileWriters;

void mitk::CoreObjectFactory::RegisterExtraFactory(CoreObjectFactoryBase* factory) {
  MITK_INFO << "CoreObjectFactory: registering extra factory of type " << factory->GetNameOfClass();
  m_ExtraFactories.push_back(CoreObjectFactoryBase::Pointer(factory));  
}

mitk::CoreObjectFactory::Pointer mitk::CoreObjectFactory::GetInstance() {
  static mitk::CoreObjectFactory::Pointer instance;
  if (instance.IsNull()) 
  {
     instance = mitk::CoreObjectFactory::New();
  }
  return instance;
}

#include <mitkDataNodeFactory.h>

void mitk::CoreObjectFactory::SetDefaultProperties(mitk::DataNode* node)
{
  if(node==NULL)
    return;

  mitk::DataNode::Pointer nodePointer = node;

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if(image.IsNotNull() && image->IsInitialized())
  {
    mitk::ImageMapper2D::SetDefaultProperties(node);
    mitk::VolumeDataVtkMapper3D::SetDefaultProperties(node);
  }

  mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>(node->GetData());
  if(surface.IsNotNull())
  {
    mitk::SurfaceMapper2D::SetDefaultProperties(node);
    mitk::SurfaceVtkMapper3D::SetDefaultProperties(node);
  }

  mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*>(node->GetData());
  if(pointSet.IsNotNull())
  {
    mitk::PointSetMapper2D::SetDefaultProperties(node);
    mitk::PointSetVtkMapper3D::SetDefaultProperties(node);
  }
  for (ExtraFactoriesList::iterator it = m_ExtraFactories.begin(); it != m_ExtraFactories.end() ; it++ ) {
    (*it)->SetDefaultProperties(node);
  }
}

mitk::CoreObjectFactory::CoreObjectFactory()
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_INFO << "CoreObjectFactory c'tor" << std::endl;

    itk::ObjectFactoryBase::RegisterFactory( PicFileIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( PointSetIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( STLFileIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( VtkSurfaceIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( VtkImageIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( VtiFileIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( ItkImageFileIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( PicVolumeTimeSeriesIOFactory::New() );

    mitk::SurfaceVtkWriterFactory::RegisterOneFactory();
    mitk::PointSetWriterFactory::RegisterOneFactory();
    mitk::ImageWriterFactory::RegisterOneFactory();

    alreadyDone = true;
  }
}

mitk::Mapper::Pointer mitk::CoreObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper = NULL;
  mitk::Mapper::Pointer tmpMapper = NULL;

  // check whether extra factories provide mapper
  for (ExtraFactoriesList::iterator it = m_ExtraFactories.begin(); it != m_ExtraFactories.end() ; it++ ) {
    tmpMapper = (*it)->CreateMapper(node,id);
    if(tmpMapper.IsNotNull())
      newMapper = tmpMapper;
  }

  if (newMapper.IsNull()) 
  {

    mitk::BaseData *data = node->GetData();

    if ( id == mitk::BaseRenderer::Standard2D )
    {
      if((dynamic_cast<Image*>(data)!=NULL))
      {
        mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(data);
        newMapper = mitk::ImageMapper2D::New();
        newMapper->SetDataNode(node);
      }
      else if((dynamic_cast<Geometry2DData*>(data)!=NULL))
      {
        newMapper = mitk::Geometry2DDataMapper2D::New();
        newMapper->SetDataNode(node);
      }
      else if((dynamic_cast<Surface*>(data)!=NULL))
      {
        newMapper = mitk::SurfaceMapper2D::New();
        // cast because SetDataNode is not virtual
        mitk::SurfaceMapper2D *castedMapper = dynamic_cast<mitk::SurfaceMapper2D*>(newMapper.GetPointer());
        castedMapper->SetDataNode(node);
      }
      else if((dynamic_cast<PointSet*>(data)!=NULL))
      {
        newMapper = mitk::PointSetMapper2D::New();
        newMapper->SetDataNode(node);
      }
    }
    else if ( id == mitk::BaseRenderer::Standard3D )
    {
      if((dynamic_cast<Image*>(data) != NULL))
      {
        newMapper = mitk::VolumeDataVtkMapper3D::New();
        newMapper->SetDataNode(node);
      }
      else if((dynamic_cast<Geometry2DData*>(data)!=NULL))
      {
        newMapper = mitk::Geometry2DDataVtkMapper3D::New();
        newMapper->SetDataNode(node);
      }
      else if((dynamic_cast<Surface*>(data)!=NULL))
      {
        newMapper = mitk::SurfaceVtkMapper3D::New();
        newMapper->SetDataNode(node);
      }
      else if((dynamic_cast<PointSet*>(data)!=NULL))
      {
        newMapper = mitk::PointSetVtkMapper3D::New();
        //newMapper = mitk::EnhancedPointSetVtkMapper3D::New(); // <-- use this if you want to try the new work in progres point set mapper
        newMapper->SetDataNode(node);
      }
    }
  }

  return newMapper;
}

#define EXTERNAL_FILE_EXTENSIONS \
    "All known formats(*.dcm *.DCM *.dc3 *.DC3 *.gdcm *.ima *.mhd *.mps *.nii *.pic *.pic.gz *.bmp *.png *.jpg *.tiff *.pvtk *.stl *.vtk *.vtp *.vtu *.obj *.vti *.hdr *.nrrd *.nhdr );;" \
    "DICOM files(*.dcm *.DCM *.dc3 *.DC3 *.gdcm);;" \
    "DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;" \
    "NRRD Vector Images (*.nrrd *.nhdr);;" \
    "Point sets (*.mps);;" \
    "Sets of 2D slices (*.pic *.pic.gz *.bmp *.png *.dcm *.gdcm *.ima *.tiff);;" \
    "Surface files (*.stl *.vtk *.vtp *.obj);;" \
    "NIfTI format (*.nii)"

#define INTERNAL_FILE_EXTENSIONS \
    "all (*.seq *.mps *.nii *.pic *.pic.gz *.seq.gz *.pvtk *.stl *.vtk *.vtp *.vtu *.obj *.vti *.ves *.nrrd *.nhdr " \
         "*.uvg *.dvg *.par *.dcm *.dc3 *.gdcm *.ima *.mhd *.hdr hpsonos.db HPSONOS.DB *.ssm *msm *.bmp *.png *.jpg *.tiff);;" \
    "DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;" \
    "NRRD Vector Images (*.nrrd *.nhdr);;" \
    "Point sets (*.mps);;" \
    "surface files (*.stl *.vtk *.vtp *.obj);;" \
    "stl files (*.stl);;" \
    "vtk surface files (*.vtk);;" \
    "vtk image files (*.pvtk);;" \
    "vessel files (*.ves *.uvg *.dvg);;" \
    "par/rec files (*.par);;" \
    "DSR files (hpsonos.db HPSONOS.DB);;" \
    "DICOM files (*.dcm *.gdcm *.dc3 *.ima);;" \
    "NIfTI format (*.nii)"

#define SAVE_FILE_EXTENSIONS "all (*.pic *.mhd *.vtk *.vti *.hdr *.png *.tiff *.jpg *.hdr *.bmp *.dcm *.gipl *.nii *.nrrd *.nhdr *.spr *.lsm *.dwi *.hdwi *.qbi *.hqbi)"

const char* mitk::CoreObjectFactory::GetFileExtensions()
{
  m_FileExtensions = EXTERNAL_FILE_EXTENSIONS;

  for (ExtraFactoriesList::iterator it = m_ExtraFactories.begin(); it != m_ExtraFactories.end() ; it++ ) {
    m_FileExtensions.append(";;").append((*it)->GetFileExtensions());
  }

  return m_FileExtensions.c_str();
};

const char* mitk::CoreObjectFactory::GetSaveFileExtensions() {
  m_SaveFileExtensions = SAVE_FILE_EXTENSIONS;

  for (ExtraFactoriesList::iterator it = m_ExtraFactories.begin(); it != m_ExtraFactories.end() ; it++ ) {
    m_SaveFileExtensions.append(";;").append((*it)->GetFileExtensions());
  }
  return m_SaveFileExtensions.c_str();

};

mitk::CoreObjectFactory::FileWriterList mitk::CoreObjectFactory::GetFileWriters() {
  FileWriterList allWriters = m_FileWriters;
  for (ExtraFactoriesList::iterator it = m_ExtraFactories.begin(); it != m_ExtraFactories.end() ; it++ ) {
    FileWriterList list2 = (*it)->GetFileWriters();
    allWriters.merge(list2);
  }
  return allWriters;
}
void mitk::CoreObjectFactory::MapEvent(const mitk::Event*, const int) {

}

