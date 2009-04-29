/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-09-22 11:55:20 +0200 (Sa, 22 Sep 2007) $
Version:   $Revision: 12240 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/


#include "mitkVtkPropRenderer.h"

#include "picimage.h"

// MAPPERS
#include "mitkMapper.h"
#include "mitkImageMapper2D.h"
#include "mitkBaseVtkMapper2D.h"
#include "mitkBaseVtkMapper3D.h"
#include "mitkGeometry2DDataVtkMapper3D.h"
#include "mitkPointSetMapper2D.h"

#include "mitkImageSliceSelector.h"
#include "mitkRenderingManager.h"
#include "mitkGL.h"
#include "mitkGeometry3D.h"
#include "mitkDisplayGeometry.h"
#include "mitkLevelWindow.h"
#include "mitkCameraController.h"
#include "mitkVtkInteractorCameraController.h"
#include "mitkPlaneGeometry.h"
#include "mitkProperties.h"
#include "mitkSurface.h"
#include "mitkNodePredicateDataType.h"

// VTK
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkLight.h>
#include <vtkLightKit.h>
#include <vtkRenderWindow.h>
#include <vtkLinearTransform.h>
#include <vtkCamera.h>
#include <vtkWorldPointPicker.h>
#include <vtkPointPicker.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkProp.h>
#include <vtkAssemblyPath.h>
#include <vtkAssemblyNode.h>




mitk::VtkPropRenderer::VtkPropRenderer( const char* name, vtkRenderWindow * renWin )
  : BaseRenderer(name,renWin), 
  m_VtkMapperPresent(false), 
  m_NewRenderer(true)
{
  m_WorldPointPicker = vtkWorldPointPicker::New();
  m_PointPicker = vtkPointPicker::New();

  mitk::Geometry2DDataVtkMapper3D::Pointer geometryMapper = mitk::Geometry2DDataVtkMapper3D::New();
  m_CurrentWorldGeometry2DMapper = geometryMapper;
  m_CurrentWorldGeometry2DNode->SetMapper(2, geometryMapper);

  m_LightKit = vtkLightKit::New();
  m_LightKit->AddLightsToRenderer(m_VtkRenderer);
  m_PickingMode = WorldPointPicking;

  m_TextRenderer = vtkRenderer::New();
  m_TextRenderer->SetRenderWindow(renWin);
  m_TextRenderer->SetInteractive(0);
  #if ( VTK_MAJOR_VERSION >= 5 )
    m_TextRenderer->SetErase(0);
  #endif
}

/*!
\brief Destructs the VtkPropRenderer.
*/
mitk::VtkPropRenderer::~VtkPropRenderer()
{
  if(m_LightKit != NULL)
    m_LightKit->Delete();

  if(m_VtkRenderer!=NULL)
  {
    m_CameraController = NULL;

    m_VtkRenderer->Delete();

    m_VtkRenderer = NULL;
  }
  else
    m_CameraController = NULL;

  if(m_WorldPointPicker != NULL)
    m_WorldPointPicker->Delete();
  if(m_PointPicker != NULL)
    m_PointPicker->Delete();
  if(m_TextRenderer != NULL)
    m_TextRenderer->Delete();
}

void mitk::VtkPropRenderer::SetData(const mitk::DataTreeIteratorBase* iterator)
{
  if(iterator!=GetData())
  {
    BaseRenderer::SetData(iterator);
    static_cast<mitk::Geometry2DDataVtkMapper3D*>(m_CurrentWorldGeometry2DMapper.GetPointer())->SetDataIteratorForTexture(m_DataTreeIterator.GetPointer());

    // Compute the geometry from the current data tree bounds and set it as world geometry
    this->SetWorldGeometryToDataTreeBounds();
  }
}

