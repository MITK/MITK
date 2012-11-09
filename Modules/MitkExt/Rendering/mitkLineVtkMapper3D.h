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


#ifndef MITKLINEVTKMAPPER3D_H_HEADER_INCLUDED
#define MITKLINEVTKMAPPER3D_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "MitkExtExports.h"
#include "mitkPointSetVtkMapper3D.h"
#include "mitkPointSet.h"
#include "mitkBaseRenderer.h"

#include <vtkSphereSource.h>
#include <vtkAppendPolyData.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkTubeFilter.h>
#include <vtkVectorText.h>
#include <vtkTextSource.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>


class vtkActor;
class vtkAssembly;
class vtkFollower;
class vtkPolyDataMapper;

namespace mitk {

//##Documentation
//## @brief Vtk-based mapper to draw Lines from PointSet
//##
//## @ingroup Mapper
class MitkExt_EXPORT LineVtkMapper3D : public PointSetVtkMapper3D
{
  public:

    mitkClassMacro(LineVtkMapper3D, PointSetVtkMapper3D);

    itkNewMacro(Self);


  protected:
    LineVtkMapper3D();
    virtual ~LineVtkMapper3D();

    virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer);
};

} // namespace mitk

#endif /* MITKLINEVTKMAPPER3D_H_HEADER_INCLUDED */
