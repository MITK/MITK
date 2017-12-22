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

#ifndef __vtkUnstructuredGridMapper_h
#define __vtkUnstructuredGridMapper_h

#include "mitkCommon.h"
#include "MitkMapperExtExports.h"
#include "mitkBoundingObject.h"
#include "mitkBaseRenderer.h"

#include "vtkMapper.h"

class vtkPolyDataMapper;
class vtkGeometryFilter;
class vtkUnstructuredGrid;

class MITKMAPPEREXT_EXPORT vtkUnstructuredGridMapper : public vtkMapper
{
public:
  static vtkUnstructuredGridMapper *New();
  vtkTypeMacro(vtkUnstructuredGridMapper,vtkMapper);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  void Render(vtkRenderer *ren, vtkActor *act) override;

  // Description:
  // Get the internal poly data mapper used to map data set to graphics system.
  vtkGetObjectMacro(PolyDataMapper, vtkPolyDataMapper);

  // Description:
  // Release any graphics resources that are being consumed by this mapper.
  // The parameter window could be used to determine which graphic
  // resources to release.
   // deprecatedSince{2013_12} Use ReleaseGraphicsResources(mitk::BaseRenderer* renderer) instead
  DEPRECATED(void ReleaseGraphicsResources(vtkWindow *) override);

   // Description:
  // Release any graphics resources that are being consumed by this mapper.
  // The parameter renderer could be used to determine which graphic
  // resources to release.
   // deprecatedSince{2013_12} Use ReleaseGraphicsResources(mitk::BaseRenderer* renderer) instead
  void ReleaseGraphicsResources(mitk::BaseRenderer * renderer);


  // Description:
  // Get the mtime also considering the lookup table.
  vtkMTimeType GetMTime() override;

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

  virtual void ReportReferences(vtkGarbageCollector*) override;

  // see algorithm for more info
  virtual int FillInputPortInformation(int port, vtkInformation* info) override;

private:
  vtkUnstructuredGridMapper(const vtkUnstructuredGridMapper&);  // Not implemented.
  void operator=(const vtkUnstructuredGridMapper&);  // Not implemented.
};

#endif // __vtkUnstructuredGridMapper_h
