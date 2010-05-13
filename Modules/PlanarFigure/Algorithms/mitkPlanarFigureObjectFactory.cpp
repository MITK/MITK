/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-04-16 16:53:19 +0200 (Fr, 16 Apr 2010) $
Version:   $Revision: 16916 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkPlanarFigureObjectFactory.h"

#include "mitkCoreObjectFactory.h"

#include "mitkPlanarFigureIOFactory.h"
#include "mitkPlanarFigureWriterFactory.h"
#include "mitkPlanarFigure.h"
#include "mitkPlanarFigureMapper2D.h"


mitk::PlanarFigureObjectFactory::PlanarFigureObjectFactory() 
{
  static bool alreadyDone = false;
  if ( !alreadyDone )
  {
    MITK_INFO << "PlanarFigureObjectFactory c'tor" << std::endl;
    RegisterIOFactories();

    itk::ObjectFactoryBase::RegisterFactory( PlanarFigureIOFactory::New() );

    PlanarFigureWriterFactory::RegisterOneFactory();

    alreadyDone = true;
  }
}

mitk::Mapper::Pointer mitk::PlanarFigureObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId id) 
{
  mitk::Mapper::Pointer newMapper=NULL;
  mitk::BaseData *data = node->GetData();

  if ( id == mitk::BaseRenderer::Standard2D )
  {
    if ( dynamic_cast<PlanarFigure*>(data) != NULL )
    {
      newMapper = mitk::PlanarFigureMapper2D::New();
      newMapper->SetDataNode(node);
    }
  }
  else if ( id == mitk::BaseRenderer::Standard3D )
  {
  }

  return newMapper;
}

void mitk::PlanarFigureObjectFactory::SetDefaultProperties(mitk::DataNode* node)
{
  
  if ( node == NULL )
  {
    return;
  }

  mitk::DataNode::Pointer nodePointer = node;

  mitk::PlanarFigure::Pointer pf = dynamic_cast<mitk::PlanarFigure*>( node->GetData() );
  if ( pf.IsNotNull() )
  {
    mitk::PlanarFigureMapper2D::SetDefaultProperties(node);
  } 
}

const char* mitk::PlanarFigureObjectFactory::GetFileExtensions() 
{
  return "";
};

const char* mitk::PlanarFigureObjectFactory::GetSaveFileExtensions() 
{ 
  return ";;Planar Figures (*.pf)";  // for mitk::PlanarFigure and derived classes
};

void mitk::PlanarFigureObjectFactory::RegisterIOFactories() 
{
}

void RegisterPlanarFigureObjectFactory() 
{
  static bool oneObjectFactoryRegistered = false;
  if ( !oneObjectFactoryRegistered ) 
  {
    MITK_INFO << "Registering PlanarFigureObjectFactory..." << std::endl;
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory( mitk::PlanarFigureObjectFactory::New() );
    oneObjectFactoryRegistered = true;
  }
}
