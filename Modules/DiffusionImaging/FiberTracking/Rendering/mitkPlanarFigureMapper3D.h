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


#ifndef PlanarFigureMapper3D_H
#define PlanarFigureMapper3D_H

#include <mitkBaseData.h>
#include <mitkVtkMapper3D.h>
#include <mitkPlanarFigure.h>

#include <vtkPoints.h>
#include <vtkPolygon.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkOpenGLActor.h>
#include <vtkPropAssembly.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

namespace mitk {

  //##Documentation
  //## @brief 3D mapper for planar figures
  //## @ingroup Mapper
  //  template<class TPixelType>
  class PlanarFigureMapper3D : public VtkMapper3D
  {
  public:

    mitkClassMacro(PlanarFigureMapper3D, VtkMapper3D)
    itkNewMacro(Self)

    const mitk::PlanarFigure* GetInput();

    virtual vtkProp *GetVtkProp(mitk::BaseRenderer *renderer); //looks like depricated.. should be replaced bz GetViewProp()
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = NULL, bool overwrite = false );

    static void SetVtkMapperImmediateModeRendering(vtkMapper *mapper);

    virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer);
    virtual void GenerateData();

  protected:

    PlanarFigureMapper3D();
    virtual ~PlanarFigureMapper3D();

    void UpdateVtkObjects();

    vtkSmartPointer<vtkPoints>                m_points;
    vtkSmartPointer<vtkPolygon>               m_polygon;
    vtkSmartPointer<vtkPolyData>              m_polygonPolyData;
    vtkSmartPointer<vtkCellArray>             m_polygonsCell;
    vtkSmartPointer<vtkOpenGLPolyDataMapper>  m_VtkPolygonDataMapperGL;
    vtkSmartPointer<vtkOpenGLActor>           m_PolygonActor;
    vtkSmartPointer<vtkPropAssembly>          m_PolygonAssembly;
  };

} // namespace mitk

#endif /* FiberBundleMapper3D_H_HEADER_INCLUDED */

