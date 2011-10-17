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


#ifndef PlanarCircleMapper3D_H_HEADER_INCLUDED
#define PlanarCircleMapper3D_H_HEADER_INCLUDED

//#include "mitkCommon.h"
//#include "mitkBaseRenderer.h"


#include <mitkFiberBundle.h>
#include <vtkSmartPointer.h>
#include <mitkVtkMapper3D.h>
//#include "MitkDiffusionImagingMBIExports.h"
#include <mitkBaseData.h>
#include <vtkAppendPolyData.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkOpenGLActor.h>
#include <vtkPropAssembly.h>
#include <vtkProperty.h>
#include <vtkUnsignedCharArray.h>
#include <vtkTubeFilter.h>
#include <vtkRegularPolygonSource.h>

//class mitkPlanarCircle;
#include <mitkPlanarCircle.h>

namespace mitk {
  
  //##Documentation
  //## @brief Mapper for FiberBundles
  //## @ingroup Mapper
  //  template<class TPixelType>
  class /*MitkDiffusionImagingMBI_EXPORT*/ PlanarCircleMapper3D : public VtkMapper3D
  {
  public:
    
    mitkClassMacro(PlanarCircleMapper3D, VtkMapper3D);
    itkNewMacro(Self);
    
    
    const mitk::PlanarCircle* GetInput();
    
    virtual vtkProp *GetVtkProp(mitk::BaseRenderer *renderer); //looks like depricated.. should be replaced bz GetViewProp()
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = NULL, bool overwrite = false );
    
    virtual void ApplyProperties(mitk::BaseRenderer* renderer);
    static void SetVtkMapperImmediateModeRendering(vtkMapper *mapper);
    
    virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer);
    virtual void GenerateData();
    
    
  protected:
    
    PlanarCircleMapper3D();
    virtual ~PlanarCircleMapper3D();
    
    void UpdateVtkObjects();
    
    
    vtkAppendPolyData *m_vtkCircleList;

    
    vtkOpenGLPolyDataMapper *m_VtkCircleDataMapperGL;
        
    vtkOpenGLActor *m_CircleActor;

    
    
    vtkPropAssembly *m_CircleAssembly;
    
    vtkRegularPolygonSource *m_polygonSource;
    
  };
  
} // namespace mitk




#endif /* FiberBundleMapper3D_H_HEADER_INCLUDED */

