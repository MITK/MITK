/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-06-18 15:59:04 +0200 (Do, 18 Jun 2009) $
Version:   $Revision: 16916 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkDiffusionImagingObjectFactory.h"

#include "mitkProperties.h"
#include "mitkBaseRenderer.h"
#include "mitkDataNode.h"

#include "mitkNrrdDiffusionImageIOFactory.h"
#include "mitkNrrdDiffusionImageWriterFactory.h"
#include "mitkNrrdDiffusionImageWriter.h"
#include "mitkDiffusionImage.h"

#include "mitkNrrdQBallImageIOFactory.h"
#include "mitkNrrdQBallImageWriterFactory.h"
#include "mitkNrrdQBallImageWriter.h"

#include "mitkNrrdTensorImageIOFactory.h"
#include "mitkNrrdTensorImageWriterFactory.h"
#include "mitkNrrdTensorImageWriter.h"

#include "mitkCompositeMapper.h"
#include "mitkDiffusionImageMapper.h"
#include "mitkGPUVolumeMapper3D.h"
#include "mitkVolumeDataVtkMapper3D.h"
#include "mitkTbssImageMapper.h"


//modernized fiberbundle datastrucutre
#include "mitkFiberBundleX.h"
#include "mitkFiberBundleXIOFactory.h"
#include "mitkFiberBundleXWriterFactory.h"
#include "mitkFiberBundleXWriter.h"
#include "mitkFiberBundleXMapper3D.h"
#include "mitkFiberBundleXMapper2D.h"

#include "mitkFiberBundleXThreadMonitorMapper3D.h"
#include "mitkFiberBundleXThreadMonitor.h"

#include "mitkNrrdTbssImageIOFactory.h"
#include "mitkNrrdTbssImageWriterFactory.h"
#include "mitkNrrdTbssImageWriter.h"

#include "mitkNrrdTbssRoiImageIOFactory.h"
#include "mitkNrrdTbssRoiImageWriterFactory.h"
#include "mitkNrrdTbssRoiImageWriter.h"


#include "mitkPlanarCircleMapper3D.h"
#include "mitkPlanarPolygonMapper3D.h"



typedef short DiffusionPixelType;
typedef char TbssRoiPixelType;
typedef float TbssPixelType;
typedef int TbssGradientPixelType;


typedef mitk::DiffusionImage<DiffusionPixelType> DiffusionImageShort;
typedef std::multimap<std::string, std::string> MultimapType;

mitk::DiffusionImagingObjectFactory::DiffusionImagingObjectFactory(bool /*registerSelf*/)
:CoreObjectFactoryBase()
{

  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_DEBUG << "DiffusionImagingObjectFactory c'tor" << std::endl;
    RegisterIOFactories();


    mitk::NrrdDiffusionImageIOFactory::RegisterOneFactory();
    mitk::NrrdQBallImageIOFactory::RegisterOneFactory();
    mitk::NrrdTensorImageIOFactory::RegisterOneFactory();
    mitk::NrrdTbssImageIOFactory::RegisterOneFactory();
    mitk::NrrdTbssRoiImageIOFactory::RegisterOneFactory();
    mitk::FiberBundleXIOFactory::RegisterOneFactory(); //modernized


    mitk::NrrdDiffusionImageWriterFactory::RegisterOneFactory();
    mitk::NrrdQBallImageWriterFactory::RegisterOneFactory();
    mitk::NrrdTensorImageWriterFactory::RegisterOneFactory();
    mitk::NrrdTbssImageWriterFactory::RegisterOneFactory();
    mitk::NrrdTbssRoiImageWriterFactory::RegisterOneFactory();
    mitk::FiberBundleXWriterFactory::RegisterOneFactory();//modernized


    m_FileWriters.push_back( NrrdDiffusionImageWriter<DiffusionPixelType>::New().GetPointer() );
    m_FileWriters.push_back( NrrdQBallImageWriter::New().GetPointer() );
    m_FileWriters.push_back( NrrdTensorImageWriter::New().GetPointer() );
    m_FileWriters.push_back( NrrdTbssImageWriter::New().GetPointer() );
    m_FileWriters.push_back( NrrdTbssRoiImageWriter::New().GetPointer() );
    m_FileWriters.push_back( mitk::FiberBundleXWriter::New().GetPointer() );//modernized


    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(this);
    CreateFileExtensionsMap();

    alreadyDone = true;
  }

}