void mitk::VtkPropRenderer::SetData( mitk::DataStorage::Pointer it )
{
  if( it.IsNotNull() )
  {
    BaseRenderer::SetData(it);

    static_cast<mitk::Geometry2DDataVtkMapper3D*>(m_CurrentWorldGeometry2DMapper.GetPointer())
       ->SetDataIteratorForTexture( m_DataStorage.GetPointer() );

    // Compute the geometry from the current data tree bounds and set it as world geometry
    
    this->SetWorldGeometryToDataTreeBounds();
  }
}

bool mitk::VtkPropRenderer::SetWorldGeometryToDataTreeBounds()
{
  if ( m_DataStorage.IsNotNull() )
  {
    //initialize world geometry
    mitk::Geometry3D::Pointer geometry = m_DataStorage->ComputeVisibleBoundingGeometry3D( NULL, "includeInBoundingBox" );

    if ( geometry.IsNotNull() )
    {
      this->SetWorldGeometry(geometry);
      this->GetDisplayGeometry()->Fit();
      this->GetVtkRenderer()->ResetCamera();
      this->Modified();
      return true;
    }
    else
    {
      return false;
    }
  }

  else if ( this->GetData() != NULL )
  {
    //initialize world geometry
    mitk::Geometry3D::Pointer geometry = 
      mitk::DataTree::ComputeVisibleBoundingGeometry3D(
        const_cast<mitk::DataTreeIteratorBase*>( this->GetData() ), 
        NULL, "includeInBoundingBox" );

    if ( geometry.IsNotNull() )
    {
      this->SetWorldGeometry(geometry);
      this->GetDisplayGeometry()->Fit();
      this->GetVtkRenderer()->ResetCamera();
      this->Modified();
      return true;
    }
    else
    {
      return false;
    }
  }
  return false;
}



/*!
\brief 

Called by the vtkMitkRenderProp in order to start MITK rendering process.
*/
int mitk::VtkPropRenderer::Render(mitk::VtkPropRenderer::RenderType type)
{
   // Do we have objects to render?
  if ( this->GetEmptyWorldGeometry()) 
    return 0;

  if ( m_DataStorage.IsNull() && this->GetData() == NULL )
    return 0;
     
  if( this->GetData() != NULL )
  {
    if ( dynamic_cast<mitk::DataTree*>(GetData()->GetTree()) == NULL ) 
      return 0;
  }

  // Update mappers and prepare mapper queue
  if(type == VtkPropRenderer::Opaque)
    this->PrepareMapperQueue();
  
  //go through the generated list and let the sorted mappers paint
  bool lastVtkBased = true;
  bool sthVtkBased = false;
  
  for(MappersMapType::iterator it = m_MappersMap.begin(); it != m_MappersMap.end(); it++)
  {
    Mapper * mapper = (*it).second;
    if((mapper->IsVtkBased() == true) )
    {
      sthVtkBased = true;
      mitk::BaseVtkMapper3D::Pointer vtkMapper = dynamic_cast<mitk::BaseVtkMapper3D*>(mapper);
      if(vtkMapper)
      {
         vtkMapper->GetProp()->SetAllocatedRenderTime(5000,GetVtkRenderer()); //B/ ToDo: rendering time calculation
         //vtkMapper->GetProp()->PokeMatrix(NULL); //B/ ToDo ??: VtkUserTransform
      }
      if(lastVtkBased == false)
      {
        Disable2DOpenGL();
        lastVtkBased = true;
      }
    }
    else
    if((mapper->IsVtkBased() == false) && (lastVtkBased == true))
    {
      Enable2DOpenGL();
      lastVtkBased = false;
    }
    
    switch(type)
    {
      case mitk::VtkPropRenderer::Opaque:        mapper->MitkRenderOpaqueGeometry(this); break;
      case mitk::VtkPropRenderer::Translucent:   mapper->MitkRenderTranslucentGeometry(this); break;
      case mitk::VtkPropRenderer::Overlay:       mapper->MitkRenderOverlay(this); break;
        //BUG (#1551) changed VTK_MINOR_VERSION FROM 3 to 2 cause RenderTranslucentGeometry was changed in minor version 2
      #if ( ( VTK_MAJOR_VERSION >= 5 ) && ( VTK_MINOR_VERSION>=2)  )
      case mitk::VtkPropRenderer::Volumetric:    mapper->MitkRenderVolumetricGeometry(this); break;
      #endif
    }
  }
  
  if(lastVtkBased==false)
    Disable2DOpenGL();
  
  //fix for bug 1177. In 2D rendering the camera is not needed, but nevertheless it is used by 
  //the vtk rendering mechanism to determine what is seen (and therefore has to be rendered)
  //by using the bounds of the vtkMitkRenderProp
  if(sthVtkBased == false)
    this->GetVtkRenderer()->ResetCamera();

  // Render text
  if(type == VtkPropRenderer::Overlay)
  {
    if(m_TextCollection.size() > 0)
    {
      for (TextMapType::iterator it = m_TextCollection.begin(); it != m_TextCollection.end() ; it++)
        m_TextRenderer->AddViewProp((*it).second);
      m_TextRenderer->Render();
    }
  }
  return 1;
}

