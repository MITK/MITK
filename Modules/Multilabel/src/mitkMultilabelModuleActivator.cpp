/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usModuleActivator.h>
#include <usModuleContext.h>

#include <mitkBaseRenderer.h>
#include <mitkCoreServices.h>
#include <mitkIPropertyFilters.h>
#include <mitkLabelSetImage.h>
#include <mitkLabelSetImageVtkMapper2D.h>
#include <mitkMapperProviderBase.h>
#include <mitkMultiLabelSegmentationVtkMapper3D.h>
#include <mitkPropertyFilter.h>

#include <memory>
#include <vector>

namespace mitk
{
  /**
   * \brief Module activator for the Multilabel module.
   *
   * Registers the mapper providers for mitk::MultiLabelSegmentation and
   * blacklists binary-image properties that do not apply to multi-label
   * segmentations.
   */
  class MultilabelModuleActivator : public us::ModuleActivator
  {
  public:
    void Load(us::ModuleContext *) override
    {
      m_MapperProviders.push_back(
        std::make_unique<MapperProviderBase<LabelSetImageVtkMapper2D, MultiLabelSegmentation>>(
          BaseRenderer::Standard2D));
      m_MapperProviders.push_back(
        std::make_unique<MapperProviderBase<MultiLabelSegmentationVtkMapper3D, MultiLabelSegmentation>>(
          BaseRenderer::Standard3D));

      CoreServicePointer<IPropertyFilters> propertyFilters(CoreServices::GetPropertyFilters());

      PropertyFilter labelSetImageFilter;
      labelSetImageFilter.AddEntry("binaryimage.hoveringannotationcolor", PropertyFilter::Blacklist);
      labelSetImageFilter.AddEntry("binaryimage.hoveringcolor", PropertyFilter::Blacklist);
      labelSetImageFilter.AddEntry("binaryimage.selectedannotationcolor", PropertyFilter::Blacklist);
      labelSetImageFilter.AddEntry("binaryimage.selectedcolor", PropertyFilter::Blacklist);
      labelSetImageFilter.AddEntry("outline binary shadow color", PropertyFilter::Blacklist);

      propertyFilters->AddFilter(labelSetImageFilter, "MultiLabelSegmentation");
    }

    void Unload(us::ModuleContext *) override
    {
      m_MapperProviders.clear();
    }

  private:
    std::vector<std::unique_ptr<IMapperProvider>> m_MapperProviders;
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::MultilabelModuleActivator)
