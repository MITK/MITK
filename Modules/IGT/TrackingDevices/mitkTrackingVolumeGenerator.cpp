/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTrackingVolumeGenerator.h"
#include "mitkStandardFileLocations.h"
#include "mitkConfig.h"
#include <vtkCubeSource.h>
#include <mitkTrackingTypes.h>
#include <mitkTrackingDevice.h>
#include <mitkVirtualTrackingDevice.h>
#include <vtkSmartPointer.h>
#include <mitkSurface.h>
#include <mitkIOMimeTypes.h>
#include <mitkFileReaderRegistry.h>

#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

#include <mitkCoreServices.h>
#include <mitkIFileReader.h>
#include <mitkIOUtil.h>
#include <mitkIOMimeTypes.h>
#include <mitkMimeType.h>
#include <mitkIMimeTypeProvider.h>

#include "mitkUnspecifiedTrackingTypeInformation.h"
#include "mitkTrackingDeviceTypeCollection.h"

#include <algorithm>

namespace {

//! Workaround until IOUtil::Load will guarantee to load mitk::Surface
//! even in presence of reader services that load STL as another type of
//! BaseData (T19825).
mitk::Surface::Pointer LoadCoreSurface(const us::ModuleResource& usResource)
{
  us::ModuleResourceStream resStream(usResource, std::ios_base::in);

  mitk::CoreServicePointer<mitk::IMimeTypeProvider> mimeTypeProvider(mitk::CoreServices::GetMimeTypeProvider());
  // get mime types for file (extension) and for surfaces
  std::vector<mitk::MimeType> mimetypesForFile = mimeTypeProvider->GetMimeTypesForFile(usResource.GetResourcePath());
  std::vector<mitk::MimeType> mimetypesForSurface = mimeTypeProvider->GetMimeTypesForCategory(mitk::IOMimeTypes::CATEGORY_SURFACES());

  // construct our candidates as the intersection of both sets because we need something that
  // handles the type type _and_ produces a surface out of it
  std::vector<mitk::MimeType> mimetypes;

  std::sort(mimetypesForFile.begin(), mimetypesForFile.end());
  std::sort(mimetypesForSurface.begin(), mimetypesForSurface.end());
  std::set_intersection(mimetypesForFile.begin(), mimetypesForFile.end(),
                        mimetypesForSurface.begin(), mimetypesForSurface.end(),
                        std::back_inserter(mimetypes));

  mitk::Surface::Pointer surface;
  if (mimetypes.empty())
  {
    mitkThrow() << "No mimetype for resource stream: " << usResource.GetResourcePath();
    return surface;
  }

  mitk::FileReaderRegistry fileReaderRegistry;
  std::vector<us::ServiceReference<mitk::IFileReader>> refs = fileReaderRegistry.GetReferences(mimetypes[0]);
  if (refs.empty())
  {
    mitkThrow() << "No reader available for resource stream: " << usResource.GetResourcePath();
    return surface;
  }

  mitk::IFileReader *reader = fileReaderRegistry.GetReader(refs[0]);
  reader->SetInput(usResource.GetResourcePath(), &resStream);
  auto basedatas = reader->Read();
  if (!basedatas.empty())
  {
    surface = dynamic_cast<mitk::Surface*>(basedatas.front().GetPointer());
  }

  return surface;
}

} // unnamed namespace


mitk::TrackingVolumeGenerator::TrackingVolumeGenerator()
{
  m_Data = mitk::UnspecifiedTrackingTypeInformation::GetDeviceDataUnspecified();
}

void mitk::TrackingVolumeGenerator::SetTrackingDevice (mitk::TrackingDevice::Pointer tracker)
{
  std::vector<us::ServiceReference<mitk::TrackingDeviceTypeCollection> > refs = us::GetModuleContext()->GetServiceReferences<mitk::TrackingDeviceTypeCollection>();
  if (refs.empty())
  {
    MITK_ERROR << "No tracking device service found!";
  }
  mitk::TrackingDeviceTypeCollection* deviceTypeCollection = us::GetModuleContext()->GetService<mitk::TrackingDeviceTypeCollection>(refs.front());

  this->m_Data = deviceTypeCollection->GetFirstCompatibleDeviceDataForLine(tracker->GetType());
}

void mitk::TrackingVolumeGenerator::GenerateData()
{
  mitk::Surface::Pointer output = this->GetOutput();  //the surface wich represents the tracking volume

  std::string filename = this->m_Data.VolumeModelLocation; // Name of the file or possibly a magic String, e.g. "cube"

  MITK_INFO << "volume: " << filename;

  // See if filename matches a magic string.
  if (filename.compare("cube") == 0){
    vtkSmartPointer<vtkCubeSource> cubeSource = vtkSmartPointer<vtkCubeSource>::New();
    double bounds[6];
    bounds[0] = bounds[2] = bounds[4] = -400.0;  // initialize bounds to -400 ... +400 cube. This is the default value of the
    bounds[1] = bounds[3] = bounds[5] =  400.0;  // virtual tracking device, but it can be changed. In that case,
    // the tracking volume polydata has to be updated manually
    cubeSource->SetBounds(bounds);
    cubeSource->Update();

    output->SetVtkPolyData(cubeSource->GetOutput()); //set the vtkCubeSource as polyData of the surface
    return;
  }
  if (filename.compare("") == 0) // empty String means no model, return empty output
  {
    // initialize with empty poly data (otherwise old surfaces may be returned) => so an empty surface is returned
    vtkSmartPointer<vtkPolyData> emptyPolyData = vtkSmartPointer<vtkPolyData>::New();
    output->SetVtkPolyData(emptyPolyData);
    return;
  }

  // from here on, we assume that filename contains an actual filename and not a magic string

  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource moduleResource = module->GetResource(filename);

  // TODO one would want to call mitk::IOUtils::Load(moduleResource) here.
  //      However this function is not guaranteed to find a reader that loads
  //      named resource as a Surface (given the presence of alternative readers
  //      that produce another data type but has a higher ranking than the core
  //      surface reader) - see bug T22608.
  mitk::Surface::Pointer fileoutput = LoadCoreSurface(moduleResource);

  if (fileoutput == nullptr)
  {
     MITK_ERROR << "Exception while casting data loaded from file: " << moduleResource.GetResourcePath();
     output->SetVtkPolyData(vtkSmartPointer<vtkPolyData>(vtkPolyData::New()));
  }
  else
  {
      output->SetVtkPolyData(fileoutput->GetVtkPolyData());
  }
}

void mitk::TrackingVolumeGenerator::SetTrackingDeviceType(mitk::TrackingDeviceType deviceType)
{
  std::vector<us::ServiceReference<mitk::TrackingDeviceTypeCollection> > refs = us::GetModuleContext()->GetServiceReferences<mitk::TrackingDeviceTypeCollection>();
  if (refs.empty())
  {
    MITK_ERROR << "No tracking device service found!";
  }
  mitk::TrackingDeviceTypeCollection* deviceTypeCollection = us::GetModuleContext()->GetService<mitk::TrackingDeviceTypeCollection>(refs.front());

  m_Data = deviceTypeCollection->GetFirstCompatibleDeviceDataForLine(deviceType);
}

mitk::TrackingDeviceType mitk::TrackingVolumeGenerator::GetTrackingDeviceType() const
{
  return m_Data.Line;
}

void mitk::TrackingVolumeGenerator::SetTrackingDeviceData(mitk::TrackingDeviceData deviceData)
{
  m_Data= deviceData;
}

mitk::TrackingDeviceData mitk::TrackingVolumeGenerator::GetTrackingDeviceData() const
{
  return m_Data;
}