mitk::Mapper::Pointer mitk::DiffusionImagingObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper=NULL;

  if ( id == mitk::BaseRenderer::Standard2D )
  {
    std::string classname("QBallImage");
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::CompositeMapper::New();
      newMapper->SetDataNode(node);
      node->SetMapper(3, ((CompositeMapper*)newMapper.GetPointer())->GetImageMapper());
    }
    classname = "TensorImage";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::CompositeMapper::New();
      newMapper->SetDataNode(node);
      node->SetMapper(3, ((CompositeMapper*)newMapper.GetPointer())->GetImageMapper());
    }

    classname = "DiffusionImage";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::DiffusionImageMapper<short>::New();
      newMapper->SetDataNode(node);
    }

    classname = "TbssRoiImage";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::ImageVtkMapper2D::New();
      newMapper->SetDataNode(node);
    }

    classname = "FiberBundleX";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::FiberBundleXMapper2D::New();
      newMapper->SetDataNode(node);
    }

    classname = "TbssImage";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::TbssImageMapper::New();
      newMapper->SetDataNode(node);
    }

  }
  else if ( id == mitk::BaseRenderer::Standard3D )
  {
    std::string classname("QBallImage");
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::GPUVolumeMapper3D::New();
      newMapper->SetDataNode(node);
    }
    classname = "TensorImage";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::GPUVolumeMapper3D::New();
      newMapper->SetDataNode(node);
    }
    classname = "DiffusionImage";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::GPUVolumeMapper3D::New();
      newMapper->SetDataNode(node);
    }

    classname = "FiberBundleX";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::FiberBundleXMapper3D::New();
      newMapper->SetDataNode(node);
    }

    classname = "FiberBundleXThreadMonitor";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::FiberBundleXThreadMonitorMapper3D::New();
      newMapper->SetDataNode(node);
    }

    classname = "TbssRoiImage";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::VolumeDataVtkMapper3D::New();
      newMapper->SetDataNode(node);
    }

    classname = "TbssImage";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::TbssImageMapper::New();
      newMapper->SetDataNode(node);
    }

    classname =  "PlanarCircle";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::PlanarCircleMapper3D::New();
      newMapper->SetDataNode(node);
    }

    classname = "PlanarPolygon";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::PlanarPolygonMapper3D::New();
      newMapper->SetDataNode(node);
    }

  }

  return newMapper;
}

void mitk::DiffusionImagingObjectFactory::SetDefaultProperties(mitk::DataNode* node)
{
  std::string classname = "QBallImage";
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::CompositeMapper::SetDefaultProperties(node);
    mitk::GPUVolumeMapper3D::SetDefaultProperties(node);
  }

  classname = "TensorImage";
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::CompositeMapper::SetDefaultProperties(node);
    mitk::GPUVolumeMapper3D::SetDefaultProperties(node);
  }

  classname = "DiffusionImage";
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::DiffusionImageMapper<short>::SetDefaultProperties(node);
    mitk::GPUVolumeMapper3D::SetDefaultProperties(node);
  }

  classname = "FiberBundleX";
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::FiberBundleXMapper3D::SetDefaultProperties(node);
    mitk::FiberBundleXMapper2D::SetDefaultProperties(node);
  }

  classname = "FiberBundleXThreadMonitor";
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::FiberBundleXThreadMonitorMapper3D::SetDefaultProperties(node);
  }

  classname = "TbssRoiImage";
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::ImageVtkMapper2D::SetDefaultProperties(node);
    mitk::VolumeDataVtkMapper3D::SetDefaultProperties(node);
  }

  classname = "TbssImage";
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::TbssImageMapper::SetDefaultProperties(node);
    mitk::GPUVolumeMapper3D::SetDefaultProperties(node);
  }

  classname = "PlanarCircle";
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::PlanarCircleMapper3D::SetDefaultProperties(node);
  }

  classname = "PlanarPolygon";
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::PlanarPolygonMapper3D::SetDefaultProperties(node);
  }


}

const char* mitk::DiffusionImagingObjectFactory::GetFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_FileExtensionsMap, fileExtension);
  return fileExtension.c_str();
};

mitk::CoreObjectFactoryBase::MultimapType mitk::DiffusionImagingObjectFactory::GetFileExtensionsMap()
{
  return m_FileExtensionsMap;
}

const char* mitk::DiffusionImagingObjectFactory::GetSaveFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_SaveFileExtensionsMap, fileExtension);
  return fileExtension.c_str();
};

mitk::CoreObjectFactoryBase::MultimapType mitk::DiffusionImagingObjectFactory::GetSaveFileExtensionsMap()
{
  return m_SaveFileExtensionsMap;
}

void mitk::DiffusionImagingObjectFactory::CreateFileExtensionsMap()
{
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.dwi", "Diffusion Weighted Images"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.hdwi", "Diffusion Weighted Images"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.nii", "Diffusion Weighted Images for FSL"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.fsl", "Diffusion Weighted Images for FSL"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.fslgz", "Diffusion Weighted Images for FSL"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.qbi", "Q-Ball Images"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.hqbi", "Q-Ball Images"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.dti", "Tensor Images"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.hdti", "Tensor Images"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.fib", "Fiber Bundle"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.vtk", "Fiber Bundle"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.tbss", "TBSS data"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.pf", "Planar Figure File"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.roi", "TBSS ROI data"));

  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.dwi", "Diffusion Weighted Images"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.hdwi", "Diffusion Weighted Images"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.nii", "Diffusion Weighted Images for FSL"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.fsl", "Diffusion Weighted Images for FSL"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.fslgz", "Diffusion Weighted Images for FSL"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.qbi", "Q-Ball Images"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.hqbi", "Q-Ball Images"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.dti", "Tensor Images"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.hdti", "Tensor Images"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.fib", "Fiber Bundle"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.vtk", "Fiber Bundle"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.tbss", "TBSS data"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.pf", "Planar Figure File"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.roi", "TBSS ROI data"));
}

void mitk::DiffusionImagingObjectFactory::RegisterIOFactories()
{
}

void RegisterDiffusionImagingObjectFactory()
{
  static bool oneDiffusionImagingObjectFactoryRegistered = false;
  if ( ! oneDiffusionImagingObjectFactoryRegistered ) {
    MITK_DEBUG << "Registering DiffusionImagingObjectFactory..." << std::endl;
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(mitk::DiffusionImagingObjectFactory::New());

    oneDiffusionImagingObjectFactoryRegistered = true;
  }
}
