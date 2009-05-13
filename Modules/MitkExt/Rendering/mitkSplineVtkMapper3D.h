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


#ifndef _MITK_SPLINE_VTK_MAPPER_3D__H
#define _MITK_SPLINE_VTK_MAPPER_3D__H

#include "mitkPointSetVtkMapper3D.h"

class vtkActor;
class vtkAssembly;

namespace mitk
{
  //##Documentation
  //## @brief Vtk-based mapper for Splines
  //##
  //## properties looked for:
  //## "line width" = with of the spline
  //## @ingroup Mapper

  /************************************************************************/
  /* 
  *   - \b "line width": (FloatProperty) line width of the spline
  
  */
  /************************************************************************/
class MITKEXT_CORE_EXPORT SplineVtkMapper3D : public PointSetVtkMapper3D
{
public:

    mitkClassMacro( SplineVtkMapper3D, PointSetVtkMapper3D );

    itkNewMacro( Self );

    virtual vtkProp* GetProp();
    virtual void UpdateVtkTransform();

    bool SplinesAreAvailable();
    
    vtkPolyData* GetSplinesPolyData();
    
    vtkActor* GetSplinesActor();
    
    unsigned long GetLastUpdateTime() const;
    
    virtual void UpdateSpline();
    
    itkSetMacro( SplineResolution, unsigned int );
    
    itkGetMacro( SplineResolution, unsigned int );
    
protected:

    SplineVtkMapper3D();

    virtual ~SplineVtkMapper3D();

    virtual void GenerateData();

    virtual void GenerateData(mitk::BaseRenderer * renderer);

    virtual void ApplyProperties();
    
    vtkActor* m_SplinesActor;
    
    vtkPropAssembly* m_SplineAssembly;
    
    bool m_SplinesAvailable;
    
    bool m_SplinesAddedToAssembly;
    
    unsigned int m_SplineResolution;
    
    itk::TimeStamp m_SplineUpdateTime;
};


} //namespace mitk


#endif


