/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

class SplineVtkMapper3D : public PointSetVtkMapper3D
{
public:

    mitkClassMacro( SplineVtkMapper3D, PointSetVtkMapper3D );

    itkNewMacro( Self );

    virtual vtkProp* GetProp();

    
    bool SplinesAreAvailable();
    
    vtkPolyData* GetSplinesPolyData();
    
    vtkPolyData* GetPointsPolyData();
    
    vtkActor* GetSplinesActor();
    
    vtkActor* GetPointsActor();
    
    
protected:

    SplineVtkMapper3D();

    virtual ~SplineVtkMapper3D();

    virtual void GenerateData();

    virtual void GenerateData(mitk::BaseRenderer * renderer);
    
    vtkActor* m_SplinesActor;
    
    vtkAssembly* m_Assembly;
    
    bool m_SplinesAvailable;
    
    bool m_SplinesAddedToAssembly;
    
};


} //namespace mitk


#endif

