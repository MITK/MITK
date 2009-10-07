/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-08 13:23:19 +0100 (Fr, 08 Feb 2008) $
Version:   $Revision: 13561 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef ODFVTKMAPPER2D_H_HEADER_INCLUDED
#define ODFVTKMAPPER2D_H_HEADER_INCLUDED

#include "mitkBaseVtkMapper2D.h"
#include "vtkPropAssembly.h"
#include "vtkAppendPolyData.h"
#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkPlane.h"
#include "vtkCutter.h"
#include "vtkClipPolyData.h"
#include "vtkTransform.h"
#include "vtkDataArrayTemplate.h"

#include "vtkOdfSource.h"
#include "vtkThickPlane.h"

//#include "mitkTrackingCameraController.h"

namespace mitk {

  //##Documentation
  //## @brief Base class of all vtk-based 2D-Mappers
  //##
  //## Those must implement the abstract
  //## method vtkProp* GetProp().
  //## @ingroup Mapper
  template<class TPixelType, int NrOdfDirections>
  class OdfVtkMapper2D : public BaseVtkMapper2D
  {
  public:

    mitkClassMacro(OdfVtkMapper2D,BaseVtkMapper2D);
    itkNewMacro(Self);

    virtual vtkProp* GetProp(mitk::BaseRenderer* renderer);

    virtual void MitkRenderOverlay(mitk::BaseRenderer* renderer);
    virtual void MitkRenderOpaqueGeometry(mitk::BaseRenderer* renderer);
    virtual void MitkRenderTranslucentGeometry(mitk::BaseRenderer* renderer);

#if ( ( VTK_MAJOR_VERSION >= 5 ) && ( VTK_MINOR_VERSION>=2)  )
    virtual void MitkRenderVolumetricGeometry(mitk::BaseRenderer* renderer){};
#endif

    virtual void Slice(mitk::BaseRenderer* renderer);
    virtual int GetIndex(mitk::BaseRenderer* renderer);

    static void SetDefaultProperties(DataTreeNode* node, BaseRenderer* renderer = NULL, bool overwrite = false);

  protected:
    OdfVtkMapper2D();

    virtual ~OdfVtkMapper2D();

    static void GlyphMethod(void *arg);

  private:

    std::vector<vtkPropAssembly*> m_PropAssemblies;
    std::vector<vtkAppendPolyData*> m_OdfsPlanes;
    std::vector<vtkActor*> m_OdfsActors;    
    std::vector<vtkPolyDataMapper*> m_OdfsMappers;
    vtkPolyData* m_TemplateOdf;

    static vtkTransform* m_OdfTransform;
    static vtkDoubleArray* m_OdfVals;
    static vtkOdfSource* m_OdfSource;
    static float m_Scaling;
    static float m_GlobalScaling;

    int m_ShowMaxNumber;

    //std::vector<mitk::TrackingCameraController::Pointer> m_TrackingCameraControllers;

    std::vector<vtkPlane*> m_Planes;
    std::vector<vtkCutter*> m_Cutters;

    std::vector<vtkThickPlane*> m_ThickPlanes1;
    std::vector<vtkClipPolyData *> m_Clippers1;
    
    std::vector<vtkThickPlane*> m_ThickPlanes2;
    std::vector<vtkClipPolyData *> m_Clippers2;

    vtkImageData* m_VtkImage ;

    virtual void GenerateData();
    virtual void GenerateData(mitk::BaseRenderer* renderer);

    const mitk::Image* GetInput();
  };

} // namespace mitk

#include "mitkOdfVtkMapper2D.txx"

#endif /* ODFVTKMAPPER2D_H_HEADER_INCLUDED */
