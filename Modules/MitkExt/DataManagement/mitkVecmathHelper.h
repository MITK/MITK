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


#ifndef MITKVECHELPER_H_HEADER_INCLUDED_C1EBD0AD
#define MITKVECHELPER_H_HEADER_INCLUDED_C1EBD0AD

#include "mitkVector.h"
#include "vecmath.h"

namespace mitk {

//itk vs. vecmath conversion
template <class T>
  inline void vm2itk(const Point3<T>& p, Point3D& a)
{
  a[0]=p.x;
  a[1]=p.y;
  a[2]=p.z;
}

template <class T>
  inline void itk2vm(const Point3D& a, Point3<T> & p)
{
  p.set(a[0], a[1], a[2]);
}

template <class T>
  inline void vm2itk(const Vector3<T>& p, Vector3D& a)
{
  a[0]=p.x;
  a[1]=p.y;
  a[2]=p.z;
}

template <class T>
  inline void itk2vm(const Vector3D& a, Vector3<T> & p)
{
  p.set(a[0], a[1], a[2]);
}

template <class T>
  inline void vm2itk(const Vector3<T>& p, VnlVector& a)
{
  a[0]=p.x;
  a[1]=p.y;
  a[2]=p.z;
}

template <class T>
  inline void itk2vm(const VnlVector& a, Vector3<T> & p)
{
  p.set(a[0], a[1], a[2]);
}

template <class Tin, class Tout>
  inline void vtk2vm(const Tin a[3], Vector3<Tout> & p)
{
  p.set(a[0], a[1], a[2]);
}

template <class Tin, class Tout>
  inline void vm2vtk(const Vector3<Tin> & p, Tout a[3])
{
  a[0]=p.x;
  a[1]=p.y;
  a[2]=p.z;
}

template <class Tin, class Tout>
  inline void vtk2vm(const Tin a[3], Point3<Tout> & p)
{
  p.set(a[0], a[1], a[2]);
}

template <class Tin, class Tout>
  inline void vm2vtk(const Point3<Tin> & p, Tout a[3])
{
  a[0]=p.x;
  a[1]=p.y;
  a[2]=p.z;
}

} // namespace mitk


#endif /* MITKVECHELPER */
