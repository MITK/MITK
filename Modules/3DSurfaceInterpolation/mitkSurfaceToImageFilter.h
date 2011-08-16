/*=========================================================================
Program: Medical Imaging & Interaction Toolkit
Module: $RCSfile$
Language: C++
Date: $Date: $
Version: $Revision: $
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the above copyright notices for more information.
=========================================================================*/

#ifndef mitkSurfaceToImageFilter_h_Included
#define mitkSurfaceToImageFilter_h_Included

#include <mitk3DSurfaceInterpolationExports.h>
#include "mitkSurface.h"
#include "mitkImageSource.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "mitkSurface.h"
#include "vtkDoubleArray.h"
#include "vtkCellData.h"

namespace mitk {

  /**
  \brief 

  \ingroup Process

  $Author: fetzer$
  */
  class mitk3DSurfaceInterpolation_EXPORT SurfaceToImageFilter : public ImageSource
  {

  public:

    typedef std::vector<Surface::Pointer> SurfaceList;

    mitkClassMacro(SurfaceToImageFilter,ImageSource);
    itkNewMacro(Self);

    //Methods copied from mitkSurfaceToSurfaceFilter
    virtual void SetInput( const mitk::Surface* surface );

    virtual void SetInput( unsigned int idx, const mitk::Surface* surface );

    virtual const mitk::Surface* GetInput();

    virtual const mitk::Surface* GetInput( unsigned int idx );

    virtual void RemoveInputs(mitk::Surface* input);

  protected:

    SurfaceToImageFilter();
    virtual ~SurfaceToImageFilter();

  };

  }//namespace


#endif