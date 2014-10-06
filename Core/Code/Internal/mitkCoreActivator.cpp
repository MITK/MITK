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
#include <mitkIOMimeTypes.h>
#include <Internal/mitkItkImageIO.h>
#include <Internal/mitkMimeTypeProvider.h>
#include <Internal/mitkPointSetReaderService.h>
#include <Internal/mitkPointSetWriterService.h>
#include <Internal/mitkRawImageFileReader.h>
#include <Internal/mitkSurfaceStlIO.h>
#include <Internal/mitkSurfaceVtkLegacyIO.h>
#include <Internal/mitkSurfaceVtkXmlIO.h>
#include <Internal/mitkImageVtkXmlIO.h>
#include <Internal/mitkImageVtkLegacyIO.h>

#include <mitkFileWriter.h>
#include "mitkLegacyFileWriterService.h"

#include <itkNiftiImageIO.h>

// Micro Services
#include <usGetModuleContext.h>
#include <usModuleInitialization.h>
#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <usModuleSettings.h>
#include <usModuleEvent.h>
#include <usModule.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>
#include <usServiceTracker.h>


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

class ShaderRepositoryTracker : public us::ServiceTracker<mitk::IShaderRepository>
{

public:

  ShaderRepositoryTracker()
    : Superclass(us::GetModuleContext())
  {
  }

  virtual void Close()
  {
    us::GetModuleContext()->RemoveModuleListener(this, &ShaderRepositoryTracker::HandleModuleEvent);
    Superclass::Close();
  }

  virtual void Open()
  {
    us::GetModuleContext()->AddModuleListener(this, &ShaderRepositoryTracker::HandleModuleEvent);
    Superclass::Open();
  }

private:

  typedef us::ServiceTracker<mitk::IShaderRepository> Superclass;

  TrackedType AddingService(const ServiceReferenceType &reference)
  {
    mitk::IShaderRepository* shaderRepo = Superclass::AddingService(reference);
    if (shaderRepo)
    {
      // Add all existing shaders from modules to the new shader repository.
      // If the shader repository is registered in a modules activator, the
      // GetLoadedModules() function call below will also return the module
      // which is currently registering the repository. The HandleModuleEvent
      // method contains code to avoid double registrations due to a fired
      // ModuleEvent::LOADED event after the activators Load() method finished.
      std::vector<us::Module*> modules = us::ModuleRegistry::GetLoadedModules();
      for (std::vector<us::Module*>::const_iterator iter = modules.begin(),
           endIter = modules.end(); iter != endIter; ++iter)
      {
        this->AddModuleShaderToRepository(*iter, shaderRepo);
      }

      m_ShaderRepositories.push_back(shaderRepo);
    }
    return shaderRepo;
  }

  void RemovedService(const ServiceReferenceType& /*reference*/, TrackedType tracked)
  {
    m_ShaderRepositories.erase(std::remove(m_ShaderRepositories.begin(), m_ShaderRepositories.end(), tracked),
                               m_ShaderRepositories.end());
  }

  void HandleModuleEvent(const us::ModuleEvent moduleEvent)
  {
    if (moduleEvent.GetType() == us::ModuleEvent::LOADED)
    {
      std::vector<mitk::IShaderRepository*> shaderRepos;
      for (std::map<mitk::IShaderRepository*, std::map<long, std::vector<int> > >::const_iterator shaderMapIter = m_ModuleIdToShaderIds.begin(),
           shaderMapEndIter = m_ModuleIdToShaderIds.end(); shaderMapIter != shaderMapEndIter; ++shaderMapIter)
      {
        if (shaderMapIter->second.find(moduleEvent.GetModule()->GetModuleId()) == shaderMapIter->second.end())
        {
          shaderRepos.push_back(shaderMapIter->first);
        }
      }
      AddModuleShadersToRepositories(moduleEvent.GetModule(), shaderRepos);
    }
    else if (moduleEvent.GetType() == us::ModuleEvent::UNLOADED)
    {
      RemoveModuleShadersFromRepositories(moduleEvent.GetModule(), m_ShaderRepositories);
    }
  }

