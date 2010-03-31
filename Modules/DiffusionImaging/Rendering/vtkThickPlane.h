/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

// .NAME vtkPlane - perform various plane computations
// .SECTION Description
// vtkPlane provides methods for various plane computations. These include
// projecting points onto a plane, evaluating the plane equation, and 
// returning plane normal. vtkPlane is a concrete implementation of the 
// abstract class vtkImplicitFunction.

#ifndef __vtkThickPlane_h
#define __vtkThickPlane_h

#include "MitkDiffusionImagingExports.h"

#include "vtkImplicitFunction.h"
#include "mitkCommon.h"

class MitkDiffusionImaging_EXPORT vtkThickPlane : public vtkImplicitFunction
{
public:
  // Description
  // Construct plane passing through origin and normal to z-axis.
  static vtkThickPlane *New();

  vtkTypeRevisionMacro(vtkThickPlane,vtkImplicitFunction);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description
  // Evaluate plane equation for point x[3].
  double EvaluateFunction(double x[3]);
  double EvaluateFunction(double x, double y, double z);

  // Description
  // Evaluate function gradient at point x[3].
  void EvaluateGradient(double x[3], double g[3]);

  // Description:
  // Set/get plane normal. Plane is defined by point and normal.
  virtual void SetPose (double _n1, double _n2, double _n3, double _o1, double _o2, double _o3);
  virtual void SetPose (double _n[3], double _o[3]);
  
  vtkGetVectorMacro(Normal,double,3);
  vtkGetVectorMacro(Origin,double,3);

  // Description:
  // Set/get point through which plane passes. Plane is defined by point 
  // and normal.
  virtual void SetThickness (double _arg);
  vtkGetMacro(Thickness,double);

  int count;

protected:
  vtkThickPlane();
  ~vtkThickPlane() {};

  // Description:
  // Set/get plane normal. Plane is defined by point and normal.
  virtual void SetNormal (double _arg1, double _arg2, double _arg3);
  virtual void SetNormal (double _arg[3]);
  
  // Description:
  // Set/get point through which plane passes. Plane is defined by point 
  // and normal.
  virtual void SetOrigin (double _arg1, double _arg2, double _arg3);
  virtual void SetOrigin (double _arg[3]);

  double Normal[3];
  double Origin[3];
  double Thickness;
  double Distance;

private:
  vtkThickPlane(const vtkThickPlane&);  // Not implemented.
  void operator=(const vtkThickPlane&);  // Not implemented.
};

#endif



