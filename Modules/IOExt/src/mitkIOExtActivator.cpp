/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkIOExtActivator.h"

#include "mitkObjFileReaderService.h"
#include "mitkPlyFileReaderService.h"
#include "mitkPlyFileWriterService.h"

#include <mitkBaseRenderer.h>
#include <mitkDataNode.h>
#include <mitkImage.h>
#include <mitkMapperProviderBase.h>
#include <mitkVolumeMapperVtkSmart3D.h>

#include <string>

namespace mitk
{
  namespace
  {
    /**
     * \brief Provider for VolumeMapperVtkSmart3D that only accepts plain images.
     *
     * Subclasses of Image are declined so that they do not get 3D volume
     * rendering unless they register a mapper on their own. The default
     * properties are deliberately not restricted the same way, matching the
     * historic behavior of the IOExt object factory.
     */
    class VolumeMapperProvider : public MapperProviderBase<VolumeMapperVtkSmart3D, Image>
    {
    public:
      using MapperProviderBase::MapperProviderBase;

      Mapper::Pointer CreateMapper(DataNode *node) const override
      {
        auto *data = node != nullptr ? node->GetData() : nullptr;

        if (data == nullptr || std::string("Image") != data->GetNameOfClass())
          return nullptr;

        return MapperProviderBase::CreateMapper(node);
      }
    };
  }

  void IOExtActivator::Load(us::ModuleContext *)
  {
    m_ObjReader.reset(new ObjFileReaderService());

    m_PlyReader.reset(new PlyFileReaderService());
    m_ObjWriter.reset(new PlyFileWriterService());

    m_VolumeMapperProvider.reset(new VolumeMapperProvider(BaseRenderer::Standard3D));
  }

  void IOExtActivator::Unload(us::ModuleContext *)
  {
    m_VolumeMapperProvider.reset();
  }
}

US_EXPORT_MODULE_ACTIVATOR(mitk::IOExtActivator)