  void AddModuleShadersToRepositories(us::Module* module, const std::vector<mitk::IShaderRepository*>& shaderRepos)
  {
    // search and load shader files
    std::vector<us::ModuleResource> shaderResources = module->FindResources("Shaders", "*.xml", true);
    for (std::vector<us::ModuleResource>::iterator i = shaderResources.begin();
         i != shaderResources.end(); ++i)
    {
      if (*i)
      {
        us::ModuleResourceStream rs(*i);
        for (std::vector<mitk::IShaderRepository*>::const_iterator shaderRepoIter = shaderRepos.begin(),
             shaderRepoEndIter = shaderRepos.end(); shaderRepoIter != shaderRepoEndIter; ++shaderRepoIter)
        {
          int id = (*shaderRepoIter)->LoadShader(rs, i->GetBaseName());
          if (id >= 0)
          {
            m_ModuleIdToShaderIds[*shaderRepoIter][module->GetModuleId()].push_back(id);
          }
        }
        rs.seekg(0, std::ios_base::beg);
      }
    }
  }

  void AddModuleShaderToRepository(us::Module* module, mitk::IShaderRepository* shaderRepo)
  {
    std::vector<mitk::IShaderRepository*> shaderRepos;
    shaderRepos.push_back(shaderRepo);
    this->AddModuleShadersToRepositories(module, shaderRepos);
  }

  void RemoveModuleShadersFromRepositories(us::Module* module,
                                           const std::vector<mitk::IShaderRepository*>& shaderRepos)
  {
    for (std::vector<mitk::IShaderRepository*>::const_iterator shaderRepoIter = shaderRepos.begin(),
         shaderRepoEndIter = shaderRepos.end(); shaderRepoIter != shaderRepoEndIter; ++shaderRepoIter)
    {
      std::map<long, std::vector<int> >& moduleIdToShaderIds = m_ModuleIdToShaderIds[*shaderRepoIter];
      std::map<long, std::vector<int> >::iterator shaderIdsIter =
        moduleIdToShaderIds.find(module->GetModuleId());
      if (shaderIdsIter != moduleIdToShaderIds.end())
      {
        for (std::vector<int>::iterator idIter = shaderIdsIter->second.begin();
             idIter != shaderIdsIter->second.end(); ++idIter)
        {
          (*shaderRepoIter)->UnloadShader(*idIter);
        }
        moduleIdToShaderIds.erase(shaderIdsIter);
      }
    }
  }

private:

  // Maps to each shader repository a map containing module ids and related
  // shader registration ids
  std::map<mitk::IShaderRepository*, std::map<long, std::vector<int> > > m_ModuleIdToShaderIds;
  std::vector<mitk::IShaderRepository*> m_ShaderRepositories;
};

class FixedNiftiImageIO : public itk::NiftiImageIO
{
public:

  /** Standard class typedefs. */
  typedef FixedNiftiImageIO         Self;
  typedef itk::NiftiImageIO         Superclass;
  typedef itk::SmartPointer< Self > Pointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self)

  /** Run-time type information (and related methods). */
  itkTypeMacro(FixedNiftiImageIO, Superclass)

  virtual bool SupportsDimension(unsigned long dim)
  {
    return dim > 1 && dim < 5;
  }

};

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

  m_ShaderRepositoryTracker.reset(new ShaderRepositoryTracker);

  //m_RenderingManager = mitk::RenderingManager::New();
  //context->RegisterService<mitk::RenderingManager>(renderingManager.GetPointer());
  m_PlanePositionManager.reset(new mitk::PlanePositionManagerService);
  context->RegisterService<mitk::PlanePositionManagerService>(m_PlanePositionManager.get());

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

  this->RegisterDefaultMimeTypes();
  this->RegisterItkReaderWriter();
  this->RegisterVtkReaderWriter();

  // Add custom Reader / Writer Services
  m_FileReaders.push_back(new mitk::PointSetReaderService());
  m_FileWriters.push_back(new mitk::PointSetWriterService());
  m_FileReaders.push_back(new mitk::RawImageFileReader());

  m_ShaderRepositoryTracker->Open();

  /*
    There IS an option to exchange ALL vtkTexture instances against vtkNeverTranslucentTextureFactory.
    This code is left here as a reminder, just in case we might need to do that some time.

    vtkNeverTranslucentTextureFactory* textureFactory = vtkNeverTranslucentTextureFactory::New();
    vtkObjectFactory::RegisterFactory( textureFactory );
    textureFactory->Delete();
    */

  this->RegisterLegacyWriter();
}