/*!
\brief PrepareMapperQueue iterates the datatree

PrepareMapperQueue iterates the datatree in order to find mappers which shall be rendered. Also, it sortes the mappers wrt to their layer.
*/
void mitk::VtkPropRenderer::PrepareMapperQueue()
{
  // variable for counting LOD-enabled mappers
  m_NumberOfVisibleLODEnabledMappers = 0;

  // Do we have to update the mappers ?
  if ( m_LastUpdateTime < GetMTime() || m_LastUpdateTime < GetDisplayGeometry()->GetMTime() )
    Update();
  else if(m_MapperID>=2 && m_MapperID < 6)
    Update();

  // remove all text properties before mappers will add new ones
  m_TextRenderer->RemoveAllViewProps();
  m_TextCollection.clear();
  
  // clear priority_queue
  m_MappersMap.clear();
 
  int mapperNo = 0;

  //DataStorage
  if( m_DataStorage.IsNotNull() )
  {
    DataStorage::SetOfObjects::ConstPointer allObjects = m_DataStorage->GetAll();
    
    for(DataStorage::SetOfObjects::const_iterator iter = allObjects->begin();  iter != allObjects->end(); ++iter)
    {
      DataTreeNode::Pointer node = *iter;

      if ( node.IsNull() )
        continue;

      mitk::Mapper::Pointer mapper = node->GetMapper(m_MapperID);
      if ( mapper.IsNull() )
        continue;

      // The information about LOD-enabled mappers is required by RenderingManager
      if ( mapper->IsLODEnabled( this ) && mapper->IsVisible( this ) )
      {
        ++m_NumberOfVisibleLODEnabledMappers;
      }

      // mapper without a layer property get layer number 1
      int layer = 1;
      node->GetIntProperty("layer", layer, this);

      int nr = (layer<<16) + mapperNo;
      m_MappersMap.insert( std::pair< int, Mapper * >( nr, mapper ) );
      mapperNo++;
    }

  }

  //DataTree Iterator
  else if( m_DataTreeIterator.IsNotNull() )
  {
    mitk::DataTreeIteratorClone it = m_DataTreeIterator;
    for ( it->GoToBegin(); it->IsAtEnd() == false; ++it )
    {
      mitk::DataTreeNode::Pointer node = it->Get();

      if ( node.IsNull() )
        continue;

      mitk::Mapper::Pointer mapper = node->GetMapper(m_MapperID);
      if ( mapper.IsNull() )
        continue;

      // The information about LOD-enabled mappers is required by RenderingManager
      if ( mapper->IsLODEnabled( this ) && mapper->IsVisible( this ) )
      {
        ++m_NumberOfVisibleLODEnabledMappers;
      }

      // mapper without a layer property get layer number 1
      int layer = 1;
      node->GetIntProperty("layer", layer, this);

      int nr = (layer<<16) + mapperNo;
      m_MappersMap.insert( std::pair< int, Mapper * >( nr, mapper ) );
      mapperNo++;
    }
  }  
}

