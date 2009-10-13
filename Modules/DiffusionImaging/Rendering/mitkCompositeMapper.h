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
  class CompositeMapper : public GLMapper2D, public BaseVtkMapper2D
  {
  public:

    typedef CompositeMapper                 Self;
    typedef itk::SmartPointer<Self>         Pointer;
    typedef itk::SmartPointer<const Self>   ConstPointer;
    virtual const char *GetNameOfClass() const
    {return "CompositeMapper";}

    itkNewMacro(Self);

    virtual void MitkRenderOverlay(BaseRenderer* renderer)
    {
      m_ImgMapper->MitkRenderOverlay(renderer);
      m_OdfMapper->MitkRenderOverlay(renderer);
    }

    virtual void MitkRenderOpaqueGeometry(BaseRenderer* renderer)
    {
      m_ImgMapper->MitkRenderOpaqueGeometry(renderer);
      m_OdfMapper->MitkRenderOpaqueGeometry(renderer);
    }

    virtual void MitkRenderTranslucentGeometry(BaseRenderer* renderer)
    {
      m_ImgMapper->MitkRenderTranslucentGeometry(renderer);
      m_OdfMapper->MitkRenderTranslucentGeometry(renderer);
    }

#if ( ( VTK_MAJOR_VERSION >= 5 ) && ( VTK_MINOR_VERSION>=2)  )
    virtual void MitkRenderVolumetricGeometry(BaseRenderer* renderer)
    {
      m_ImgMapper->MitkRenderVolumetricGeometry(renderer);
      m_OdfMapper->MitkRenderVolumetricGeometry(renderer);
    }
#endif

    void Paint(mitk::BaseRenderer *renderer)
    {
      m_ImgMapper->Paint(renderer);
    }

    void SetDataTreeNode(DataTreeNode* node)
    {
      m_DataTreeNode = node;
      m_ImgMapper->SetDataTreeNode(node);
      m_OdfMapper->SetDataTreeNode(node);
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

    void SetDefaultProperties(DataTreeNode* node, BaseRenderer* renderer, bool overwrite )
    {
      m_ImgMapper->SetDefaultProperties(node, renderer, overwrite);
      m_OdfMapper->SetDefaultProperties(node, renderer, overwrite);
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

  protected:

    virtual void GenerateData()
    {
      //m_ImgMapper->GenerateData();
      m_OdfMapper->GenerateData();
    }

    virtual void GenerateData(mitk::BaseRenderer* renderer)
    {
      m_ImgMapper->GenerateData(renderer);
      m_OdfMapper->GenerateData(renderer);
    }

    CompositeMapper();

    virtual ~CompositeMapper();

  private:

    mitk::OdfVtkMapper2D<float,QBALL_ODFSIZE>::Pointer m_OdfMapper;
    mitk::CopyImageMapper2D::Pointer m_ImgMapper;
  };

} // namespace mitk



#endif /* COMPOSITEMAPPER_H_HEADER_INCLUDED */
