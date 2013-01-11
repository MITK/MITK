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


#ifndef COMPOSITEMAPPER_H_HEADER_INCLUDED
#define COMPOSITEMAPPER_H_HEADER_INCLUDED

#include "mitkGLMapper2D.h"
#include "mitkVtkMapper2D.h"
#include "mitkQBallImage.h"
#include "mitkImageVtkMapper2D.h"
#include "mitkOdfVtkMapper2D.h"
#include "mitkLevelWindowProperty.h"

namespace mitk {

  class CopyImageMapper2D : public ImageVtkMapper2D
  {
  public:
    mitkClassMacro(CopyImageMapper2D,ImageVtkMapper2D);
    itkNewMacro(Self);

    friend class CompositeMapper;
  };

  //##Documentation
  //## @brief Composite pattern for combination of different mappers
  //## @ingroup Mapper
  class CompositeMapper : public VtkMapper2D
  {
  public:

    mitkClassMacro(CompositeMapper,VtkMapper2D)
    itkNewMacro(Self)

    virtual void MitkRenderOverlay(BaseRenderer* renderer)
    {
      m_ImgMapper->MitkRenderOverlay(renderer);
      m_OdfMapper->MitkRenderOverlay(renderer);
    }

    virtual void MitkRenderOpaqueGeometry(BaseRenderer* renderer)
    {
      m_ImgMapper->MitkRenderOpaqueGeometry(renderer);
      m_OdfMapper->MitkRenderOpaqueGeometry(renderer);
      if( mitk::RenderingManager::GetInstance()->GetNextLOD( renderer ) == 0 )
      {
        renderer->Modified();
      }
    }

    virtual void MitkRenderTranslucentGeometry(BaseRenderer* renderer)
    {
      m_ImgMapper->MitkRenderTranslucentGeometry(renderer);
      m_OdfMapper->MitkRenderTranslucentGeometry(renderer);
    }

    virtual void MitkRenderVolumetricGeometry(BaseRenderer* renderer)
    {
      m_ImgMapper->MitkRenderVolumetricGeometry(renderer);
      m_OdfMapper->MitkRenderVolumetricGeometry(renderer);
    }

    void SetDataNode(DataNode* node)
    {
      m_DataNode = node;
      m_ImgMapper->SetDataNode(node);
      m_OdfMapper->SetDataNode(node);
    }

    mitk::ImageVtkMapper2D::Pointer GetImageMapper()
    {
      ImageVtkMapper2D* retval = m_ImgMapper;
      return retval;
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

      mitk::LevelWindow opaclevwin;
      opaclevwin.SetRangeMinMax(0,255);
      opaclevwin.SetWindowBounds(0,0);
      mitk::LevelWindowProperty::Pointer prop = mitk::LevelWindowProperty::New(opaclevwin);
      node->AddProperty( "opaclevelwindow", prop );
    }

    bool IsLODEnabled( BaseRenderer * renderer ) const
    {
      return m_ImgMapper->IsLODEnabled(renderer) || m_OdfMapper->IsLODEnabled(renderer);
    }

    vtkProp* GetVtkProp(mitk::BaseRenderer* renderer)
    {
      vtkPropAssembly* assembly = vtkPropAssembly::New();
      assembly->AddPart( m_OdfMapper->GetVtkProp(renderer));
      assembly->AddPart( m_ImgMapper->GetVtkProp(renderer));
      return assembly;
    }

    void SetGeometry3D(const mitk::Geometry3D* aGeometry3D)
    {
      m_ImgMapper->SetGeometry3D(aGeometry3D);
      m_OdfMapper->SetGeometry3D(aGeometry3D);
    }

    virtual void Update(mitk::BaseRenderer* renderer)
    {
      m_OdfMapper->Update(renderer);
      GenerateDataForRenderer(renderer);
    }

  protected:

    virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer)
    {
      m_ImgMapper->GenerateDataForRenderer(renderer);
//      if( mitk::RenderingManager::GetInstance()->GetNextLOD( renderer ) > 0 )
//      {
//        m_OdfMapper->GenerateDataForRenderer(renderer);
//      }
    }

    CompositeMapper();

    virtual ~CompositeMapper();

  private:

    mitk::OdfVtkMapper2D<float,QBALL_ODFSIZE>::Pointer m_OdfMapper;
    mitk::CopyImageMapper2D::Pointer m_ImgMapper;

  };

} // namespace mitk



#endif /* COMPOSITEMAPPER_H_HEADER_INCLUDED */

