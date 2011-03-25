/*=========================================================================
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef IMAGEVTKMAPPER2D_H_HEADER_INCLUDED_C1C5453B
#define IMAGEVTKMAPPER2D_H_HEADER_INCLUDED_C1C5453B

//MITK includes
#include "mitkVtkMapper2D.h"
#include <mitkImage.h>

//VTK includes
#include <vtkSmartPointer.h>
#include <vtkImageActor.h>
#include <vtkActor2D.h>
#include <vtkPolyDataMapper.h>
#include <vtkImageData.h>
#include <vtkMapper2D.h>
#include <vtkImageMapper.h>
#include <vtkOpenGLImageMapper.h>
#include <vtkImage.h>
#include <vtkOpenGLImageResliceMapper.h>


namespace mitk {

  //##Documentation
  //## @brief Class to display 2D images

  class MITK_CORE_EXPORT ImageVtkMapper2D : public VtkMapper2D
  {
  public:
    mitkClassMacro(ImageVtkMapper2D,VtkMapper2D);
    itkNewMacro(Self);
    virtual const mitk::Image* GetInput();

    virtual void MitkRenderOverlay(BaseRenderer* renderer);
    virtual void MitkRenderOpaqueGeometry(BaseRenderer* renderer);
    virtual void MitkRenderTranslucentGeometry(BaseRenderer* renderer);
    virtual void MitkRenderVolumetricGeometry(BaseRenderer* renderer) ;

    virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer);

  protected:
    ImageVtkMapper2D();

    virtual ~ImageVtkMapper2D();

    virtual void GenerateData(BaseRenderer* renderer);
  private:
    vtkSmartPointer<vtkImageData> m_VtkImage;

    vtkSmartPointer<vtkImage> m_VtkActor;
    vtkSmartPointer<vtkOpenGLImageResliceMapper> m_VtkMapper;

    int m_TimeStep;
    bool m_VtkBased;

    vtkImageData *GenerateTestImageForTSFilter();
  };

} // namespace mitk



#endif /* IMAGEVTKMAPPER2D_H_HEADER_INCLUDED_C1C5453B */
