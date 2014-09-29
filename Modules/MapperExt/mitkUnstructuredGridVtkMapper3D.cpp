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


#include "mitkUnstructuredGridVtkMapper3D.h"
#include "mitkDataNode.h"
#include "mitkProperties.h"
#include "mitkTransferFunctionProperty.h"
#include "mitkColorProperty.h"
//#include "mitkLookupTableProperty.h"
#include "mitkGridRepresentationProperty.h"
#include "mitkGridVolumeMapperProperty.h"
#include "mitkVtkInterpolationProperty.h"
#include "mitkVtkScalarModeProperty.h"

#include "mitkDataStorage.h"

#include "mitkSurfaceVtkMapper3D.h"

#include <vtkUnstructuredGrid.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkProperty.h>

#include <vtkPlanes.h>



const mitk::UnstructuredGrid* mitk::UnstructuredGridVtkMapper3D::GetInput()
{
  return static_cast<const mitk::UnstructuredGrid * > ( GetDataNode()->GetData() );
}


mitk::UnstructuredGridVtkMapper3D::UnstructuredGridVtkMapper3D()
{

  m_VtkTriangleFilter = vtkDataSetTriangleFilter::New();

  m_Assembly = vtkAssembly::New();

  m_Volume = vtkVolume::New();
  m_Actor = vtkActor::New();
  m_ActorWireframe = vtkActor::New();

  m_VtkDataSetMapper = vtkUnstructuredGridMapper::New();
  m_VtkDataSetMapper->SetResolveCoincidentTopologyToPolygonOffset();
  m_VtkDataSetMapper->SetResolveCoincidentTopologyPolygonOffsetParameters(0,1);
  m_Actor->SetMapper(m_VtkDataSetMapper);

  m_VtkDataSetMapper2 = vtkUnstructuredGridMapper::New();
  m_VtkDataSetMapper2->SetResolveCoincidentTopologyToPolygonOffset();
  m_VtkDataSetMapper2->SetResolveCoincidentTopologyPolygonOffsetParameters(1,1);
  m_ActorWireframe->SetMapper(m_VtkDataSetMapper2);
  m_ActorWireframe->GetProperty()->SetRepresentationToWireframe();

  m_Assembly->AddPart(m_Actor);
  m_Assembly->AddPart(m_ActorWireframe);
  m_Assembly->AddPart(m_Volume);

  m_VtkVolumeRayCastMapper = 0;
  m_VtkPTMapper = 0;
  m_VtkVolumeZSweepMapper = 0;

  //m_GenerateNormals = false;
}


mitk::UnstructuredGridVtkMapper3D::~UnstructuredGridVtkMapper3D()
{

  if (m_VtkTriangleFilter != 0)
    m_VtkTriangleFilter->Delete();

  if (m_VtkVolumeRayCastMapper != 0)
    m_VtkVolumeRayCastMapper->Delete();

  if (m_VtkVolumeZSweepMapper != 0)
    m_VtkVolumeZSweepMapper->Delete();

  if (m_VtkPTMapper != 0)
    m_VtkPTMapper->Delete();

  if (m_VtkDataSetMapper != 0)
    m_VtkDataSetMapper->Delete();

  if (m_VtkDataSetMapper2 != 0)
      m_VtkDataSetMapper2->Delete();

  if (m_Assembly != 0)
    m_Assembly->Delete();

  if (m_Actor != 0)
    m_Actor->Delete();

  if (m_ActorWireframe != 0)
      m_ActorWireframe->Delete();

  if (m_Volume != 0)
    m_Volume->Delete();

}

vtkProp* mitk::UnstructuredGridVtkMapper3D::GetVtkProp(mitk::BaseRenderer*  /*renderer*/)
{
  return m_Assembly;
}


