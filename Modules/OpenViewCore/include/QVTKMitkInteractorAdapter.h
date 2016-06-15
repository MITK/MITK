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

// original copyright below

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    QVTKMitkInteractorAdapter.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

/*=========================================================================

  Copyright 2004 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
  license for use of this work by or on behalf of the
  U.S. Government. Redistribution and use in source and binary forms, with
  or without modification, are permitted provided that this Notice and any
  statement of authorship are reproduced on all copies.

=========================================================================*/

/*========================================================================
 For general information about using VTK and Qt, see:
 http://www.trolltech.com/products/3rdparty/vtksupport.html
=========================================================================*/

// .NAME QVTKMitkInteractorAdapter - Handle Qt events.
// .SECTION Description
// QVTKInteractor handles relaying Qt events to VTK.

#ifndef Q_VTK_MITK_INTERACTOR_ADAPTER_H
#define Q_VTK_MITK_INTERACTOR_ADAPTER_H

#include <QtCore/QObject>

#include <MitkOpenViewCoreExports.h>

class vtkRenderWindowInteractor;
class QEvent;

// .NAME QVTKMitkInteractorAdapter - A QEvent translator.
// .SECTION Description
// QVTKMitkInteractorAdapter translates QEvents and send them to a
// vtkRenderWindowInteractor.
class MITKOPENVIEWCORE_EXPORT QVTKMitkInteractorAdapter : public QObject
{
  Q_OBJECT
public:
  // Description:
  // Constructor: takes QObject parent
  QVTKMitkInteractorAdapter(QObject* parent);

  // Description:
  // Destructor
  ~QVTKMitkInteractorAdapter();

  // Description:
  // Process a QEvent and send it to the interactor
  // returns whether the event was recognized and processed
  bool ProcessEvent(QEvent* e, vtkRenderWindowInteractor* iren);
};

#endif
