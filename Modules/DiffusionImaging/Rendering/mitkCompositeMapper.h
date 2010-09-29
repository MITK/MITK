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
#include "mitkVtkMapper2D.h"
#include "mitkQBallImage.h"
#include "mitkImageMapperGL2D.h"
#include "mitkOdfVtkMapper2D.h"
#include "mitkLevelWindowProperty.h"

namespace mitk {

  class CopyImageMapper2D : public ImageMapperGL2D
  {
  public:
    mitkClassMacro(CopyImageMapper2D,ImageMapperGL2D);
    itkNewMacro(Self);

    friend class CompositeMapper;
  };

  //##Documentation
  //## @brief Composite pattern for combination of different mappers
  //## @ingroup Mapper
  class CompositeMapper : public VtkMapper2D
  {
  public:

    mitkClassMacro(CompositeMapper,VtkMapper2D);
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

      node->AddProperty( "opacity min fa", mitk::FloatProperty::New( 0.0 ), renderer, overwrite );
      node->AddProperty( "opacity max fa", mitk::FloatProperty::New( 0.0 ), renderer, overwrite );
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
      float min;
      bool success = m_ImgMapper->GetDataNode()->GetFloatProperty("opacity min fa", min);

      float max;
      success = success && m_ImgMapper->GetDataNode()->GetFloatProperty("opacity max fa", max);

      if(success)
      {
        if (max < min)
        {
          max = min;
        }

        float level = (min+max)/2;
        float window = max-min;

        mitk::TensorImage::Pointer timg = dynamic_cast<mitk::TensorImage*>(m_ImgMapper->GetDataNode()->GetData());
        if(timg.IsNotNull())
        {
          timg->UpdateInternalRGBAImage(level, window);
        }

        mitk::QBallImage::Pointer qimg = dynamic_cast<mitk::QBallImage*>(m_ImgMapper->GetDataNode()->GetData());
        if(qimg.IsNotNull())
        {
          qimg->UpdateInternalRGBAImage(level, window);
        }
      }
      else
      {
        MITK_ERROR << "opacity level window property not set";
      }

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

