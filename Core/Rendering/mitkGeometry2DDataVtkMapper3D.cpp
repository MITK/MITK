/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkGeometry2DDataVtkMapper3D.h"

#include "mitkPlaneGeometry.h"

#include "mitkDataTree.h"
#include "mitkImageMapper2D.h"

#include "mitkSurface.h"
#include "mitkGeometry2DDataToSurfaceFilter.h"

#include "vtkActor.h"
#include "vtkProperty.h"
#include "vtkTexture.h"
#include "vtkPlaneSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkLookupTable.h"
//#include "vtkImageMapToWindowLevelColors";
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkLookupTableProperty.h"


#include "mitkLevelWindowProperty.h"
#include "mitkSmartPointerProperty.h"
#include "mitkWeakPointerProperty.h"
#include <vtkActor.h>
#include <vtkImageData.h>
#include <vtkLinearTransform.h>

#include "pic2vtk.h"

//##ModelId=3E691E09038E
mitk::Geometry2DDataVtkMapper3D::Geometry2DDataVtkMapper3D() : m_DataTreeIterator(NULL), m_LastTextureUpdateTime(0)
{
  m_VtkPlaneSource = vtkPlaneSource::New();

  m_VtkPolyDataMapper = vtkPolyDataMapper::New();
  m_VtkPolyDataMapper->ImmediateModeRenderingOn();

  m_Actor = vtkActor::New();
  m_Actor->SetMapper(m_VtkPolyDataMapper);
  m_Actor->GetProperty()->SetAmbient(0.5);

  m_Prop3D = m_Actor;
  m_Prop3D->Register(NULL);

  m_VtkLookupTable = vtkLookupTable::New();
  m_VtkLookupTable->SetTableRange (-1024, 4096);
  m_VtkLookupTable->SetSaturationRange (0, 0);
  m_VtkLookupTable->SetHueRange (0, 0);
  m_VtkLookupTable->SetValueRange (0, 1);
  m_VtkLookupTable->Build ();

  m_VtkLookupTableDefault = m_VtkLookupTable;

  m_VtkTexture = vtkTexture::New();
  m_VtkTexture->InterpolateOn();
  m_VtkTexture->SetLookupTable(m_VtkLookupTable);
  m_VtkTexture->MapColorScalarsThroughLookupTableOn();
}

//##ModelId=3E691E090394
mitk::Geometry2DDataVtkMapper3D::~Geometry2DDataVtkMapper3D()
{
  m_VtkPlaneSource->Delete();
  m_VtkPolyDataMapper->Delete();
  m_Actor->Delete();
  m_VtkLookupTable->Delete();
  m_VtkTexture->Delete();
}



//##ModelId=3E691E090380
const mitk::Geometry2DData *mitk::Geometry2DDataVtkMapper3D::GetInput()
{
  return static_cast<const mitk::Geometry2DData * > ( GetData() );
}

//##ModelId=3E6E874F0007
void mitk::Geometry2DDataVtkMapper3D::SetDataIteratorForTexture(const mitk::DataTreeIteratorBase* iterator)
{
  if(m_DataTreeIterator != iterator)
  {
    m_DataTreeIterator = iterator;
    Modified();
  }
}

