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

#include "mitkCoreActivator.h"

// File IO
#include <mitkIOUtil.h>
#include <Internal/mitkItkFileReaderService.h>
#include <Internal/mitkMimeTypeProvider.h>
#include <Internal/mitkPointSetReaderService.h>

// Micro Services
#include <usModuleInitialization.h>
#include <usModuleContext.h>
#include <usModuleSettings.h>
#include <usModule.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>
#include <usSharedLibrary.h>


void HandleMicroServicesMessages(us::MsgType type, const char* msg)
{
  switch (type)
  {
  case us::DebugMsg:
    MITK_DEBUG << msg;
    break;
  case us::InfoMsg:
    MITK_INFO << msg;
    break;
  case us::WarningMsg:
    MITK_WARN << msg;
    break;
  case us::ErrorMsg:
    MITK_ERROR << msg;
    break;
  }
}

void AddMitkAutoLoadPaths(const std::string& programPath)
{
  us::ModuleSettings::AddAutoLoadPath(programPath);
#ifdef __APPLE__
  // Walk up three directories since that is where the .dylib files are located
  // for build trees.
  std::string additionalPath = programPath;
  bool addPath = true;
  for(int i = 0; i < 3; ++i)
  {
    std::size_t index = additionalPath.find_last_of('/');
    if (index != std::string::npos)
    {
      additionalPath = additionalPath.substr(0, index);
    }
    else
    {
      addPath = false;
      break;
    }
  }
  if (addPath)
  {
    us::ModuleSettings::AddAutoLoadPath(additionalPath);
  }
#endif
}

void MitkCoreActivator::Load(us::ModuleContext* context)
{
  // Handle messages from CppMicroServices
  us::installMsgHandler(HandleMicroServicesMessages);

  this->m_Context = context;

  // Add the current application directory to the auto-load paths.
  // This is useful for third-party executables.
  std::string programPath = mitk::IOUtil::GetProgramPath();
  if (programPath.empty())
  {
    MITK_WARN << "Could not get the program path.";
  }
  else
  {
    AddMitkAutoLoadPaths(programPath);
  }

  //m_RenderingManager = mitk::RenderingManager::New();
  //context->RegisterService<mitk::RenderingManager>(renderingManager.GetPointer());
  m_PlanePositionManager.reset(new mitk::PlanePositionManagerService);
  context->RegisterService<mitk::PlanePositionManagerService>(m_PlanePositionManager.get());

  m_ShaderRepository.reset(new mitk::ShaderRepository);
  context->RegisterService<mitk::IShaderRepository>(m_ShaderRepository.get());

  m_PropertyAliases.reset(new mitk::PropertyAliases);
  context->RegisterService<mitk::IPropertyAliases>(m_PropertyAliases.get());

  m_PropertyDescriptions.reset(new mitk::PropertyDescriptions);
  context->RegisterService<mitk::IPropertyDescriptions>(m_PropertyDescriptions.get());

  m_PropertyExtensions.reset(new mitk::PropertyExtensions);
  context->RegisterService<mitk::IPropertyExtensions>(m_PropertyExtensions.get());

  m_PropertyFilters.reset(new mitk::PropertyFilters);
  context->RegisterService<mitk::IPropertyFilters>(m_PropertyFilters.get());

  m_MimeTypeProvider.reset(new mitk::MimeTypeProvider);
  m_MimeTypeProvider->Start();
  m_MimeTypeProviderReg = context->RegisterService<mitk::IMimeTypeProvider>(m_MimeTypeProvider.get());

  context->AddModuleListener(this, &MitkCoreActivator::HandleModuleEvent);

  this->RegisterMimeTypes();
  this->RegisterItkReaderWriter();

  // Explicitly load the LegacyIO module
  us::SharedLibrary legacyIOLib(programPath, "LegacyIO");
  legacyIOLib.Load();

  // Add Reader / Writer Services

  mitk::IFileReader* reader;
  //    mitk::IFileWriter* writer;
  reader = new mitk::PointSetReaderService();
  m_FileReaders.push_back(reader);

  /*
    There IS an option to exchange ALL vtkTexture instances against vtkNeverTranslucentTextureFactory.
    This code is left here as a reminder, just in case we might need to do that some time.

    vtkNeverTranslucentTextureFactory* textureFactory = vtkNeverTranslucentTextureFactory::New();
    vtkObjectFactory::RegisterFactory( textureFactory );
    textureFactory->Delete();
    */
}

