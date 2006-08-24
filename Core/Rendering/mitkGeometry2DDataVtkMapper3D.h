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


#ifndef MITKGEOMETRY2DDATAVTKMAPPER3D_H_HEADER_INCLUDED_C196C71F
#define MITKGEOMETRY2DDATAVTKMAPPER3D_H_HEADER_INCLUDED_C196C71F

#include "mitkCommon.h"
#include "mitkBaseVtkMapper3D.h"
#include "mitkDataTree.h"

class vtkActor;
class vtkTexture;
class vtkPlaneSource;
class vtkImageMapToWindowLevelColors;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkLookupTable;
class vtkAssembly;
class vtkFeatureEdges;
class vtkTubeFilter;

namespace mitk {

class Geometry2DData;
class BaseRenderer;

//##ModelId=3E691E09014F
//##Documentation
//## @brief Vtk-based mapper to display a Geometry2D in a 3D window
//##
//## @ingroup Mapper
class Geometry2DDataVtkMapper3D : public BaseVtkMapper3D
{
  public:
    //##ModelId=3E691E09036C
    mitkClassMacro(Geometry2DDataVtkMapper3D, BaseVtkMapper3D);

    itkNewMacro(Geometry2DDataVtkMapper3D);

    //##ModelId=3E691E090380
    //##Documentation
    //## @brief Get the Geometry2DData to map
    virtual const mitk::Geometry2DData *GetInput();
  
    //##ModelId=3E6E874F0007
    //##Documentation
    //## @brief The first image found when traversing the @a iterator (stored in m_DataTreeIterator)
    //## will be used as a texture on the mapped Geometry2D, when it has 
    //## been resliced by an ImageMapper2D according to the Geometry2D.
    //## See ImageMapper2D for problems of the current version.
    //## @warning Works only when the node to the Geometry2DData was created by 
    //## calling BaseRenderer::GetCurrentWorldGeometry2DNode()
    virtual void SetDataIteratorForTexture(const mitk::DataTreeIteratorBase* iterator);

  protected:
    //##ModelId=3E691E09038E
    Geometry2DDataVtkMapper3D();

    //##ModelId=3E691E090394
    virtual ~Geometry2DDataVtkMapper3D();

    virtual void GenerateData(mitk::BaseRenderer* renderer);

    //##ModelId=3E691E090331
    //##Documentation
    //## @brief vtk-mapper to map the vtk-representation of the Geometry2D
    vtkPolyDataMapper* m_VtkPolyDataMapper;

    //##ModelId=3E691E090344
    //##Documentation
    //## @brief vtk-actor containing the m_VtkPolyDataMapper and the m_VtkTexture
	//## if set.
    vtkActor* m_ImageActor;

    //##Documentation
    //## @brief PropAssembly to hold the plane and its tubal frame
    vtkAssembly* m_Prop3DAssembly;

    //##ModelId=3E691E090358
    //##Documentation
    //## @brief vtk-representation of the Geometry2D
    vtkPolyData* m_VtkPolyData;

    //##ModelId=3E691E090362
    //##Documentation
    //## @brief source to create the vtk-representation of a PlaneGeometry
    vtkPlaneSource* m_VtkPlaneSource;

    //##Documentation
    //## @brief Edge extractor for tube-shaped frame
    vtkFeatureEdges* m_Edges;

    //##Documentation
    //## @brief Source to create the tube-shaped frame
    vtkTubeFilter* m_EdgeTuber;

    //##Documentation
    //## @brief Mapper for the tube-shaped frame
    vtkPolyDataMapper* m_EdgeMapper;

    //##Documentation
    //## @brief Actor for the tube-shaped frame
    vtkActor* m_EdgeActor;

    //##ModelId=3E6E8AA40375
    //## @brief The first image found when traversing the this iterator (if set)
    //## will be used as a texture on the mapped Geometry2D, when it has 
    //## been resliced by an ImageMapper2D according to the Geometry2D.
    //## \sa SetDataIteratorForTexture
    mitk::DataTreeIteratorClone m_DataTreeIterator;

    //##ModelId=3E691E09034E
    //##Documentation
    //## @brief vtk-texture when m_DataTreeIterator is set (by SetDataIteratorForTexture)
    //## if set.
    vtkTexture* m_VtkTexture;

    //##ModelId=3E6E8AA4034C
    //##Documentation
    //## @brief lookup-table for texture (m_VtkTexture)
    vtkLookupTable *m_VtkLookupTable;

    //##Documentation
    //## @brief default lookup-table for texture (m_VtkTexture)
    vtkLookupTable *m_VtkLookupTableDefault;

    //##ModelId=3EDD039F0240
    //##Documentation
    //## @brief timestamp of last update of texture (m_VtkTexture) from image data
    unsigned long int m_LastTextureUpdateTime;
};

} // namespace mitk



#endif /* MITKGEOMETRY2DDATAVTKMAPPER3D_H_HEADER_INCLUDED_C196C71F */