/*!
\brief 

Enable2DOpenGL() and Disable2DOpenGL() are used to switch between 2D rendering (orthographic projection) and 3D rendering (perspective projection)
*/
void mitk::VtkPropRenderer::Enable2DOpenGL()
{
  GLint iViewport[4];  
   
  // Get a copy of the viewport  
  glGetIntegerv( GL_VIEWPORT, iViewport );  
   
  // Save a copy of the projection matrix so that we can restore it  
  // when it's time to do 3D rendering again.  
  glMatrixMode( GL_PROJECTION );  
  glPushMatrix();  
  glLoadIdentity();  
   
  // Set up the orthographic projection  
  glOrtho( 
    iViewport[0], iViewport[0]+iViewport[2],  
    iViewport[1], iViewport[1]+iViewport[3],
    -1.0, 1.0 
  );
  glMatrixMode( GL_MODELVIEW );  
  glPushMatrix();  
  glLoadIdentity();  
   
  // Make sure depth testing and lighting are disabled for 2D rendering until  
  // we are finished rendering in 2D  
  glPushAttrib( GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT );  
  glDisable( GL_DEPTH_TEST );  
  glDisable( GL_LIGHTING );  
}

/*!
\brief Initialize the VtkPropRenderer

Enable2DOpenGL() and Disable2DOpenGL() are used to switch between 2D rendering (orthographic projection) and 3D rendering (perspective projection)
*/
void mitk::VtkPropRenderer::Disable2DOpenGL()
{
  glPopAttrib();  
  glMatrixMode( GL_PROJECTION );  
  glPopMatrix();  
  glMatrixMode( GL_MODELVIEW );  
  glPopMatrix(); 
}

void mitk::VtkPropRenderer::Update(mitk::DataTreeNode* datatreenode)
{
  if(datatreenode!=NULL)
  {
    mitk::Mapper::Pointer mapper = datatreenode->GetMapper(m_MapperID);
    if(mapper.IsNotNull())
    {
      Mapper2D* mapper2d=dynamic_cast<Mapper2D*>(mapper.GetPointer());
      if(mapper2d != NULL)
      {
        if(GetDisplayGeometry()->IsValid())
        {
          BaseVtkMapper2D* vtkmapper2d=dynamic_cast<BaseVtkMapper2D*>(mapper.GetPointer());
          if(vtkmapper2d != NULL)
          {
            vtkmapper2d->Update(this);
            m_VtkMapperPresent=true;
          }
          else
            mapper2d->Update(this);
        }
      }
      else
      {
        BaseVtkMapper3D* vtkmapper3d=dynamic_cast<BaseVtkMapper3D*>(mapper.GetPointer());
        if(vtkmapper3d != NULL)
        {
          vtkmapper3d->Update(this);
          vtkmapper3d->UpdateVtkTransform();
          m_VtkMapperPresent=true;
        }
      }
    }
  }
}


void mitk::VtkPropRenderer::Update()
{
  if( m_DataStorage.IsNotNull() )
  {
    m_VtkMapperPresent = false;

    mitk::DataStorage::SetOfObjects::ConstPointer all = m_DataStorage->GetAll();
    for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
      Update(it->Value());

    Modified();
    m_LastUpdateTime = GetMTime();
  }
  else if( m_DataTreeIterator.IsNotNull() )
  {
    m_VtkMapperPresent = false;

    mitk::DataTreeIteratorClone it = m_DataTreeIterator;
    it->GoToBegin();

    while(!it->IsAtEnd())
    {
      Update(it->Get());
      ++it;
    }
    Modified();
    m_LastUpdateTime = GetMTime();
  }  
}