void MitkCoreActivator::Unload(us::ModuleContext* )
{
  for(std::vector<mitk::IFileReader*>::iterator iter = m_FileReaders.begin(),
      endIter = m_FileReaders.end(); iter != endIter; ++iter)
  {
    delete *iter;
  }

  // The mitk::ModuleContext* argument of the Unload() method
  // will always be 0 for the Mitk library. It makes no sense
  // to use it at this stage anyway, since all libraries which
  // know about the module system have already been unloaded.

  // we need to close the internal service tracker of the
  // MimeTypeProvider class here. Otherwise it
  // would hold on to the ModuleContext longer than it is
  // actually valid.
  m_MimeTypeProviderReg.Unregister();
  m_MimeTypeProvider->Stop();
}

void MitkCoreActivator::HandleModuleEvent(const us::ModuleEvent moduleEvent)
{
  if (moduleEvent.GetType() == us::ModuleEvent::LOADED)
  {
    // search and load shader files
    std::vector<us::ModuleResource> shaderResoruces =
        moduleEvent.GetModule()->FindResources("Shaders", "*.xml", true);
    for (std::vector<us::ModuleResource>::iterator i = shaderResoruces.begin();
         i != shaderResoruces.end(); ++i)
    {
      if (*i)
      {
        us::ModuleResourceStream rs(*i);
        int id = m_ShaderRepository->LoadShader(rs, i->GetBaseName());
        if (id >= 0)
        {
          moduleIdToShaderIds[moduleEvent.GetModule()->GetModuleId()].push_back(id);
        }
      }
    }
  }
  else if (moduleEvent.GetType() == us::ModuleEvent::UNLOADED)
  {
    std::map<long, std::vector<int> >::iterator shaderIdsIter =
        moduleIdToShaderIds.find(moduleEvent.GetModule()->GetModuleId());
    if (shaderIdsIter != moduleIdToShaderIds.end())
    {
      for (std::vector<int>::iterator idIter = shaderIdsIter->second.begin();
           idIter != shaderIdsIter->second.end(); ++idIter)
      {
        m_ShaderRepository->UnloadShader(*idIter);
      }
      moduleIdToShaderIds.erase(shaderIdsIter);
    }
  }
}

void MitkCoreActivator::RegisterMimeTypes()
{
  // Register some default mime-types

  // 3D Images
  std::vector<std::string> mimeTypeExtensions;
  mimeTypeExtensions.push_back("dc3");
  mimeTypeExtensions.push_back("dcm");
  RegisterMimeType("application/dicom", "Images", "Dicom Images", mimeTypeExtensions);
  RegisterMimeType("application/vnd.mitk.pic", "Images", "DKFZ PIC Format", "pic");
  RegisterMimeType("application/vnd.mitk.pic+gz", "Images", "DKFZ Compressed PIC Format", "pic.gz");

  // REMOVE: Test multiple mime types for same extension
  RegisterMimeType("application/vnd.fancy", "Images", "Fancy Compressed PIC Format", "pic.gz");

  // 2D Images
  RegisterMimeType("image/bmp", "2D Images", "Bitmap Image", "bmp");
}

