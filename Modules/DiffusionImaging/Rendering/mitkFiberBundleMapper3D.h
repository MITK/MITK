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


#ifndef FiberBundleMapper3D_H_HEADER_INCLUDED
#define FiberBundleMapper3D_H_HEADER_INCLUDED

//#include "mitkCommon.h"
//#include "mitkBaseRenderer.h"

#include "mitkFiberBundle.h"
#include <vtkSmartPointer.h>
#include "mitkVtkMapper3D.h"
#include "MitkDiffusionImagingExports.h"
#include "mitkBaseData.h"
#include "vtkAppendPolyData.h"
#include "vtkOpenGLPolyDataMapper.h"
#include "vtkOpenGLActor.h"
#include "vtkPropAssembly.h"
#include "vtkProperty.h"
#include "vtkUnsignedCharArray.h"
#include "vtkTubeFilter.h"


namespace mitk {

  //##Documentation
  //## @brief Mapper for FiberBundles
  //## @ingroup Mapper
//  template<class TPixelType>
  class MitkDiffusionImaging_EXPORT FiberBundleMapper3D : public VtkMapper3D
  {
  public:

    mitkClassMacro(FiberBundleMapper3D, VtkMapper3D);
    itkNewMacro(Self);

    
    const mitk::FiberBundle* GetInput();

    virtual vtkProp *GetVtkProp(mitk::BaseRenderer *renderer); //looks like depricated.. should be replaced bz GetViewProp()
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = NULL, bool overwrite = false );

    virtual void ApplyProperties(mitk::BaseRenderer* renderer);
    static void SetVtkMapperImmediateModeRendering(vtkMapper *mapper);
    
    virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer);
    virtual void GenerateData();

    
  protected:

    FiberBundleMapper3D();
    virtual ~FiberBundleMapper3D();

    void UpdateVtkObjects();
    
    
    vtkAppendPolyData *m_vtkFiberList;
    //vtkSmartPointer<vtkAppendPolyData> m_vtkFiberList;
    
    vtkOpenGLPolyDataMapper *m_VtkFiberDataMapperGL;
    //vtkPainterPolyDataMapper *m_VtkFiberDataMapperGL;
    //vtkSmartPointer<vtkOpenGLPolyDataMapper> m_VtkFiberDataMapperGL;
    
    //vtkOpenGLPolyDataMapper *m_VtkFiberDataMapper;
    //vtkSmartPointer<vtkPolyDataMapper> m_VtkFiberDataMapper;
    
    vtkOpenGLActor *m_FiberActor;
    //vtkSmartPointer<vtkActor> m_FiberActor;
    
    vtkTubeFilter *m_tubes;
    vtkOpenGLActor *m_TubeActor;
    vtkOpenGLPolyDataMapper *m_vtkTubeMapper;
    
    
    vtkPropAssembly *m_FiberAssembly;
    


  };

} // namespace mitk




#endif /* FiberBundleMapper3D_H_HEADER_INCLUDED */

