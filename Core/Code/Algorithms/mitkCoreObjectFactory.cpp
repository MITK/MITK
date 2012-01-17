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
#include <mitkImageVtkMapper2D.h>
#include "mitkLevelWindowProperty.h"
#include "mitkLookupTable.h"
#include "mitkLookupTableProperty.h"
#include "mitkPlaneGeometry.h"
#include "mitkPointSet.h"
#include "mitkPointSetGLMapper2D.h"
#include "mitkPointSetVtkMapper3D.h"
#include "mitkPolyDataGLMapper2D.h"
#include "mitkProperties.h"
#include "mitkPropertyList.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkSmartPointerProperty.h"
#include "mitkStringProperty.h"
#include "mitkSurface.h"
#include "mitkSurface.h"
#include "mitkSurfaceGLMapper2D.h"
#include "mitkSurfaceVtkMapper3D.h"
#include "mitkTimeSlicedGeometry.h"
#include "mitkTransferFunctionProperty.h"
#include "mitkVolumeDataVtkMapper3D.h"
#include "mitkVtkInterpolationProperty.h"
#include "mitkVtkRepresentationProperty.h"
#include "mitkVtkResliceInterpolationProperty.h"

//#include "mitkPicFileIOFactory.h"
#include "mitkPointSetIOFactory.h"
#include "mitkItkImageFileIOFactory.h"
#include "mitkSTLFileIOFactory.h"
#include "mitkVtkSurfaceIOFactory.h"
#include "mitkVtkImageIOFactory.h"
#include "mitkVtiFileIOFactory.h"
//#include "mitkPicVolumeTimeSeriesIOFactory.h"

#include "mitkImageWriterFactory.h"
#include "mitkPointSetWriterFactory.h"
#include "mitkSurfaceVtkWriterFactory.h"

mitk::CoreObjectFactory::FileWriterList mitk::CoreObjectFactory::m_FileWriters;

void mitk::CoreObjectFactory::RegisterExtraFactory(CoreObjectFactoryBase* factory) {
  MITK_DEBUG << "CoreObjectFactory: registering extra factory of type " << factory->GetNameOfClass();
  m_ExtraFactories.insert(CoreObjectFactoryBase::Pointer(factory));
}

void mitk::CoreObjectFactory::UnRegisterExtraFactory(CoreObjectFactoryBase *factory)
{
  MITK_DEBUG << "CoreObjectFactory: un-registering extra factory of type " << factory->GetNameOfClass();
  try
  {
    m_ExtraFactories.erase(factory);
  }
  catch( std::exception const& e)
  {
    MITK_ERROR << "Caugt exception while unregistering: " << e.what();
  }
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
    mitk::ImageVtkMapper2D::SetDefaultProperties(node);
    mitk::VolumeDataVtkMapper3D::SetDefaultProperties(node);
  }

  mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>(node->GetData());
  if(surface.IsNotNull())
  {
    mitk::SurfaceGLMapper2D::SetDefaultProperties(node);
    mitk::SurfaceVtkMapper3D::SetDefaultProperties(node);
  }

  mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*>(node->GetData());
  if(pointSet.IsNotNull())
  {
    mitk::PointSetGLMapper2D::SetDefaultProperties(node);
    mitk::PointSetVtkMapper3D::SetDefaultProperties(node);
  }
  for (ExtraFactoriesContainer::iterator it = m_ExtraFactories.begin(); it != m_ExtraFactories.end() ; it++ ) {
    (*it)->SetDefaultProperties(node);
  }
}

mitk::CoreObjectFactory::CoreObjectFactory()
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_DEBUG << "CoreObjectFactory c'tor" << std::endl;

    // FIXME itk::ObjectFactoryBase::RegisterFactory( PicFileIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( PointSetIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( STLFileIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( VtkSurfaceIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( VtkImageIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( VtiFileIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( ItkImageFileIOFactory::New() );
    // FIXME itk::ObjectFactoryBase::RegisterFactory( PicVolumeTimeSeriesIOFactory::New() );

    mitk::SurfaceVtkWriterFactory::RegisterOneFactory();
    mitk::PointSetWriterFactory::RegisterOneFactory();
    mitk::ImageWriterFactory::RegisterOneFactory();

    CreateFileExtensionsMap();

    alreadyDone = true;
  }
}