void mitk::UnstructuredGridVtkMapper3D::GenerateDataForRenderer(mitk::BaseRenderer* renderer)
{

  mitk::DataNode::ConstPointer node = this->GetDataNode();

  BaseLocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  bool needGenerateData = ls->IsGenerateDataRequired( renderer, this, GetDataNode() );

  if(needGenerateData)
  {
    ls->UpdateGenerateDataTime();

    m_Assembly->VisibilityOn();

    m_ActorWireframe->GetProperty()->SetAmbient(1.0);
    m_ActorWireframe->GetProperty()->SetDiffuse(0.0);
    m_ActorWireframe->GetProperty()->SetSpecular(0.0);

    mitk::TransferFunctionProperty::Pointer transferFuncProp;
    if (node->GetProperty(transferFuncProp, "TransferFunction"))
    {
      mitk::TransferFunction::Pointer transferFunction = transferFuncProp->GetValue();
      if (transferFunction->GetColorTransferFunction()->GetSize() < 2)
      {
        mitk::UnstructuredGrid::Pointer input  = const_cast< mitk::UnstructuredGrid* >(this->GetInput());
        if (input.IsNull()) return;

        vtkUnstructuredGrid * grid = input->GetVtkUnstructuredGrid(this->GetTimestep());
        if (grid == 0) return;

        double* scalarRange = grid->GetScalarRange();
        vtkColorTransferFunction* colorFunc = transferFunction->GetColorTransferFunction();
        colorFunc->RemoveAllPoints();
        colorFunc->AddRGBPoint(scalarRange[0], 1, 0, 0);
        colorFunc->AddRGBPoint((scalarRange[0] + scalarRange[1])/2.0, 0, 1, 0);
        colorFunc->AddRGBPoint(scalarRange[1], 0, 0, 1);
      }
    }
  }

  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");

  if(!visible)
  {
    m_Assembly->VisibilityOff();
    return;
  }

  //
  // get the TimeGeometry of the input object
  //
  mitk::UnstructuredGrid::Pointer input  = const_cast< mitk::UnstructuredGrid* >( this->GetInput() );

  //
  // set the input-object at time t for the mapper
  //
  vtkUnstructuredGrid * grid = input->GetVtkUnstructuredGrid( this->GetTimestep() );
  if(grid == 0)
  {
    m_Assembly->VisibilityOff();
    return;
  }

  m_Assembly->VisibilityOn();

  m_VtkTriangleFilter->SetInputData(grid);
  m_VtkDataSetMapper->SetInput(grid);
  m_VtkDataSetMapper2->SetInput(grid);

  bool clip = false;
  node->GetBoolProperty("enable clipping", clip);
  mitk::DataNode::Pointer bbNode = renderer->GetDataStorage()->GetNamedDerivedNode("Clipping Bounding Object", node);
  if (clip && bbNode.IsNotNull())
  {
    m_VtkDataSetMapper->SetBoundingObject(dynamic_cast<mitk::BoundingObject*>(bbNode->GetData()));
    m_VtkDataSetMapper2->SetBoundingObject(dynamic_cast<mitk::BoundingObject*>(bbNode->GetData()));
  }
  else
  {
    m_VtkDataSetMapper->SetBoundingObject(0);
    m_VtkDataSetMapper2->SetBoundingObject(0);
  }

  //
  // apply properties read from the PropertyList
  //
  ApplyProperties(0, renderer);
}


void mitk::UnstructuredGridVtkMapper3D::ResetMapper( BaseRenderer* /*renderer*/ )
{
  m_Assembly->VisibilityOff();
}