/*!
\brief

This method is called from the two Constructors
*/
void mitk::VtkPropRenderer::InitRenderer(vtkRenderWindow* renderWindow)
{
  BaseRenderer::InitRenderer(renderWindow);

  if(renderWindow == NULL)
  {
    m_InitNeeded = false;
    m_ResizeNeeded = false;
    return;
  }

  m_InitNeeded = true;
  m_ResizeNeeded = true;

  m_LastUpdateTime = 0;

}

/*!
\brief Resize the OpenGL Window
*/
void mitk::VtkPropRenderer::Resize(int w, int h)
{
  glViewport (0, 0, w, h);
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glOrtho( 0.0, w, 0.0, h , 0.0 , 1.0);
  glMatrixMode( GL_MODELVIEW );

  BaseRenderer::Resize(w, h);
   
  Update();
}

void mitk::VtkPropRenderer::InitSize(int w, int h)
{
  m_RenderWindow->SetSize(w,h);
  Superclass::InitSize(w, h);
  Modified();
  Update();
  if(m_VtkRenderer!=NULL)
  {
    int w=vtkObject::GetGlobalWarningDisplay();
    vtkObject::GlobalWarningDisplayOff();
    m_VtkRenderer->ResetCamera();
    vtkObject::SetGlobalWarningDisplay(w);
  }
}

void mitk::VtkPropRenderer::SetMapperID(const MapperSlotId mapperId)
{
  if(m_MapperID != mapperId)
    Superclass::SetMapperID(mapperId);
}

/*!
\brief Activates the current renderwindow.
*/
void mitk::VtkPropRenderer::MakeCurrent()
{
  if(m_RenderWindow!=NULL)
     m_RenderWindow->MakeCurrent();
}

void mitk::VtkPropRenderer::PickWorldPoint(const mitk::Point2D& displayPoint, mitk::Point3D& worldPoint) const
{
  if(m_VtkMapperPresent)
  {
    //m_WorldPointPicker->SetTolerance (0.0001);
    switch ( m_PickingMode )
    {
     case (WorldPointPicking) :
     {
       m_WorldPointPicker->Pick(displayPoint[0], displayPoint[1], 0, m_VtkRenderer);
       vtk2itk(m_WorldPointPicker->GetPickPosition(), worldPoint);
       break;
     }
     case (PointPicking) :
     {
       // create a new vtkRenderer
       // give it all neccessary information (camera position, etc.)
       // get all surfaces from datastorage, get actors from them
       // add all those actors to the new renderer
       // give this new renderer to pointpicker
       /*
       vtkRenderer* pickingRenderer = vtkRenderer::New();
       pickingRenderer->SetActiveCamera( );
      
       DataStorage* dataStorage = DataStorage::GetInstance();
       TNodePredicateDataType<Surface> isSurface;

       DataStorage::SetOfObjects::ConstPointer allSurfaces = dataStorage->GetSubset( isSurface );
       std::cout << "in picking: got " << allSurfaces->size() << " surfaces." << std::endl;

       for (DataStorage::SetOfObjects::const_iterator iter = allSurfaces->begin();
            iter != allSurfaces->end();
            ++iter)
       {
         const DataTreeNode* currentNode = *iter;
         BaseVtkMapper3D* baseVtkMapper3D = dynamic_cast<BaseVtkMapper3D*>( currentNode->GetMapper( BaseRenderer::Standard3D ) );
         if ( baseVtkMapper3D )
         {
           vtkActor* actor = dynamic_cast<vtkActor*>( baseVtkMapper3D->GetViewProp() );
           if (actor)
           {
             std::cout << "a" << std::flush;
             pickingRenderer->AddActor( actor );
           }
         }
       }

       std::cout << ";" << std::endl;
       */
       m_PointPicker->Pick(displayPoint[0], displayPoint[1], 0, m_VtkRenderer);
       vtk2itk(m_PointPicker->GetPickPosition(), worldPoint);
       break;
     }
    }
  }
  else
  {
    Superclass::PickWorldPoint(displayPoint, worldPoint);
  }
}

