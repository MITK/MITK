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


#ifndef BASEVTKMAPPER3D_H_HEADER_INCLUDED_C1CD0F0A
#define BASEVTKMAPPER3D_H_HEADER_INCLUDED_C1CD0F0A

#include "mitkCommon.h"
#include "mitkMapper.h"
#include "mitkMapper3D.h"
#include "mitkBaseRenderer.h"

class vtkProp;
class vtkProp3D;
class vtkActor;

namespace mitk {

//##ModelId=3E32C62B00BE
//##Documentation
//## @brief Base class of all vtk-based 3D-Mappers
//## @ingroup Mapper
//## Those must implement the abstract
//## method vtkProp* GetProp().
class BaseVtkMapper3D : public Mapper3D
{
public:
  mitkClassMacro(BaseVtkMapper3D, Mapper3D);

  //##ModelId=3E32C93D0000
  virtual vtkProp* GetProp();
  //##ModelId=3EF180C70065
  //##Documentation
  //## @brief Apply color and opacity read from the PropertyList
  virtual void ApplyProperties(vtkActor* actor, mitk::BaseRenderer* renderer);



protected:
  //##ModelId=3E3424950213
  BaseVtkMapper3D();

  //##ModelId=3E3424950231
  virtual ~BaseVtkMapper3D();

public:

  itkGetObjectMacro(Geometry,Geometry3D);
  itkSetObjectMacro(Geometry,Geometry3D);

  itkGetConstMacro(SliceNr,int);
  itkSetMacro(SliceNr,int);

  itkGetConstMacro(TimeNr,int);
  itkSetMacro(TimeNr,int);

  itkGetConstMacro(ChannelNr,int);
  itkSetMacro(ChannelNr,int);

protected:
  //##ModelId=3EDD039F0117
  Geometry3D::Pointer m_Geometry;
  //##ModelId=3ED91D050038
  LevelWindow m_LevelWindow;

  //##ModelId=3E3AE0DC023D
  int m_SliceNr;

  //##ModelId=3E33ECF203A1
  int m_TimeNr;

  //##ModelId=3E33ECF203B5
  int m_ChannelNr;

  //##ModelId=3ED91D050046
  vtkProp3D* m_Prop3D;
};

} // namespace mitk

#endif /* BASEVTKMAPPER3D_H_HEADER_INCLUDED_C1CD0F0A */
