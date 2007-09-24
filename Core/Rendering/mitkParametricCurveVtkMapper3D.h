/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 12006 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKPARAMETRICCURVEMAPPER3D_H_HEADER_INCLUDED
#define MITKPARAMETRICCURVEMAPPER3D_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkBaseVtkMapper3D.h"
#include "mitkBaseRenderer.h"

class vtkTubeFilter;
class vtkPolyData;
class vtkPoints;
class vtkCellArray;
class vtkActor;
class vtkPropAssembly;
class vtkAppendPolyData;
class vtkPolyData;
class vtkPolyDataMapper;

namespace mitk {

class ParametricCurve;

/**
 * \brief VTK based mapper for parametric curves
 *
 * \ingroup Mapper
 */
class ParametricCurveVtkMapper3D : public BaseVtkMapper3D
{
public:
  mitkClassMacro( ParametricCurveVtkMapper3D, BaseVtkMapper3D );

  itkNewMacro(Self);

  virtual const mitk::ParametricCurve *GetInput();

protected:
  ParametricCurveVtkMapper3D();

  virtual ~ParametricCurveVtkMapper3D();

  virtual void GenerateData();
  virtual void GenerateData( mitk::BaseRenderer *renderer );

  vtkActor *m_Actor;
  vtkPolyDataMapper *m_Mapper;

  vtkTubeFilter *m_TubeFilter;
  vtkPolyData *m_CurveData;
  vtkPoints *m_CurvePoints;
  vtkCellArray *m_CurveLines;

};


} // namespace mitk

#endif /* MITKPARAMETRICCURVEMAPPER3D_H_HEADER_INCLUDED */
