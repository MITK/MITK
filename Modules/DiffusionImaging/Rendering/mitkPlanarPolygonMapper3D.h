/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date: 2009-05-12 19:56:03 +0200 (Di, 12 Mai 2009) $
 Version:   $Revision: 17179 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef PlanarPolygonMapper3D_H
#define PlanarPolygonMapper3D_H

#include <mitkBaseData.h>
#include <mitkVtkMapper3D.h>
#include <mitkPlanarPolygon.h>

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
  //## @brief Mapper for FiberBundles
  //## @ingroup Mapper
  //  template<class TPixelType>
  class PlanarPolygonMapper3D : public VtkMapper3D
  {
  public:

    mitkClassMacro(PlanarPolygonMapper3D, VtkMapper3D);
    itkNewMacro(Self);

    const mitk::PlanarPolygon* GetInput();

    virtual vtkProp *GetVtkProp(mitk::BaseRenderer *renderer); //looks like depricated.. should be replaced bz GetViewProp()
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = NULL, bool overwrite = false );

    static void SetVtkMapperImmediateModeRendering(vtkMapper *mapper);

    virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer);
    virtual void GenerateData();

  protected:

    PlanarPolygonMapper3D();
    virtual ~PlanarPolygonMapper3D();

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