void MitkCoreActivator::Unload(us::ModuleContext* )
{
  for(std::vector<mitk::IFileReader*>::iterator iter = m_FileReaders.begin(),
      endIter = m_FileReaders.end(); iter != endIter; ++iter)
  {
    delete *iter;
  }

  for(std::vector<mitk::IFileWriter*>::iterator iter = m_FileWriters.begin(),
      endIter = m_FileWriters.end(); iter != endIter; ++iter)
  {
    delete *iter;
  }

  for(std::vector<mitk::AbstractFileIO*>::iterator iter = m_FileIOs.begin(),
      endIter = m_FileIOs.end(); iter != endIter; ++iter)
  {
    delete *iter;
  }

  for(std::vector<mitk::IFileWriter*>::iterator iter = m_LegacyWriters.begin(),
      endIter = m_LegacyWriters.end(); iter != endIter; ++iter)
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

  m_ShaderRepositoryTracker->Close();
}

void MitkCoreActivator::RegisterDefaultMimeTypes()
{
  // Register some default mime-types

  std::vector<mitk::CustomMimeType> mimeTypes = mitk::IOMimeTypes::Get();
  for (std::vector<mitk::CustomMimeType>::const_iterator mimeTypeIter = mimeTypes.begin(),
       iterEnd = mimeTypes.end(); mimeTypeIter != iterEnd; ++mimeTypeIter)
  {
    m_DefaultMimeTypes.push_back(*mimeTypeIter);
    m_Context->RegisterService(&m_DefaultMimeTypes.back());
  }
}

void MitkCoreActivator::RegisterItkReaderWriter()
{
  std::list<itk::LightObject::Pointer> allobjects =
    itk::ObjectFactoryBase::CreateAllInstance("itkImageIOBase");
  for (std::list<itk::LightObject::Pointer >::iterator i = allobjects.begin(),
       endIter = allobjects.end(); i != endIter; ++i)
  {
    itk::ImageIOBase* io = dynamic_cast<itk::ImageIOBase*>(i->GetPointer());

    // NiftiImageIO does not provide a correct "SupportsDimension()" methods
    // and the supported read/write extensions are not ordered correctly
    if (dynamic_cast<itk::NiftiImageIO*>(io)) continue;

    if (io)
    {
      m_FileIOs.push_back(new mitk::ItkImageIO(io));
    }
    else
    {
      MITK_WARN << "Error ImageIO factory did not return an ImageIOBase: "
                << ( *i )->GetNameOfClass();
    }
  }

  mitk::ItkImageIO* io = new mitk::ItkImageIO(mitk::CustomMimeType(mitk::IOMimeTypes::NIFTI_MIMETYPE_NAME()),
                                              new FixedNiftiImageIO(), 0);
  m_FileIOs.push_back(io);
}

void MitkCoreActivator::RegisterVtkReaderWriter()
{
  m_FileIOs.push_back(new mitk::SurfaceVtkXmlIO());
  m_FileIOs.push_back(new mitk::SurfaceStlIO());
  m_FileIOs.push_back(new mitk::SurfaceVtkLegacyIO());

  m_FileIOs.push_back(new mitk::ImageVtkXmlIO());
  m_FileIOs.push_back(new mitk::ImageVtkLegacyIO());
}

void MitkCoreActivator::RegisterLegacyWriter()
{
  std::list<itk::LightObject::Pointer> allobjects = itk::ObjectFactoryBase::CreateAllInstance("IOWriter");

  for( std::list<itk::LightObject::Pointer>::iterator i = allobjects.begin();
       i != allobjects.end(); ++i)
  {
    mitk::FileWriter::Pointer io = dynamic_cast<mitk::FileWriter*>(i->GetPointer());
    if(io)
    {
      std::string description = std::string("Legacy ") + io->GetNameOfClass() + " Reader";
      mitk::IFileWriter* writer = new mitk::LegacyFileWriterService(io, description);
      m_LegacyWriters.push_back(writer);
    }
    else
    {
      MITK_ERROR << "Error IOWriter override is not of type mitk::FileWriter: "
                 << (*i)->GetNameOfClass()
                 << std::endl;
    }
  }
}

US_EXPORT_MODULE_ACTIVATOR(MitkCore, MitkCoreActivator)

// Call CppMicroservices initialization code at the end of the file.
// This especially ensures that VTK object factories have already
// been registered (VTK initialization code is injected by implicitly
// include VTK header files at the top of this file).
US_INITIALIZE_MODULE("MitkCore", "MitkCore")
