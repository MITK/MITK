/*=========================================================================

Program:   Visualization Toolkit
Module:    $RCSfile: vtkPlane.cxx,v $

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkThickPlane.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkThickPlane, "$Revision: 1.41 $");
vtkStandardNewMacro(vtkThickPlane);

// Construct plane passing through origin and normal to z-axis.
vtkThickPlane::vtkThickPlane()
{
  this->Normal[0] = 0.0;
  this->Normal[1] = 0.0;
  this->Normal[2] = 1.0;

  this->Origin[0] = 0.0;
  this->Origin[1] = 0.0;
  this->Origin[2] = 0.0;
}

// Evaluate plane equation for point x[3].
double vtkThickPlane::EvaluateFunction(double x[3])
{
  return this->EvaluateFunction(x[0],x[1],x[2]);
}

// Evaluate plane equation for point x,y,z.
double vtkThickPlane::EvaluateFunction(double x,double y,double z)
{
  // PointPlaneDistance = Distance - Normal*Point
  double ppd = Distance - ( this->Normal[0]*x + 
    this->Normal[1]*y + 
    this->Normal[2]*z );

  if( abs(ppd) <= Thickness )
  {
    count++;
    return 0;
  }

  if( ppd >= 0 )
    return ppd - Thickness;
  
  return abs(ppd + Thickness);
}

// Evaluate function gradient at point x[3].
void vtkThickPlane::EvaluateGradient(double vtkNotUsed(x)[3], double n[3])
{
  for (int i=0; i<3; i++)
  {
    n[i] = this->Normal[i];
  }
}

void vtkThickPlane::SetPose (double _n1, double _n2, double _n3, double _o1, double _o2, double _o3)
{
  SetNormal(_n1,_n2,_n3);
  SetOrigin(_o1,_o2,_o3);
  Distance = Normal[0]*Origin[0]+Normal[1]*Origin[1]+Normal[2]*Origin[2];
  if(Distance < 0.0)
  {
    Distance = -Distance;
    Normal[0] = -Normal[0];
    Normal[1] = -Normal[1];
    Normal[2] = -Normal[2];
  }
}

void vtkThickPlane::SetPose (double _n[3], double _o[3])
{
  SetPose(_n[0],_n[1],_n[2],_o[0],_o[1],_o[2]);
}

void vtkThickPlane::SetNormal (double _arg1, double _arg2, double _arg3)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting " << Normal << " to (" << _arg1 << "," << _arg2 << "," << _arg3 << ")");
  if ((this->Normal[0] != _arg1)||(this->Normal[1] != _arg2)||(this->Normal[2] != _arg3))
  {
    double length = sqrt(_arg1*_arg1+_arg2*_arg2+_arg3*_arg3);
    this->Normal[0] = _arg1/length;
    this->Normal[1] = _arg2/length;
    this->Normal[2] = _arg3/length;
    this->Modified();
  }
};

void vtkThickPlane::SetNormal (double _arg[3])
{
  this->SetNormal (_arg[0], _arg[1], _arg[2]);
} 

void vtkThickPlane::SetOrigin (double _arg1, double _arg2, double _arg3)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting " << Origin << " to (" << _arg1 << "," << _arg2 << "," << _arg3 << ")");
  if ((this->Normal[0] != _arg1)||(this->Normal[1] != _arg2)||(this->Normal[2] != _arg3))
  {
    this->Origin[0] = _arg1;
    this->Origin[1] = _arg2;
    this->Origin[2] = _arg3;
    this->Modified();
  }
};

void vtkThickPlane::SetOrigin (double _arg[3])
{
  this->SetOrigin (_arg[0], _arg[1], _arg[2]);
} 

void vtkThickPlane::SetThickness (double _arg)
{
  Thickness = abs(_arg);
}

#define VTK_PLANE_TOL 1.0e-06

void vtkThickPlane::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Normal: (" << this->Normal[0] << ", " 
    << this->Normal[1] << ", " << this->Normal[2] << ")\n";

  os << indent << "Origin: (" << this->Origin[0] << ", " 
    << this->Origin[1] << ", " << this->Origin[2] << ")\n";

  os << indent << "Thickness: " << this->Thickness << "\n";
}
