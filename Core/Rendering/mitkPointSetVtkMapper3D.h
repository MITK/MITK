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


#ifndef MITKPointSetVtkMAPPER3D_H_HEADER_INCLUDED_C1907273
#define MITKPointSetVtkMAPPER3D_H_HEADER_INCLUDED_C1907273

#include "mitkCommon.h"
#include "mitkBaseVtkMapper3D.h"
#include "mitkBaseRenderer.h"

class vtkActor;
class vtkPropAssembly;
class vtkAppendPolyData;
class vtkPolyData;
class vtkTubeFilter;
class vtkPolyDataMapper;

namespace mitk {

class PointSet;

/**
  * @brief Vtk-based mapper for PointSet
  *
  * Due to the need of different colors for selected 
  * and unselected points and the facts, that we also have a contour and 
  * labels for the points, the vtk structure is build up the following way: 
  *
  * We have two AppendPolyData, one selected, and one unselected and one 
  * for a contour between the points. Each one is connected to an own 
  * PolyDaraMapper and an Actor. The different color for the unselected and 
  * selected state and for the contour is read from properties.
  *
  * "unselectedcolor", "selectedcolor" and "contourcolor" are the strings, 
  * that are looked for. Pointlabels are added besides the selected or the 
  * deselected points.
  *
  * Then the three Actors are combined inside a vtkPropAssembly and this 
  * object is returned in GetProp() and so hooked up into the rendering 
  * pipeline. Other properties looked for are:
  *
  *   - \b "contour": if set to on, lines between the points are shown
  *   - \b "close": if set to on, the open strip is closed (first point 
  *       connected with last point)
  *   - \b "pointsize": size of the points mapped
  *   - \b "label": text of the Points to show besides points
  *   - \b "contoursize": size of the contour drawn between the points 
  *       (if not set, the pointsize is taken)
  *
  * @ingroup Mapper
  */
class PointSetVtkMapper3D : public BaseVtkMapper3D
{
public:
  mitkClassMacro(PointSetVtkMapper3D, BaseVtkMapper3D);

  itkNewMacro(Self);

  virtual const mitk::PointSet* GetInput();

  //overwritten from BaseVtkMapper3D to be able to return a 
  //m_PointsAssembly which is much faster than a vtkAssembly
  virtual vtkProp* GetProp();
  virtual void UpdateVtkTransform(mitk::BaseRenderer* renderer);

protected:
  PointSetVtkMapper3D();

  virtual ~PointSetVtkMapper3D();

  virtual void GenerateData();
  virtual void GenerateData(mitk::BaseRenderer* renderer);
  virtual void ApplyProperties(mitk::BaseRenderer* renderer);
  virtual void CreateContour(mitk::BaseRenderer* renderer);

  vtkAppendPolyData *m_vtkSelectedPointList;
  vtkAppendPolyData *m_vtkUnselectedPointList;
//  vtkAppendPolyData *m_vtkContourPolyData;

  vtkPolyDataMapper *m_VtkSelectedPolyDataMapper;
  vtkPolyDataMapper *m_VtkUnselectedPolyDataMapper;
//  vtkPolyDataMapper *m_vtkContourPolyDataMapper;

  vtkActor *m_SelectedActor;
  vtkActor *m_UnselectedActor;
  vtkActor *m_ContourActor;

  vtkPropAssembly *m_PointsAssembly;

  //help for contour between points
  vtkAppendPolyData *m_vtkTextList;
  //vtkPolyData *m_Contour;
  //vtkTubeFilter *m_TubeFilter;

  //variables to be able to log, how many inputs have been added to PolyDatas
  unsigned int m_NumberOfSelectedAdded;
  unsigned int m_NumberOfUnselectedAdded;
};


} // namespace mitk

#endif /* MITKPointSetVtkMAPPER3D_H_HEADER_INCLUDED_C1907273 */
