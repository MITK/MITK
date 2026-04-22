/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCoreObjectFactory.h>
#include <mitkConfig.h>

#include <mitkColorProperty.h>
#include <mitkDataNode.h>
#include <mitkEnumerationProperty.h>
#include <mitkGeometry3D.h>
#include <mitkGeometryData.h>
#include <mitkImage.h>
#include <mitkLevelWindowProperty.h>
#include <mitkLookupTable.h>
#include <mitkLookupTableProperty.h>
#include <mitkPlaneGeometry.h>
#include <mitkPlaneGeometryData.h>
#include <mitkPlaneGeometryDataMapper2D.h>
#include <mitkPlaneGeometryDataVtkMapper3D.h>
#include <mitkPointSet.h>
#include <mitkPointSetVtkMapper2D.h>
#include <mitkPointSetVtkMapper3D.h>
#include <mitkProperties.h>
#include <mitkPropertyList.h>
#include <mitkSlicedGeometry3D.h>
#include <mitkSmartPointerProperty.h>
#include <mitkStringProperty.h>
#include <mitkSurface.h>
#include <mitkSurfaceVtkMapper2D.h>
#include <mitkSurfaceVtkMapper3D.h>
#include <mitkTimeGeometry.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkVtkInterpolationProperty.h>
#include <mitkVtkRepresentationProperty.h>
#include <mitkVtkResliceInterpolationProperty.h>
#include <mitkImageVtkMapper2D.h>

#include <mitkCoreServices.h>

#include <mitkCrosshairData.h>
#include <mitkCrosshairVtkMapper2D.h>

void mitk::CoreObjectFactory::RegisterExtraFactory(CoreObjectFactoryBase *factory)
{
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
  catch ( const std::exception &e )
  {
    MITK_ERROR << "Caught exception while unregistering: " << e.what();
  }
}

mitk::CoreObjectFactory::Pointer mitk::CoreObjectFactory::GetInstance()
{
  static mitk::CoreObjectFactory::Pointer instance;
  if (instance.IsNull())
  {
    instance = mitk::CoreObjectFactory::New();
  }
  return instance;
}

mitk::CoreObjectFactory::~CoreObjectFactory()
{
}

void mitk::CoreObjectFactory::SetDefaultProperties(mitk::DataNode *node)
{
  if (node == nullptr)
    return;

  mitk::DataNode::Pointer nodePointer = node;

  mitk::Image* image = dynamic_cast<mitk::Image *>(node->GetData());
  if (nullptr != image && image->IsInitialized())
  {
    mitk::ImageVtkMapper2D::SetDefaultProperties(node);
  }

  if (nullptr != dynamic_cast<mitk::PlaneGeometryData*>(node->GetData()))
  {
    mitk::PlaneGeometryDataMapper2D::SetDefaultProperties(node);
  }

  if (nullptr != dynamic_cast<mitk::Surface*>(node->GetData()))
  {
    mitk::SurfaceVtkMapper2D::SetDefaultProperties(node);
    mitk::SurfaceVtkMapper3D::SetDefaultProperties(node);
  }

  if (nullptr != dynamic_cast<mitk::PointSet*>(node->GetData()))
  {
    mitk::PointSetVtkMapper2D::SetDefaultProperties(node);
    mitk::PointSetVtkMapper3D::SetDefaultProperties(node);
  }

  if (nullptr != dynamic_cast<mitk::CrosshairData*>(node->GetData()))
  {
    mitk::CrosshairVtkMapper2D::SetDefaultProperties(node);
  }

  for (auto it = m_ExtraFactories.begin(); it != m_ExtraFactories.end(); ++it)
  {
    (*it)->SetDefaultProperties(node);
  }
}

mitk::CoreObjectFactory::CoreObjectFactory()
{
}

mitk::Mapper::Pointer mitk::CoreObjectFactory::CreateMapper(mitk::DataNode *node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper = nullptr;
  mitk::Mapper::Pointer tmpMapper = nullptr;

  // check whether extra factories provide mapper
  for (auto it = m_ExtraFactories.begin(); it != m_ExtraFactories.end(); ++it)
  {
    tmpMapper = (*it)->CreateMapper(node, id);
    if (tmpMapper.IsNotNull())
      newMapper = tmpMapper;
  }

  if (newMapper.IsNull())
  {
    mitk::BaseData *data = node->GetData();

    if (id == mitk::BaseRenderer::Standard2D)
    {
      if ((dynamic_cast<Image *>(data) != nullptr))
      {
        newMapper = mitk::ImageVtkMapper2D::New();
        newMapper->SetDataNode(node);
      }
      else if ((dynamic_cast<PlaneGeometryData *>(data) != nullptr))
      {
        newMapper = mitk::PlaneGeometryDataMapper2D::New();
        newMapper->SetDataNode(node);
      }
      else if ((dynamic_cast<Surface *>(data) != nullptr))
      {
        newMapper = mitk::SurfaceVtkMapper2D::New();
        // cast because SetDataNode is not virtual
        auto *castedMapper = dynamic_cast<mitk::SurfaceVtkMapper2D *>(newMapper.GetPointer());
        castedMapper->SetDataNode(node);
      }
      else if ((dynamic_cast<PointSet *>(data) != nullptr))
      {
        newMapper = mitk::PointSetVtkMapper2D::New();
        newMapper->SetDataNode(node);
      }
    }
    else if (id == mitk::BaseRenderer::Standard3D)
    {
      if ((dynamic_cast<PlaneGeometryData *>(data) != nullptr))
      {
        newMapper = mitk::PlaneGeometryDataVtkMapper3D::New();
        newMapper->SetDataNode(node);
      }
      else if ((dynamic_cast<Surface *>(data) != nullptr))
      {
        newMapper = mitk::SurfaceVtkMapper3D::New();
        newMapper->SetDataNode(node);
      }
      else if ((dynamic_cast<PointSet *>(data) != nullptr))
      {
        newMapper = mitk::PointSetVtkMapper3D::New();
        newMapper->SetDataNode(node);
      }
    }
  }

  return newMapper;
}

void mitk::CoreObjectFactory::MapEvent(const mitk::Event *, const int)
{
}
