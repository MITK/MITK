/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#ifndef IMAGEVTKMAPPER2D_H_HEADER_INCLUDED_C1C5453B
#define IMAGEVTKMAPPER2D_H_HEADER_INCLUDED_C1C5453B

//MITK includes
#include "mitkVtkMapper2D.h"
#include <mitkImage.h>

//VTK includes
#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkImageData.h>

namespace mitk {

  //##Documentation
  //## @brief Class to display 2D images

  class MITK_CORE_EXPORT ImageVtkMapper2D : public VtkMapper2D
  {
  public:
    mitkClassMacro(ImageVtkMapper2D,VtkMapper2D);

  protected:
    ImageVtkMapper2D();

    virtual ~ImageVtkMapper2D();
  private:
        vtkSmartPointer<vtkImageData> m_VtkImage;

//        mitk::Image* GetInput();
        vtkSmartPointer<vtkActor> m_VtkActor;
        vtkSmartPointer<vtkPolyDataMapper> m_VtkMapper;
  };

} // namespace mitk



#endif /* IMAGEVTKMAPPER2D_H_HEADER_INCLUDED_C1C5453B */
