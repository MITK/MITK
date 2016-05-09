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
#include <mitkItkImageIO.h>
#include <mitkMimeTypeProvider.h>
#include <mitkPointSetReaderService.h>
#include <mitkPointSetWriterService.h>
#include <mitkGeometryDataReaderService.h>
#include <mitkGeometryDataWriterService.h>
#include <mitkRawImageFileReader.h>
#include <mitkSurfaceStlIO.h>
#include <mitkSurfaceVtkLegacyIO.h>
#include <mitkSurfaceVtkXmlIO.h>
#include <mitkImageVtkXmlIO.h>
#include <mitkImageVtkLegacyIO.h>

#include <mitkFileWriter.h>
#include "mitkLegacyFileWriterService.h"
#include "mitkDicomSeriesReaderService.h"

#include <itkNiftiImageIO.h>
#include <itkGDCMImageIO.h>

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

// ITK "injects" static initialization code for IO factories
// via the itkImageIOFactoryRegisterManager.h header (which
// is generated in the application library build directory).
// To ensure that the code is called *before* the CppMicroServices
// static initialization code (which triggers the Activator::Start
// method), we include the ITK header here.
#include <itkImageIOFactoryRegisterManager.h>

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

  virtual bool SupportsDimension(unsigned long dim) override
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

  m_PropertyPersistence.reset(new mitk::PropertyPersistence);
  context->RegisterService<mitk::IPropertyPersistence>(m_PropertyPersistence.get());

  m_MimeTypeProvider.reset(new mitk::MimeTypeProvider);
  m_MimeTypeProvider->Start();
  m_MimeTypeProviderReg = context->RegisterService<mitk::IMimeTypeProvider>(m_MimeTypeProvider.get());

  this->RegisterDefaultMimeTypes();
  this->RegisterItkReaderWriter();
  this->RegisterVtkReaderWriter();

  // Add custom Reader / Writer Services
  m_FileReaders.push_back(new mitk::PointSetReaderService());
  m_FileWriters.push_back(new mitk::PointSetWriterService());
  m_FileReaders.push_back(new mitk::GeometryDataReaderService());
  m_FileWriters.push_back(new mitk::GeometryDataWriterService());
  m_FileReaders.push_back(new mitk::DicomSeriesReaderService());
  m_FileReaders.push_back(new mitk::RawImageFileReaderService());

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
  for(auto & elem : m_FileReaders)
  {
    delete elem;
  }

  for(auto & elem : m_FileWriters)
  {
    delete elem;
  }

  for(auto & elem : m_FileIOs)
  {
    delete elem;
  }

  for(auto & elem : m_LegacyWriters)
  {
    delete elem;
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

  for (std::vector<mitk::CustomMimeType*>::const_iterator mimeTypeIter = m_DefaultMimeTypes.begin(),
       iterEnd = m_DefaultMimeTypes.end(); mimeTypeIter != iterEnd; ++mimeTypeIter)
  {
    delete *mimeTypeIter;
  }
}

void MitkCoreActivator::RegisterDefaultMimeTypes()
{
  // Register some default mime-types

  std::vector<mitk::CustomMimeType*> mimeTypes = mitk::IOMimeTypes::Get();
  for (std::vector<mitk::CustomMimeType*>::const_iterator mimeTypeIter = mimeTypes.begin(),
       iterEnd = mimeTypes.end(); mimeTypeIter != iterEnd; ++mimeTypeIter)
  {
    m_DefaultMimeTypes.push_back(*mimeTypeIter);
    m_Context->RegisterService(m_DefaultMimeTypes.back());
  }
}

void MitkCoreActivator::RegisterItkReaderWriter()
{
  std::list<itk::LightObject::Pointer> allobjects =
    itk::ObjectFactoryBase::CreateAllInstance("itkImageIOBase");

  for (auto & allobject : allobjects)
  {
    itk::ImageIOBase* io = dynamic_cast<itk::ImageIOBase*>(allobject.GetPointer());

    // NiftiImageIO does not provide a correct "SupportsDimension()" methods
    // and the supported read/write extensions are not ordered correctly
    if (dynamic_cast<itk::NiftiImageIO*>(io)) continue;

    // Use a custom mime-type for GDCMImageIO below
    if (dynamic_cast<itk::GDCMImageIO*>(allobject.GetPointer()))
    {
      // MITK provides its own DICOM reader (which internally uses GDCMImageIO).
      continue;
    }

    if (io)
    {
      m_FileIOs.push_back(new mitk::ItkImageIO(io));
    }
    else
    {
      MITK_WARN << "Error ImageIO factory did not return an ImageIOBase: "
                << ( allobject )->GetNameOfClass();
    }
  }

  FixedNiftiImageIO::Pointer itkNiftiIO = FixedNiftiImageIO::New();
  mitk::ItkImageIO* niftiIO = new mitk::ItkImageIO(mitk::IOMimeTypes::NIFTI_MIMETYPE(),
                                                   itkNiftiIO.GetPointer(), 0);
  m_FileIOs.push_back(niftiIO);
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
      std::string description = std::string("Legacy ") + io->GetNameOfClass() + " Writer";
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

US_EXPORT_MODULE_ACTIVATOR(MitkCoreActivator)

// Call CppMicroservices initialization code at the end of the file.
// This especially ensures that VTK object factories have already
// been registered (VTK initialization code is injected by implicitly
// include VTK header files at the top of this file).
US_INITIALIZE_MODULE
