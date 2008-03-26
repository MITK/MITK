/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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
#include "vtkUnstructuredGridMapper.h"

#include "vtkGeometryFilter.h"
#include "vtkExecutive.h"
#include "vtkGarbageCollector.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkUnstructuredGrid.h"

vtkCxxRevisionMacro(vtkUnstructuredGridMapper, "$Revision$");
vtkStandardNewMacro(vtkUnstructuredGridMapper);

//----------------------------------------------------------------------------
vtkUnstructuredGridMapper::vtkUnstructuredGridMapper()
{
  this->GeometryExtractor = 0;
  this->PolyDataMapper = 0;
}

//----------------------------------------------------------------------------
vtkUnstructuredGridMapper::~vtkUnstructuredGridMapper()
{
  // delete internally created objects.
  if ( this->GeometryExtractor )
    {
    this->GeometryExtractor->Delete();
    }
  if ( this->PolyDataMapper )
    {
    this->PolyDataMapper->Delete();
    }
  
}

void vtkUnstructuredGridMapper::SetBoundingObject(mitk::BoundingObject* bo)
{
  m_BoundingObject = bo;
}

//----------------------------------------------------------------------------
void vtkUnstructuredGridMapper::SetInput(vtkUnstructuredGrid *input)
{
  if(input)
    {
    this->SetInputConnection(0, input->GetProducerPort());
    }
  else
    {
    // Setting a NULL input removes the connection.
    this->SetInputConnection(0, 0);
    }
}

//----------------------------------------------------------------------------
vtkUnstructuredGrid *vtkUnstructuredGridMapper::GetInput()
{
  //return this->Superclass::GetInputAsDataSet();
  return vtkUnstructuredGrid::SafeDownCast(
      this->GetExecutive()->GetInputData(0, 0));
}

//----------------------------------------------------------------------------
void vtkUnstructuredGridMapper::ReleaseGraphicsResources( vtkWindow *renWin )
{
  if (this->PolyDataMapper)
    {
    this->PolyDataMapper->ReleaseGraphicsResources( renWin );
    }
}

//----------------------------------------------------------------------------
// Receives from Actor -> maps data to primitives
//
void vtkUnstructuredGridMapper::Render(vtkRenderer *ren, vtkActor *act)
{
  // make sure that we've been properly initialized
  //
  if ( !this->GetInput() )
    {
    vtkErrorMacro(<< "No input!\n");
    return;
    } 

  // Need a lookup table
  //
  if ( this->LookupTable == 0 )
    {
    this->CreateDefaultLookupTable();
    }
  this->LookupTable->Build();

  // Now can create appropriate mapper
  //
  if ( this->PolyDataMapper == 0 ) 
    {
    vtkGeometryFilter *gf = vtkGeometryFilter::New();
    vtkPolyDataMapper *pm = vtkPolyDataMapper::New();
    pm->SetInput(gf->GetOutput());

    this->GeometryExtractor = gf;
    this->PolyDataMapper = pm;
    }

  // share clipping planes with the PolyDataMapper
  //
  if (this->ClippingPlanes != this->PolyDataMapper->GetClippingPlanes()) 
    {
    this->PolyDataMapper->SetClippingPlanes(this->ClippingPlanes);
    }
  
  if (this->m_BoundingObject)
  {
    mitk::BoundingBox::BoundsArrayType bounds = this->m_BoundingObject->GetGeometry()->CalculateBoundingBoxRelativeToTransform(0)->GetBounds();
    this->GeometryExtractor->SetExtent(bounds[0], bounds[1], bounds[2], 
        bounds[3], bounds[4], bounds[5]);
    this->GeometryExtractor->ExtentClippingOn();
  }
  else
  {
    this->GeometryExtractor->ExtentClippingOff();
  }

  this->GeometryExtractor->SetInput(this->GetInput());
  this->PolyDataMapper->SetInput(this->GeometryExtractor->GetOutput());
  
  // update ourselves in case something has changed
  this->PolyDataMapper->SetLookupTable(this->GetLookupTable());
  this->PolyDataMapper->SetScalarVisibility(this->GetScalarVisibility());
  this->PolyDataMapper->SetUseLookupTableScalarRange(
    this->GetUseLookupTableScalarRange());
  this->PolyDataMapper->SetScalarRange(this->GetScalarRange());
  this->PolyDataMapper->SetImmediateModeRendering(
    this->GetImmediateModeRendering());
  this->PolyDataMapper->SetColorMode(this->GetColorMode());
  this->PolyDataMapper->SetInterpolateScalarsBeforeMapping(
                               this->GetInterpolateScalarsBeforeMapping());

  this->PolyDataMapper->SetScalarMode(this->GetScalarMode());
  if ( this->ScalarMode == VTK_SCALAR_MODE_USE_POINT_FIELD_DATA ||
       this->ScalarMode == VTK_SCALAR_MODE_USE_CELL_FIELD_DATA )
    {
    if ( this->ArrayAccessMode == VTK_GET_ARRAY_BY_ID )
      {
      this->PolyDataMapper->ColorByArrayComponent(this->ArrayId,ArrayComponent);
      }
    else
      {
      this->PolyDataMapper->ColorByArrayComponent(this->ArrayName,ArrayComponent);
      }
    }
  
  this->PolyDataMapper->Render(ren,act);
  this->TimeToDraw = this->PolyDataMapper->GetTimeToDraw();
}

//----------------------------------------------------------------------------
void vtkUnstructuredGridMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if ( this->PolyDataMapper )
    {
    os << indent << "Poly Mapper: (" << this->PolyDataMapper << ")\n";
    }
  else
    {
    os << indent << "Poly Mapper: (none)\n";
    }

  if ( this->GeometryExtractor )
    {
    os << indent << "Geometry Extractor: (" << this->GeometryExtractor << ")\n";
    }
  else
    {
    os << indent << "Geometry Extractor: (none)\n";
    }
}

//----------------------------------------------------------------------------
unsigned long vtkUnstructuredGridMapper::GetMTime()
{
  unsigned long mTime=this->vtkMapper::GetMTime();
  unsigned long time;

  if ( this->LookupTable != NULL )
    {
    time = this->LookupTable->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }

  return mTime;
}

//----------------------------------------------------------------------------
int vtkUnstructuredGridMapper::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkUnstructuredGrid");
  return 1;
}

//----------------------------------------------------------------------------
void vtkUnstructuredGridMapper::ReportReferences(vtkGarbageCollector* collector)
{
  this->Superclass::ReportReferences(collector);
  // These filters share our input and are therefore involved in a
  // reference loop.
  vtkGarbageCollectorReport(collector, this->GeometryExtractor,
                            "GeometryExtractor");
  vtkGarbageCollectorReport(collector, this->PolyDataMapper,
                            "PolyDataMapper");
}
