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

#include "mitkConfig.h"
#include "mitkCoreObjectFactory.h"

#include "mitkAffineInteractor.h"
#include "mitkColorProperty.h"
#include "mitkDataNode.h"
#include "mitkEnumerationProperty.h"
#include "mitkPlaneGeometryData.h"
#include "mitkPlaneGeometryDataMapper2D.h"
#include "mitkPlaneGeometryDataVtkMapper3D.h"
#include "mitkGeometry3D.h"
#include "mitkGeometryData.h"
#include "mitkImage.h"
#include <mitkImageVtkMapper2D.h>
#include "mitkLevelWindowProperty.h"
#include "mitkLookupTable.h"
#include "mitkLookupTableProperty.h"
#include "mitkPlaneGeometry.h"
#include "mitkPointSet.h"
#include "mitkPointSetVtkMapper2D.h"
#include "mitkPointSetVtkMapper3D.h"
#include "mitkProperties.h"
#include "mitkPropertyList.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkSmartPointerProperty.h"
#include "mitkStringProperty.h"
#include "mitkSurface.h"
#include "mitkSurface.h"
#include "mitkSurfaceGLMapper2D.h"
#include "mitkSurfaceVtkMapper3D.h"
#include "mitkTimeGeometry.h"
#include "mitkTransferFunctionProperty.h"
#include "mitkVolumeDataVtkMapper3D.h"
#include "mitkVtkInterpolationProperty.h"
#include "mitkVtkRepresentationProperty.h"
#include "mitkVtkResliceInterpolationProperty.h"

// Legacy Support:
#include <mitkCoreServices.h>
#include <Internal/mitkLegacyFileReaderService.h>
#include <Internal/mitkLegacyFileWriterService.h>


void mitk::CoreObjectFactory::RegisterExtraFactory(CoreObjectFactoryBase* factory) {
  MITK_DEBUG << "CoreObjectFactory: registering extra factory of type " << factory->GetNameOfClass();
  m_ExtraFactories.insert(CoreObjectFactoryBase::Pointer(factory));
  // Register Legacy Reader and Writer
  this->RegisterLegacyReaders(factory);
  this->RegisterLegacyWriters(factory);
}