/*!
\brief Writes some 2D text as overlay. Function returns an unique int Text_ID for each call, which can be used via the GetTextLabelProperty(int text_id) function
in order to get a vtkTextProperty. This property enables the setup of font, font size, etc.
*/
int mitk::VtkPropRenderer::WriteSimpleText(std::string text, double posX, double posY, double color1, double color2, double color3)
{
 if(text.size() > 0)
  {
    vtkTextActor* textActor = vtkTextActor::New();

    textActor->SetPosition(posX,posY);
    textActor->SetInput(text.c_str());
    textActor->GetTextProperty()->SetColor(color1, color2, color3); //TODO: Read color from node property
    int text_id = m_TextCollection.size();
    m_TextCollection.insert(TextMapType::value_type(text_id,textActor));
    return text_id;
  }
  return -1;
}

/*!
\brief Can be used in order to get a vtkTextProperty for a specific text_id. This property enables the setup of font, font size, etc.
*/
vtkTextProperty* mitk::VtkPropRenderer::GetTextLabelProperty(int text_id)
{
  return this->m_TextCollection[text_id]->GetTextProperty();
}

void mitk::VtkPropRenderer::InitPathTraversal()
{
  if (m_DataStorage.IsNotNull())
  {
    m_PickingObjects = m_DataStorage->GetAll();
    m_PickingObjectsIterator = m_PickingObjects->begin();
  }
}

vtkAssemblyPath* mitk::VtkPropRenderer::GetNextPath()
{
  if ( m_PickingObjectsIterator == m_PickingObjects->end() )
  {
    return NULL;
  }

  vtkAssemblyPath* returnPath = vtkAssemblyPath::New();
  //returnPath->Register(NULL);

  bool success = false;
 
  while (!success)
  {
    // loop until AddNode can be called successfully
    const DataTreeNode* node = *m_PickingObjectsIterator;
    if (node)
    {
      Mapper* mapper = node->GetMapper( BaseRenderer::Standard3D );
      if (mapper)
      {
        BaseVtkMapper3D* vtkmapper = dynamic_cast<BaseVtkMapper3D*>( mapper );
        if (vtkmapper)
        {
          vtkProp* prop = vtkmapper->GetProp();
          if ( prop && prop->GetVisibility() )
          {
            // add to assembly path
            returnPath->AddNode( prop, prop->GetMatrix() );
            success = true;
          }
        }
      }
    }

    ++m_PickingObjectsIterator;

    if ( m_PickingObjectsIterator == m_PickingObjects->end() ) break;
  }

  if ( success )
  {
    return returnPath;
  }
  else
  {
    return NULL;
  }
}

void mitk::VtkPropRenderer::ReleaseGraphicsResources(vtkWindow *renWin)
{
  DataStorage::Pointer storage = m_DataStorage;

  DataStorage::SetOfObjects::ConstPointer allObjects = storage->GetAll();
  for (DataStorage::SetOfObjects::const_iterator iter = allObjects->begin();
      iter != allObjects->end();
      ++iter)
  {
    DataTreeNode::Pointer node = *iter;
    if ( node.IsNull() )
      continue;

    Mapper::Pointer mapper = node->GetMapper(m_MapperID);
    if(mapper.IsNotNull())
    {
      mapper->ReleaseGraphicsResources(renWin);
    }
  }
}
  
vtkWorldPointPicker* mitk::VtkPropRenderer::GetWorldPointPicker()
{
  return m_WorldPointPicker;
}

vtkPointPicker* mitk::VtkPropRenderer::GetPointPicker()
{
  return m_PointPicker;
}
#if ( ( VTK_MAJOR_VERSION >= 5 ) && ( VTK_MINOR_VERSION>=2)  )
mitk::VtkPropRenderer::MappersMapType mitk::VtkPropRenderer::GetMappersMap() const
{
  return m_MappersMap;
}
#endif