mitk::Mapper::Pointer mitk::CoreObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper = NULL;
  mitk::Mapper::Pointer tmpMapper = NULL;

  // check whether extra factories provide mapper
  for (ExtraFactoriesContainer::iterator it = m_ExtraFactories.begin(); it != m_ExtraFactories.end() ; it++ ) {
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
        newMapper = mitk::ImageVtkMapper2D::New();
        newMapper->SetDataNode(node);
      }
      else if((dynamic_cast<Geometry2DData*>(data)!=NULL))
      {
        newMapper = mitk::Geometry2DDataMapper2D::New();
        newMapper->SetDataNode(node);
      }
      else if((dynamic_cast<Surface*>(data)!=NULL))
      {
        newMapper = mitk::SurfaceGLMapper2D::New();
        // cast because SetDataNode is not virtual
        mitk::SurfaceGLMapper2D *castedMapper = dynamic_cast<mitk::SurfaceGLMapper2D*>(newMapper.GetPointer());
        castedMapper->SetDataNode(node);
      }
      else if((dynamic_cast<PointSet*>(data)!=NULL))
      {
        newMapper = mitk::PointSetGLMapper2D::New();
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
        newMapper->SetDataNode(node);
      }
    }
  }

  return newMapper;
}

/*
// @deprecated
//
#define EXTERNAL_FILE_EXTENSIONS \
    "All known formats(*.dcm *.DCM *.dc3 *.DC3 *.gdcm *.ima *.mhd *.mps *.nii *.pic *.pic.gz *.bmp *.png *.jpg *.tiff *.pvtk *.stl *.vtk *.vtp *.vtu *.obj *.vti *.hdr *.nrrd *.nhdr );;" \
    "DICOM files(*.dcm *.DCM *.dc3 *.DC3 *.gdcm);;" \
    "DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;" \
    "NRRD Vector Images (*.nrrd *.nhdr);;" \
    "Point sets (*.mps);;" \
    "Sets of 2D slices (*.pic *.pic.gz *.bmp *.png *.dcm *.gdcm *.ima *.tiff);;" \
    "Surface files (*.stl *.vtk *.vtp *.obj);;" \
    "NIfTI format (*.nii)"

#define SAVE_FILE_EXTENSIONS "all (*.pic *.mhd *.vtk *.vti *.hdr *.png *.tiff *.jpg *.hdr *.bmp *.dcm *.gipl *.nii *.nrrd *.nhdr *.spr *.lsm *.dwi *.hdwi *.qbi *.hqbi)"
*/

/**
 * @brief This method gets the supported (open) file extensions as string. This string is can then used by the QT QFileDialog widget.
 * @return The c-string that contains the file extensions
 *
 */
const char* mitk::CoreObjectFactory::GetFileExtensions()
{
  MultimapType aMap;
  for (ExtraFactoriesContainer::iterator it = m_ExtraFactories.begin(); it != m_ExtraFactories.end() ; it++ )
  {
    aMap = (*it)->GetFileExtensionsMap();
    this->MergeFileExtensions(m_FileExtensionsMap, aMap);
  }
  this->CreateFileExtensions(m_FileExtensionsMap, m_FileExtensions);
  return m_FileExtensions.c_str();
}
  
/**
 * @brief Merge the input map into the fileExtensionsMap. Duplicate entries are removed
 * @param fileExtensionsMap the existing map, it contains value pairs like ("*.dcm", "DICOM files"),("*.dc3", "DICOM files").
 *                          This map is extented/merged with the values from the input map.
 * @param inputMap the input map, it contains value pairs like ("*.dcm", "DICOM files"),("*.dc3", "DICOM files") returned by
 *                 the extra factories.
 *
 */
void mitk::CoreObjectFactory::MergeFileExtensions(MultimapType& fileExtensionsMap, MultimapType inputMap)
{
  bool duplicateFound = false;
  std::pair<MultimapType::iterator, MultimapType::iterator> pairOfIter; 
  for (MultimapType::iterator it = inputMap.begin(); it != inputMap.end(); ++it)
  {
    duplicateFound = false;
    pairOfIter = fileExtensionsMap.equal_range((*it).first);
    for (MultimapType::iterator it2 = pairOfIter.first; it2 != pairOfIter.second; ++it2)
    {
      //cout << "  [" << (*it).first << ", " << (*it).second << "]" << endl;
      std::string aString = (*it2).second;
      if (aString.compare((*it).second) == 0)
      {
        //cout << "  DUP!! [" << (*it).first << ", " << (*it).second << "]" << endl;
        duplicateFound = true;
        break;
      }
    }
    if (!duplicateFound)
    {
      fileExtensionsMap.insert(std::pair<std::string, std::string>((*it).first, (*it).second));
    }
  }
}