void MitkCoreActivator::RegisterItkReaderWriter()
{
  std::list<itk::ImageIOBase::Pointer> possibleImageIO;
  std::list<itk::LightObject::Pointer> allobjects =
    itk::ObjectFactoryBase::CreateAllInstance("itkImageIOBase");
  for (std::list<itk::LightObject::Pointer >::iterator i = allobjects.begin(),
       endIter = allobjects.end(); i != endIter; ++i)
  {
    itk::ImageIOBase* io = dynamic_cast<itk::ImageIOBase*>(i->GetPointer());
    if (io)
    {
      if(!io->GetSupportedReadExtensions().empty())
      {
        possibleImageIO.push_back(io);
      }
      else
      {
        MITK_INFO << "ImageIO factory " << io->GetNameOfClass() << " does not report its supported read extensions.";
      }
    }
    else
    {
      MITK_WARN << "Error ImageIO factory did not return an ImageIOBase: "
                << ( *i )->GetNameOfClass();
    }
  }
  for (std::list<itk::ImageIOBase::Pointer>::iterator k = possibleImageIO.begin(),
    endIter = possibleImageIO.end(); k != endIter; ++k)
  {
    m_FileReaders.push_back(new mitk::ItkFileReaderService((*k)->GetSupportedReadExtensions(), "ITK Image Reader"));
  }

  // Some ITK ImageIOBase sub-classes do not report supported read extensions.
  // Registering them manually here.
  std::vector<std::string> extensions;

  extensions.clear();
  extensions.push_back("gdcm");
  m_FileReaders.push_back(new mitk::ItkFileReaderService(extensions, "ITK Image Reader"));

  extensions.clear();
  extensions.push_back("png");
  extensions.push_back("PNG");
  m_FileReaders.push_back(new mitk::ItkFileReaderService(extensions, "ITK Image Reader"));

  extensions.clear();
  extensions.push_back("spr");
  m_FileReaders.push_back(new mitk::ItkFileReaderService(extensions, "ITK Image Reader"));

  extensions.clear();
  extensions.push_back("gipl");
  extensions.push_back("gipl.gz");
  m_FileReaders.push_back(new mitk::ItkFileReaderService(extensions, "ITK Image Reader"));

  extensions.clear();
  extensions.push_back("hdf");
  extensions.push_back("h4");
  extensions.push_back("hdf4");
  extensions.push_back("h5");
  extensions.push_back("hdf5");
  extensions.push_back("he4");
  extensions.push_back("he5");
  extensions.push_back("hd5");
  m_FileReaders.push_back(new mitk::ItkFileReaderService(extensions, "ITK Image Reader"));
}

void MitkCoreActivator::RegisterMimeType(const std::string& id, const std::string& category,
                                         const std::string& description, const std::string& extension)
{
  std::vector<std::string> extensions;
  extensions.push_back(extension);
  this->RegisterMimeType(id, category, description, extensions);
}

void MitkCoreActivator::RegisterMimeType(const std::string& id, const std::string& category,
                                         const std::string& description, const std::vector<std::string>& extensions)
{
  us::ServiceProperties mimeTypeProps;
  mimeTypeProps[mitk::IMimeType::PROP_ID()] = id;
  mimeTypeProps[mitk::IMimeType::PROP_CATEGORY()] = category;
  mimeTypeProps[mitk::IMimeType::PROP_DESCRIPTION()] = description;
  mimeTypeProps[mitk::IMimeType::PROP_EXTENSIONS()] = extensions;
  mimeTypeProps[us::ServiceConstants::SERVICE_RANKING()] = -100;
  m_Context->RegisterService<mitk::IMimeType>(&m_MimeType, mimeTypeProps);
}

US_EXPORT_MODULE_ACTIVATOR(Mitk, MitkCoreActivator)

// Call CppMicroservices initialization code at the end of the file.
// This especially ensures that VTK object factories have already
// been registered (VTK initialization code is injected by implicitly
// include VTK header files at the top of this file).
US_INITIALIZE_MODULE("Mitk", "Mitk")
