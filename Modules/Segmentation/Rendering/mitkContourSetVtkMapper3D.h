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


#ifndef MITK_CONTOUR_SET_VTK_MAPPER_3D_H
#define MITK_CONTOUR_SET_VTK_MAPPER_3D_H

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include "mitkVtkMapper3D.h"
#include "mitkContourSet.h"
#include "mitkBaseRenderer.h"

#include <vtkPolyData.h>

class vtkPolyDataMapper;
class vtkAppendPolyData;
class vtkActor;
class vtkTubeFilter;

namespace mitk {

//##Documentation
//## @brief Vtk-based mapper for mitk::Contour
//## @ingroup Mapper
class Segmentation_EXPORT ContourSetVtkMapper3D : public VtkMapper3D
{
  public:

    mitkClassMacro(ContourSetVtkMapper3D, VtkMapper3D);

    itkNewMacro(Self);

    virtual const mitk::ContourSet* GetInput();

    virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer);

  protected:
   
    ContourSetVtkMapper3D();
   
    virtual ~ContourSetVtkMapper3D();

    virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer);

    vtkPolyDataMapper* m_VtkPolyDataMapper;
    vtkTubeFilter*     m_TubeFilter;

    vtkPolyData *m_ContourSet;
    vtkActor *m_Actor;
  
};

} // namespace mitk

#endif // MITK_CONTOUR_VTK_MAPPER_3D_H
