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

#ifndef __vtkUnstructuredGridMapper_h
#define __vtkUnstructuredGridMapper_h

#include "mitkCommon.h"
#include "mitkBoundingObject.h"

#include "vtkMapper.h"

class vtkPolyDataMapper;
class vtkGeometryFilter;
class vtkUnstructuredGrid;

class MITKEXT_CORE_EXPORT vtkUnstructuredGridMapper : public vtkMapper 
{
public:
  static vtkUnstructuredGridMapper *New();
  vtkTypeRevisionMacro(vtkUnstructuredGridMapper,vtkMapper);
  void PrintSelf(ostream& os, vtkIndent indent);
  void Render(vtkRenderer *ren, vtkActor *act);

  // Description:
  // Get the internal poly data mapper used to map data set to graphics system.
  vtkGetObjectMacro(PolyDataMapper, vtkPolyDataMapper);

  // Description:
  // Release any graphics resources that are being consumed by this mapper.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Get the mtime also considering the lookup table.
  unsigned long GetMTime();

  // Description:
  // Set the Input of this mapper.
  void SetInput(vtkUnstructuredGrid *input);
  vtkUnstructuredGrid *GetInput();
  
  void SetBoundingObject(mitk::BoundingObject* bo);
  
protected:
  vtkUnstructuredGridMapper();
  ~vtkUnstructuredGridMapper();

  vtkGeometryFilter *GeometryExtractor;
  vtkPolyDataMapper *PolyDataMapper;
  
  mitk::BoundingObject::Pointer m_BoundingObject;

  virtual void ReportReferences(vtkGarbageCollector*);

  // see algorithm for more info
  virtual int FillInputPortInformation(int port, vtkInformation* info);

private:
  vtkUnstructuredGridMapper(const vtkUnstructuredGridMapper&);  // Not implemented.
  void operator=(const vtkUnstructuredGridMapper&);  // Not implemented.
};

#endif // __vtkUnstructuredGridMapper_h