void mitk::CoreObjectFactory::UnRegisterExtraFactory(CoreObjectFactoryBase *factory)
{
  MITK_DEBUG << "CoreObjectFactory: un-registering extra factory of type " << factory->GetNameOfClass();
  this->UnRegisterLegacyWriters(factory);
  this->UnRegisterLegacyReaders(factory);
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

mitk::CoreObjectFactory::~CoreObjectFactory()
{
  for (std::map<mitk::CoreObjectFactoryBase*, std::list< mitk::LegacyFileReaderService* > >::iterator iter = m_LegacyReaders.begin();
       iter != m_LegacyReaders.end(); ++iter)
  {
    for (std::list<mitk::LegacyFileReaderService*>::iterator readerIter = iter->second.begin(),
      readerIterEnd = iter->second.end(); readerIter != readerIterEnd; ++readerIter)
    {
      delete *readerIter;
    }
  }

  for (std::map<mitk::CoreObjectFactoryBase*, std::list< mitk::LegacyFileWriterService* > >::iterator iter = m_LegacyWriters.begin();
       iter != m_LegacyWriters.end(); ++iter)
  {
    for (std::list<mitk::LegacyFileWriterService*>::iterator writerIter = iter->second.begin(),
      writerIterEnd = iter->second.end(); writerIter != writerIterEnd; ++writerIter)
    {
      delete *writerIter;
    }
  }
}

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
    mitk::PointSetVtkMapper2D::SetDefaultProperties(node);
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
    CreateFileExtensionsMap();

    //RegisterLegacyReaders(this);
    //RegisterLegacyWriters(this);

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
      else if((dynamic_cast<PlaneGeometryData*>(data)!=NULL))
      {
        newMapper = mitk::PlaneGeometryDataMapper2D::New();
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
        newMapper = mitk::PointSetVtkMapper2D::New();
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
      else if((dynamic_cast<PlaneGeometryData*>(data)!=NULL))
      {
        newMapper = mitk::PlaneGeometryDataVtkMapper3D::New();
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

mitk::CoreObjectFactoryBase::MultimapType mitk::CoreObjectFactory::GetFileExtensionsMap()
{
  return m_FileExtensionsMap;
}

void mitk::CoreObjectFactory::CreateFileExtensionsMap()
{
  /*
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.dcm", "DICOM files"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.DCM", "DICOM files"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.dc3", "DICOM files"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.DC3", "DICOM files"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.gdcm", "DICOM files"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.seq", "DKFZ Pic"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.seq.gz", "DKFZ Pic"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.dcm", "Sets of 2D slices"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.gdcm", "Sets of 2D slices"));
  */
}

const char* mitk::CoreObjectFactory::GetSaveFileExtensions() {
  MultimapType aMap;
  for (ExtraFactoriesContainer::iterator it = m_ExtraFactories.begin(); it != m_ExtraFactories.end() ; it++ )
  {
    aMap = (*it)->GetSaveFileExtensionsMap();
    this->MergeFileExtensions(m_SaveFileExtensionsMap, aMap);

  }
  this->CreateFileExtensions(m_SaveFileExtensionsMap, m_SaveFileExtensions);
  return m_SaveFileExtensions.c_str();
}

mitk::CoreObjectFactoryBase::MultimapType mitk::CoreObjectFactory::GetSaveFileExtensionsMap()
{
  return m_SaveFileExtensionsMap;
}

mitk::CoreObjectFactory::FileWriterList mitk::CoreObjectFactory::GetFileWriters()
{
  FileWriterList allWriters = m_FileWriters;
  //sort to merge lists later on
  typedef std::set<mitk::FileWriterWithInformation::Pointer> FileWriterSet;
  FileWriterSet fileWritersSet;

  fileWritersSet.insert(allWriters.begin(), allWriters.end());

  //collect all extra factories
  for (ExtraFactoriesContainer::iterator it = m_ExtraFactories.begin();
       it != m_ExtraFactories.end(); it++ )
  {
    FileWriterList list2 = (*it)->GetFileWriters();

    //add them to the sorted set
    fileWritersSet.insert(list2.begin(), list2.end());
  }

  //write back to allWriters to return a list
  allWriters.clear();
  allWriters.insert(allWriters.end(), fileWritersSet.begin(), fileWritersSet.end());

  return allWriters;
}

void mitk::CoreObjectFactory::MapEvent(const mitk::Event*, const int)
{
}

std::string mitk::CoreObjectFactory::GetDescriptionForExtension(const std::string& extension)
{
  std::multimap<std::string, std::string> fileExtensionMap = GetSaveFileExtensionsMap();
  for(std::multimap<std::string, std::string>::iterator it = fileExtensionMap.begin(); it != fileExtensionMap.end(); it++)
    if (it->first == extension) return it->second;
  return ""; // If no matching extension was found, return emtpy string
}

void mitk::CoreObjectFactory::RegisterLegacyReaders(mitk::CoreObjectFactoryBase* factory)
{
  // We are not really interested in the string, just call the method since
  // many readers initialize the map the first time when this method is called
  factory->GetFileExtensions();

  std::map<std::string, std::vector<std::string> > extensionsByCategories;
  std::multimap<std::string, std::string> fileExtensionMap = factory->GetFileExtensionsMap();
  for(std::multimap<std::string, std::string>::iterator it = fileExtensionMap.begin(); it != fileExtensionMap.end(); it++)
  {
    std::string extension = it->first;
    // remove "*."
    extension = extension.erase(0,2);

    extensionsByCategories[it->second].push_back(extension);
  }

  for(std::map<std::string, std::vector<std::string> >::iterator iter = extensionsByCategories.begin(),
      endIter = extensionsByCategories.end(); iter != endIter; ++iter)
  {
    m_LegacyReaders[factory].push_back(new mitk::LegacyFileReaderService(iter->second, iter->first));
  }
}

void mitk::CoreObjectFactory::UnRegisterLegacyReaders(mitk::CoreObjectFactoryBase* factory)
{
  std::map<mitk::CoreObjectFactoryBase*, std::list<mitk::LegacyFileReaderService*> >::iterator iter = m_LegacyReaders.find(factory);
  if (iter != m_LegacyReaders.end())
  {
    for (std::list<mitk::LegacyFileReaderService*>::iterator readerIter = iter->second.begin(),
      readerIterEnd = iter->second.end(); readerIter != readerIterEnd; ++readerIter)
    {
      delete *readerIter;
    }

    m_LegacyReaders.erase(iter);
  }
}

void mitk::CoreObjectFactory::RegisterLegacyWriters(mitk::CoreObjectFactoryBase* factory)
{
  // Get all external Writers
  mitk::CoreObjectFactory::FileWriterList writers = factory->GetFileWriters();

  // We are not really interested in the string, just call the method since
  // many writers initialize the map the first time when this method is called
  factory->GetSaveFileExtensions();

  MultimapType fileExtensionMap = factory->GetSaveFileExtensionsMap();

  for(mitk::CoreObjectFactory::FileWriterList::iterator it = writers.begin(); it != writers.end(); it++)
  {
    std::vector<std::string> extensions = (*it)->GetPossibleFileExtensions();
    if (extensions.empty()) continue;

    std::string description;
    for(std::vector<std::string>::iterator ext = extensions.begin(); ext != extensions.end(); ext++)
    {
      if (ext->empty()) continue;

      std::string extension = *ext;
      std::string extensionWithStar = extension;
      if (extension.find_first_of('*') == 0)
      {
        // remove "*."
        extension = extension.substr(0, extension.size()-2);
      }
      else
      {
        extensionWithStar.insert(extensionWithStar.begin(), '*');
      }

      for(MultimapType::iterator fileExtensionIter = fileExtensionMap.begin();
          fileExtensionIter != fileExtensionMap.end(); fileExtensionIter++)
      {
        if (fileExtensionIter->first == extensionWithStar)
        {
          description = fileExtensionIter->second;
          break;
        }
      }
      if (!description.empty()) break;
    }
    if (description.empty())
    {
      description = std::string("Legacy ") + (*it)->GetNameOfClass() + " Reader";
    }

    mitk::FileWriter::Pointer fileWriter(it->GetPointer());
    mitk::LegacyFileWriterService* lfws = new mitk::LegacyFileWriterService(fileWriter, description);
    m_LegacyWriters[factory].push_back(lfws);
  }

}

void mitk::CoreObjectFactory::UnRegisterLegacyWriters(mitk::CoreObjectFactoryBase* factory)
{
  std::map<mitk::CoreObjectFactoryBase*, std::list<mitk::LegacyFileWriterService*> >::iterator iter = m_LegacyWriters.find(factory);
  if (iter != m_LegacyWriters.end())
  {
    for (std::list<mitk::LegacyFileWriterService*>::iterator writerIter = iter->second.begin(),
      writerIterEnd = iter->second.end(); writerIter != writerIterEnd; ++writerIter)
    {
      delete *writerIter;
    }

    m_LegacyWriters.erase(iter);
  }
}

