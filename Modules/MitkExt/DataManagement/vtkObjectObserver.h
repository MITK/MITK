/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#ifndef VTKOBJECTOBSERVER_H
#define VTKOBJECTOBSERVER_H

class vtkObject;

class vtkObjectObserver
{
public:
  virtual void OnModified( const vtkObject* object ) = 0;
  virtual void OnDeleted( const vtkObject* object ) = 0;
};

#endif // VTKOBJECTOBSERVER_H
