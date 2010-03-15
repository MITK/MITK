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


#ifndef COMPOSITEMAPPER_H_HEADER_INCLUDED
#define COMPOSITEMAPPER_H_HEADER_INCLUDED

#include "mitkGLMapper2D.h"
#include "mitkBaseVtkMapper2D.h"
#include "mitkQBallImage.h"
#include "mitkImageMapper2D.h"
#include "mitkOdfVtkMapper2D.h"

namespace mitk {

  class CopyImageMapper2D : public ImageMapper2D
  {
  public:
    mitkClassMacro(CopyImageMapper2D,ImageMapper2D);
    itkNewMacro(Self);

    friend class CompositeMapper;
  };

  //##Documentation
  //## @brief Composite pattern for combination of different mappers
  //## @ingroup Mapper
  class CompositeMapper : public BaseVtkMapper2D
  {
  public:

    mitkClassMacro(CompositeMapper,BaseVtkMapper2D);
    itkNewMacro(Self);

    virtual void MitkRenderOverlay(BaseRenderer* renderer)
    {
      Enable2DOpenGL();
      m_ImgMapper->MitkRenderOverlay(renderer);
      Disable2DOpenGL();
      m_OdfMapper->MitkRenderOverlay(renderer);
    }

    virtual void MitkRenderOpaqueGeometry(BaseRenderer* renderer)
    {
      Enable2DOpenGL();
      m_ImgMapper->MitkRenderOpaqueGeometry(renderer);
      Disable2DOpenGL();
      m_OdfMapper->MitkRenderOpaqueGeometry(renderer);
      if( mitk::RenderingManager::GetInstance()->GetNextLOD( renderer ) == 0 )
      {
        renderer->Modified();
      }
    }

    virtual void MitkRenderTranslucentGeometry(BaseRenderer* renderer)
    {
      Enable2DOpenGL();
      m_ImgMapper->MitkRenderTranslucentGeometry(renderer);
      Disable2DOpenGL();
      m_OdfMapper->MitkRenderTranslucentGeometry(renderer);
    }

#if ( ( VTK_MAJOR_VERSION >= 5 ) && ( VTK_MINOR_VERSION>=2)  )
    virtual void MitkRenderVolumetricGeometry(BaseRenderer* renderer)
    {
      Enable2DOpenGL();
      m_ImgMapper->MitkRenderVolumetricGeometry(renderer);
      Disable2DOpenGL();
      m_OdfMapper->MitkRenderVolumetricGeometry(renderer);
    }
#endif

    void SetDataNode(DataNode* node)
    {
      m_DataNode = node;
      m_ImgMapper->SetDataNode(node);
      m_OdfMapper->SetDataNode(node);
    }

    bool IsVtkBased() const
    {
      return m_OdfMapper->IsVtkBased();
    }

    bool HasVtkProp( const vtkProp* prop, BaseRenderer* renderer )
    { 
      return m_OdfMapper->HasVtkProp(prop, renderer);
    }

    void ReleaseGraphicsResources(vtkWindow* window)
    {
      m_ImgMapper->ReleaseGraphicsResources(window);
      m_OdfMapper->ReleaseGraphicsResources(window);
    }

    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = NULL, bool overwrite = false )
    {
      mitk::OdfVtkMapper2D<float,QBALL_ODFSIZE>::SetDefaultProperties(node, renderer, overwrite);
      mitk::CopyImageMapper2D::SetDefaultProperties(node, renderer, overwrite);
    }

    bool IsLODEnabled( BaseRenderer * renderer ) const 
    {
      return m_ImgMapper->IsLODEnabled(renderer) || m_OdfMapper->IsLODEnabled(renderer);
    }

    vtkProp* GetProp(mitk::BaseRenderer* renderer)
    {
      return m_OdfMapper->GetProp(renderer);
    }

    void SetGeometry3D(const mitk::Geometry3D* aGeometry3D)
    {
      m_ImgMapper->SetGeometry3D(aGeometry3D);
      m_OdfMapper->SetGeometry3D(aGeometry3D);
    }

    void Enable2DOpenGL();
    void Disable2DOpenGL();

  protected:

    virtual void GenerateData()
    {
      m_OdfMapper->GenerateData();
    }

    virtual void GenerateData(mitk::BaseRenderer* renderer)
    {
      m_ImgMapper->GenerateData(renderer);
      if( mitk::RenderingManager::GetInstance()->GetNextLOD( renderer ) > 0 )
      {
        m_OdfMapper->GenerateData(renderer);
      }
    }

    CompositeMapper();

    virtual ~CompositeMapper();

  private:

    mitk::OdfVtkMapper2D<float,QBALL_ODFSIZE>::Pointer m_OdfMapper;
    mitk::CopyImageMapper2D::Pointer m_ImgMapper;
  };

} // namespace mitk



#endif /* COMPOSITEMAPPER_H_HEADER_INCLUDED */

