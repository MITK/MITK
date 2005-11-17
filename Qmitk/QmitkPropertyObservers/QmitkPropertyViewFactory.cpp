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

QmitkPropertyViewFactory* QmitkPropertyViewFactory::GetInstance()
{
  static QmitkPropertyViewFactory instance;
  return &instance;
}

QmitkPropertyViewFactory::QmitkPropertyViewFactory()
{
}

QmitkPropertyViewFactory::~QmitkPropertyViewFactory()
{
}

QWidget* QmitkPropertyViewFactory::CreateView(mitk::BaseProperty* property, unsigned int type = 0, QWidget* parent = 0, const char* name = 0)
{
}

QWidget* QmitkPropertyViewFactory::CreateEditor(mitk::BaseProperty* property, unsigned int type = 0, QWidget* parent = 0, const char* name = 0)
{
}