void mitk::UnstructuredGridVtkMapper3D::ApplyProperties(vtkActor* /*actor*/, mitk::BaseRenderer* renderer)
{
  mitk::DataNode::Pointer node = this->GetDataNode();
  ApplyColorAndOpacityProperties(renderer, m_Actor);
  ApplyColorAndOpacityProperties(renderer, m_ActorWireframe);

  vtkVolumeProperty* volProp = m_Volume->GetProperty();
  vtkProperty* property = m_Actor->GetProperty();
  vtkProperty* wireframeProp = m_ActorWireframe->GetProperty();

  mitk::SurfaceVtkMapper3D::ApplyMitkPropertiesToVtkProperty(node,property,renderer);
  mitk::SurfaceVtkMapper3D::ApplyMitkPropertiesToVtkProperty(node,wireframeProp,renderer);

  mitk::TransferFunctionProperty::Pointer transferFuncProp;
  if (node->GetProperty(transferFuncProp, "TransferFunction", renderer))
  {
    mitk::TransferFunction::Pointer transferFunction = transferFuncProp->GetValue();

    volProp->SetColor(transferFunction->GetColorTransferFunction());
    volProp->SetScalarOpacity(transferFunction->GetScalarOpacityFunction());
    volProp->SetGradientOpacity(transferFunction->GetGradientOpacityFunction());

    m_VtkDataSetMapper->SetLookupTable(transferFunction->GetColorTransferFunction());
    m_VtkDataSetMapper2->SetLookupTable(transferFunction->GetColorTransferFunction());
  }

  bool isVolumeRenderingOn = false;
  node->GetBoolProperty("volumerendering", isVolumeRenderingOn, renderer);

  if (isVolumeRenderingOn)
  {
    m_Assembly->RemovePart(m_Actor);
    m_Assembly->RemovePart(m_ActorWireframe);
    m_Assembly->AddPart(m_Volume);

    mitk::GridVolumeMapperProperty::Pointer mapperProp;
    if (node->GetProperty(mapperProp, "volumerendering.mapper", renderer))
    {
      mitk::GridVolumeMapperProperty::IdType type = mapperProp->GetValueAsId();
      switch (type) {
        case mitk::GridVolumeMapperProperty::RAYCAST:
          if (m_VtkVolumeRayCastMapper == 0) {
            m_VtkVolumeRayCastMapper = vtkUnstructuredGridVolumeRayCastMapper::New();
            m_VtkVolumeRayCastMapper->SetInputConnection(m_VtkTriangleFilter->GetOutputPort());
          }
          m_Volume->SetMapper(m_VtkVolumeRayCastMapper);
          break;
        case mitk::GridVolumeMapperProperty::PT:
          if (m_VtkPTMapper == 0) {
            m_VtkPTMapper = vtkProjectedTetrahedraMapper::New();
            m_VtkPTMapper->SetInputConnection(m_VtkTriangleFilter->GetOutputPort());
          }
          m_Volume->SetMapper(m_VtkPTMapper);
          break;
        case mitk::GridVolumeMapperProperty::ZSWEEP:
          if (m_VtkVolumeZSweepMapper == 0) {
            m_VtkVolumeZSweepMapper = vtkUnstructuredGridVolumeZSweepMapper::New();
            m_VtkVolumeZSweepMapper->SetInputConnection(m_VtkTriangleFilter->GetOutputPort());
          }
          m_Volume->SetMapper(m_VtkVolumeZSweepMapper);
          break;
      }
    }
  }
  else
  {
    m_Assembly->RemovePart(m_Volume);
    m_Assembly->AddPart(m_Actor);
    m_Assembly->RemovePart(m_ActorWireframe);

    mitk::GridRepresentationProperty::Pointer gridRepProp;
    if (node->GetProperty(gridRepProp, "grid representation", renderer))
    {
      mitk::GridRepresentationProperty::IdType type = gridRepProp->GetValueAsId();
      switch (type) {
        case mitk::GridRepresentationProperty::POINTS:
          property->SetRepresentationToPoints();
          break;
        case mitk::GridRepresentationProperty::WIREFRAME:
          property->SetRepresentationToWireframe();
          break;
        case mitk::GridRepresentationProperty::SURFACE:
          property->SetRepresentationToSurface();
          break;
      }

//      if (type == mitk::GridRepresentationProperty::WIREFRAME_SURFACE)
//      {
//        m_Assembly->AddPart(m_ActorWireframe);
//      }
    }
  }


//   mitk::LevelWindow levelWindow;
//   if(node->GetLevelWindow(levelWindow, renderer, "levelWindow"))
//   {
//     m_VtkVolumeRayCastMapper->SetScalarRange(levelWindow.GetMin(),levelWindow.GetMax());
//   }
//   else
//   if(node->GetLevelWindow(levelWindow, renderer))
//   {
//     m_VtkVolumeRayCastMapper->SetScalarRange(levelWindow.GetMin(),levelWindow.GetMax());
//   }
//
//   mitk::VtkRepresentationProperty* representationProperty;
//   node->GetProperty(representationProperty, "material.representation", renderer);
//   if ( representationProperty != NULL )
//     m_Volume->GetProperty()->SetRepresentation( representationProperty->GetVtkRepresentation() );
//
//   mitk::VtkInterpolationProperty* interpolationProperty;
//   node->GetProperty(interpolationProperty, "material.interpolation", renderer);
//   if ( interpolationProperty != NULL )
//     m_Volume->GetProperty()->SetInterpolation( interpolationProperty->GetVtkInterpolation() );
//

   mitk::VtkScalarModeProperty* scalarMode = 0;
   if(node->GetProperty(scalarMode, "scalar mode", renderer))
   {
     if (m_VtkVolumeRayCastMapper)
       m_VtkVolumeRayCastMapper->SetScalarMode(scalarMode->GetVtkScalarMode());
     if (m_VtkPTMapper)
       m_VtkPTMapper->SetScalarMode(scalarMode->GetVtkScalarMode());
     if (m_VtkVolumeZSweepMapper)
       m_VtkVolumeZSweepMapper->SetScalarMode(scalarMode->GetVtkScalarMode());

     m_VtkDataSetMapper->SetScalarMode(scalarMode->GetVtkScalarMode());
     m_VtkDataSetMapper2->SetScalarMode(scalarMode->GetVtkScalarMode());
   }
   else
   {
     if (m_VtkVolumeRayCastMapper)
       m_VtkVolumeRayCastMapper->SetScalarModeToDefault();
     if (m_VtkPTMapper)
       m_VtkPTMapper->SetScalarModeToDefault();
     if (m_VtkVolumeZSweepMapper)
       m_VtkVolumeZSweepMapper->SetScalarModeToDefault();

     m_VtkDataSetMapper->SetScalarModeToDefault();
     m_VtkDataSetMapper2->SetScalarModeToDefault();
   }

   bool scalarVisibility = true;
   node->GetBoolProperty("scalar visibility", scalarVisibility, renderer);
   m_VtkDataSetMapper->SetScalarVisibility(scalarVisibility ? 1 : 0);
   m_VtkDataSetMapper2->SetScalarVisibility(scalarVisibility ? 1 : 0);

//   double scalarRangeLower = std::numeric_limits<double>::min();
//   double scalarRangeUpper = std::numeric_limits<double>::max();
//   mitk::DoubleProperty* lowerRange = 0;
//   if (node->GetProperty(lowerRange, "scalar range min", renderer))
//   {
//     scalarRangeLower = lowerRange->GetValue();
//   }
//   mitk::DoubleProperty* upperRange = 0;
//   if (node->GetProperty(upperRange, "scalar range max", renderer))
//   {
//     scalarRangeUpper = upperRange->GetValue();
//   }
//   m_VtkDataSetMapper->SetScalarRange(scalarRangeLower, scalarRangeUpper);
//   m_VtkDataSetMapper2->SetScalarRange(scalarRangeLower, scalarRangeUpper);


//   bool colorMode = false;
//   node->GetBoolProperty("color mode", colorMode);
//   m_VtkVolumeRayCastMapper->SetColorMode( (colorMode ? 1 : 0) );

//   double scalarsMin = 0;
//   node->GetDoubleProperty("ScalarsRangeMinimum", scalarsMin, renderer);

//   double scalarsMax = 1.0;
//   node->GetProperty("ScalarsRangeMaximum", scalarsMax, renderer);

//   m_VtkVolumeRayCastMapper->SetScalarRange(scalarsMin,scalarsMax);

}

