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
#include "vtkSmartPointer.h"
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
    struct OdfDisplayGeometry {
      vtkFloatingPointType vp[ 3 ], vnormal[ 3 ];
      Vector3D normal;
      double d, d1, d2;
      mitk::Point3D M3D, L3D, O3D;
      
      vtkFloatingPointType vp_original[ 3 ], vnormal_original[ 3 ];
      mitk::Vector2D size, origin;

      bool Equals(OdfDisplayGeometry* other)
      {
        return other->vp_original[0] == vp[0] &&
        other->vp_original[1] == vp[1] &&
        other->vp_original[2] == vp[2] &&
        other->vnormal_original[0] == vnormal[0] &&
        other->vnormal_original[1] == vnormal[1] &&
        other->vnormal_original[2] == vnormal[2] &&
        other->size[0] == size[0] &&
        other->size[1] == size[1] &&
        other->origin[0] == origin[0] &&
        other->origin[1] == origin[1];
      }
    };

  public:

    mitkClassMacro(OdfVtkMapper2D,BaseVtkMapper2D);
    itkNewMacro(Self);

    virtual vtkProp* GetProp(mitk::BaseRenderer* renderer);

    virtual void MitkRenderOverlay(mitk::BaseRenderer* renderer);
    virtual void MitkRenderOpaqueGeometry(mitk::BaseRenderer* renderer);
    virtual void MitkRenderTranslucentGeometry(mitk::BaseRenderer* renderer);

#if ( ( VTK_MAJOR_VERSION >= 5 ) && ( VTK_MINOR_VERSION>=2)  )
    virtual void MitkRenderVolumetricGeometry(mitk::BaseRenderer*  /*renderer*/){};
#endif
    
    OdfDisplayGeometry* MeasureDisplayedGeometry(mitk::BaseRenderer* renderer);
    void AdaptCameraPosition(mitk::BaseRenderer* renderer, OdfDisplayGeometry* dispGeo );
    void AdaptOdfScalingToImageSpacing( int index );
    void SetRendererLightSources( mitk::BaseRenderer *renderer );
    void ApplyPropertySettings();
    virtual void Slice(mitk::BaseRenderer* renderer,
      OdfDisplayGeometry* dispGeo);
    virtual int GetIndex(mitk::BaseRenderer* renderer);

    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = NULL, bool overwrite = false);

    virtual void GenerateData();
    virtual void GenerateData(mitk::BaseRenderer* renderer);

    virtual bool IsLODEnabled( BaseRenderer * /*renderer*/ ) const { return true; }

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

    static vtkSmartPointer<vtkTransform> m_OdfTransform;
    static vtkSmartPointer<vtkDoubleArray> m_OdfVals;
    static vtkSmartPointer<vtkOdfSource> m_OdfSource;
    static float m_Scaling;
    static int m_Normalization;
    static int m_ScaleBy;
    static float m_IndexParam1;
    static float m_IndexParam2;

    int m_ShowMaxNumber;

    //std::vector<mitk::TrackingCameraController::Pointer> m_TrackingCameraControllers;

    std::vector<vtkPlane*> m_Planes;
    std::vector<vtkCutter*> m_Cutters;

    std::vector<vtkThickPlane*> m_ThickPlanes1;
    std::vector<vtkClipPolyData *> m_Clippers1;
    
    std::vector<vtkThickPlane*> m_ThickPlanes2;
    std::vector<vtkClipPolyData *> m_Clippers2;

    vtkImageData* m_VtkImage ;

    mitk::Image* GetInput();

    OdfDisplayGeometry* m_LastDisplayGeometry;
  };

} // namespace mitk

#include "mitkOdfVtkMapper2D.txx"

#endif /* ODFVTKMAPPER2D_H_HEADER_INCLUDED */