/**
 * @brief get the defined (open) file extension map
 * @return the defined (open) file extension map
 */
mitk::CoreObjectFactoryBase::MultimapType mitk::CoreObjectFactory::GetFileExtensionsMap()
{
  return m_FileExtensionsMap;
}

/**
 * @brief initialize the file extension entries for open and save
 */
void mitk::CoreObjectFactory::CreateFileExtensionsMap()
{
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.dcm", "DICOM files"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.DCM", "DICOM files"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.dc3", "DICOM files"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.DC3", "DICOM files"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.gdcm", "DICOM files"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.seq", "DKFZ Pic"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.pic", "DKFZ Pic"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.pic.gz", "DKFZ Pic"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.mhd", "MetaImage"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.seq.gz", "DKFZ Pic"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.nrrd", "Nearly Raw Raster Data"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.nhdr", "NRRD with detached header"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.mps", "Point sets"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.pic", "Sets of 2D slices"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.pic.gz", "Sets of 2D slices"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.bmp", "Sets of 2D slices"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.png", "Sets of 2D slices"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.jpg", "Sets of 2D slices"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.jpeg", "Sets of 2D slices"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.dcm", "Sets of 2D slices"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.gdcm", "Sets of 2D slices"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.ima", "Sets of 2D slices"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.tiff", "Sets of 2D slices"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.tif", "Sets of 2D slices"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.stl", "Surface files"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.vtk", "Surface files"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.vtp", "Surface files"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.obj", "Surface files"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.nii", "NIfTI format"));

  //m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.pic", "DKFZ Pic"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.mhd", "MetaImage"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.vtk", "Surface Files"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.vti", "VTK Image Data Files"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.hdr", "Image Header Files"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.png", "Sets of 2D slices"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.tiff", "Sets of 2D slices"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.tif", "Sets of 2D slices"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.jpg", "Sets of 2D slices"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.jpeg", "Sets of 2D slices"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.bmp", "Sets of 2D slices"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.dcm", "Sets of 2D slices"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.gipl", "UMDS GIPL Format Files"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.nii", "NIfTI format"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.nrrd", "Nearly Raw Raster Data"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.nhdr", "NRRD with detached header"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.lsm", "Microscope Images"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.dwi", "Diffusion Weighted Images"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.hdwi", "Diffusion Weighted Images"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.qbi", "Q-Ball Images"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.hqbi", "Q-Ball Images"));

}

/**
 * @brief This method gets the supported (save) file extensions as string. This string is can then used by the QT QFileDialog widget.
 * @return The c-string that contains the (save) file extensions
 *
 */
const char* mitk::CoreObjectFactory::GetSaveFileExtensions() {
  MultimapType aMap;
  for (ExtraFactoriesContainer::iterator it = m_ExtraFactories.begin(); it != m_ExtraFactories.end() ; it++ )
  {
    aMap = (*it)->GetSaveFileExtensionsMap();
    this->MergeFileExtensions(m_SaveFileExtensionsMap, aMap);

  }
  this->CreateFileExtensions(m_SaveFileExtensionsMap, m_SaveFileExtensions);
  return m_SaveFileExtensions.c_str();
};

/**
 * @brief get the defined (save) file extension map
 * @return the defined (save) file extension map
 */
mitk::CoreObjectFactoryBase::MultimapType mitk::CoreObjectFactory::GetSaveFileExtensionsMap()
{
  return m_SaveFileExtensionsMap;
}

mitk::CoreObjectFactory::FileWriterList mitk::CoreObjectFactory::GetFileWriters() {
  FileWriterList allWriters = m_FileWriters;
  for (ExtraFactoriesContainer::iterator it = m_ExtraFactories.begin(); it != m_ExtraFactories.end() ; it++ ) {
    FileWriterList list2 = (*it)->GetFileWriters();
    allWriters.merge(list2);
  }
  return allWriters;
}
void mitk::CoreObjectFactory::MapEvent(const mitk::Event*, const int) {

}