void mitk::UnstructuredGridVtkMapper3D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{

  SurfaceVtkMapper3D::SetDefaultPropertiesForVtkProperty(node, renderer, overwrite);

  node->AddProperty("grid representation", GridRepresentationProperty::New(), renderer, overwrite);
  node->AddProperty("volumerendering", BoolProperty::New(false), renderer, overwrite);
  node->AddProperty("volumerendering.mapper", GridVolumeMapperProperty::New(), renderer, overwrite);
  node->AddProperty("scalar mode", VtkScalarModeProperty::New(0), renderer, overwrite);
  node->AddProperty("scalar visibility", BoolProperty::New(true), renderer, overwrite);
  //node->AddProperty("scalar range min", DoubleProperty::New(std::numeric_limits<double>::min()), renderer, overwrite);
  //node->AddProperty("scalar range max", DoubleProperty::New(std::numeric_limits<double>::max()), renderer, overwrite);
  node->AddProperty("outline polygons", BoolProperty::New(false), renderer, overwrite);
  node->AddProperty("color", ColorProperty::New(1.0f, 1.0f, 1.0f), renderer, overwrite);
  node->AddProperty("line width", IntProperty::New(1), renderer, overwrite);

  if(overwrite || node->GetProperty("TransferFunction", renderer) == 0)
  {
    // add a default transfer function
    mitk::TransferFunction::Pointer tf = mitk::TransferFunction::New();
    //tf->GetColorTransferFunction()->RemoveAllPoints();
    node->SetProperty ("TransferFunction", mitk::TransferFunctionProperty::New(tf.GetPointer()));
  }

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}