//##ModelId=3EF19F850151
void mitk::Geometry2DDataVtkMapper3D::GenerateData(mitk::BaseRenderer* renderer)
{
  if(IsVisible(renderer)==false)
  {
    m_Actor->VisibilityOff();
    return;
  }
  m_Actor->VisibilityOn();

  mitk::Geometry2DData::Pointer input  = const_cast<mitk::Geometry2DData*>(this->GetInput());

  if(input.IsNotNull() && (input->GetGeometry2D() != NULL))
  {
    mitk::Geometry2DDataToSurfaceFilter::Pointer surfaceCreator;
    mitk::SmartPointerProperty::Pointer surfacecreatorprop;
    surfacecreatorprop=dynamic_cast<mitk::SmartPointerProperty*>(GetDataTreeNode()->GetProperty("surfacegeometry", renderer).GetPointer());
    if( (surfacecreatorprop.IsNull()) || 
      (surfacecreatorprop->GetSmartPointer().IsNull()) ||
      ((surfaceCreator=dynamic_cast<mitk::Geometry2DDataToSurfaceFilter*>(surfacecreatorprop->GetSmartPointer().GetPointer())).IsNull())
      )
    {
      surfaceCreator = mitk::Geometry2DDataToSurfaceFilter::New();
      surfaceCreator->PlaceByGeometryOn();
      surfacecreatorprop=new mitk::SmartPointerProperty(surfaceCreator);
      GetDataTreeNode()->SetProperty("surfacegeometry", surfacecreatorprop);
    }

    surfaceCreator->SetInput(input);

    int res;
    if(GetDataTreeNode()->GetIntProperty("xresolution", res, renderer))
      surfaceCreator->SetXResolution(res);
    if(GetDataTreeNode()->GetIntProperty("yresolution", res, renderer))
      surfaceCreator->SetYResolution(res);

    surfaceCreator->Update(); //@FIXME ohne das crash
    m_VtkPolyDataMapper->SetInput(surfaceCreator->GetOutput()->GetVtkPolyData());

    bool texture=false;
    if(m_DataTreeIterator.IsNotNull())
    {
      mitk::DataTreeIteratorClone it=m_DataTreeIterator.GetPointer();
      while(!it->IsAtEnd())
      {
        mitk::DataTreeNode* node=it->Get();
        mitk::Mapper::Pointer mapper = node->GetMapper(1);
        mitk::ImageMapper2D* imagemapper = dynamic_cast<ImageMapper2D*>(mapper.GetPointer());

        if((node->IsVisible(renderer)) && (imagemapper))
        {
          mitk::WeakPointerProperty::Pointer rendererProp = dynamic_cast<mitk::WeakPointerProperty*>(GetDataTreeNode()->GetPropertyList()->GetProperty("renderer").GetPointer());
          if(rendererProp.IsNotNull())
          {
            mitk::BaseRenderer::Pointer renderer = dynamic_cast<mitk::BaseRenderer*>(rendererProp->GetWeakPointer().GetPointer());
            if(renderer.IsNotNull())
            {

              // check for LookupTable
              mitk::LookupTableProperty::Pointer LookupTableProb;
              LookupTableProb = dynamic_cast<mitk::LookupTableProperty*>(node->GetPropertyList()->GetProperty("LookupTable").GetPointer());
              if (LookupTableProb.IsNotNull() )
              {
                m_VtkLookupTable = LookupTableProb->GetLookupTable().GetVtkLookupTable();
                m_VtkTexture->SetLookupTable(m_VtkLookupTable);
                //						    m_VtkTexture->Modified();
              } else {
                m_VtkLookupTable = m_VtkLookupTableDefault;
              }


              // check for level window prop and use it for display if it exists
              mitk::LevelWindow levelWindow;
              if(node->GetLevelWindow(levelWindow, renderer))
                m_VtkLookupTable->SetTableRange(levelWindow.GetMin(),levelWindow.GetMax());


              //we have to do this before GenerateAllData() is called there may be
              //no RendererInfo for renderer yet, thus GenerateAllData won't update
              //the (non-existing) RendererInfo for renderer. By calling GetRendererInfo
              //a RendererInfo will be created for renderer (if it does not exist yet).
              const ImageMapper2D::RendererInfo* ri=imagemapper->GetRendererInfo(renderer);
              imagemapper->GenerateAllData();
              texture = true;
              if((ri!=NULL) && (m_LastTextureUpdateTime<ri->m_LastUpdateTime))
              {
                ipPicDescriptor *p=ri->m_Pic;
                if((p->dim==2) && (p->n[0]>2) && (p->n[1]>2))
                {
                  vtkImageData* vtkimage=Pic2vtk::convert(p);
                  m_VtkTexture->SetInput(vtkimage);
                  vtkimage->Delete(); vtkimage=NULL;
                  m_Actor->SetTexture(m_VtkTexture);
                  m_LastTextureUpdateTime=ri->m_LastUpdateTime;
                  bool textureInterpolation=true;
                  m_VtkTexture->SetInterpolate(textureInterpolation ? 1 : 0);
                }
                else
                  texture = false;
              }
              break;
            }
          }
        }
        ++it;
      }
    }
    if(texture==false)
    {
      m_Actor->SetTexture(NULL);
    }

    //apply properties read from the PropertyList
    ApplyProperties(m_Actor, renderer);
    m_Prop3D->SetUserTransform(GetDataTreeNode()->GetVtkTransform());
  }
}
