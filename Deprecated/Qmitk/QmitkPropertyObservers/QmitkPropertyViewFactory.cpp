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

#include "QmitkPropertyViewFactory.h"

// the different view and editor classes
#include <QmitkBasePropertyView.h>
#include <QmitkBoolPropertyView.h>
#include <QmitkBoolPropertyEditor.h>
#include <QmitkStringPropertyView.h>
#include <QmitkStringPropertyEditor.h>
#include <QmitkStringPropertyOnDemandEdit.h>
#include <QmitkColorPropertyView.h>
#include <QmitkColorPropertyEditor.h>
#include <QmitkNumberPropertyView.h>
#include <QmitkNumberPropertyEditor.h>

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
QWidget* QmitkPropertyViewFactory::CreateView(const mitk::BaseProperty* property, unsigned int /*type*/, QWidget* parent, const char* name)
{
  if ( const mitk::StringProperty* prop = dynamic_cast<const mitk::StringProperty*>(property) )
  {
    // a string property
    return new QmitkStringPropertyView(prop, parent, name);
  }
  else if ( const mitk::ColorProperty* prop = dynamic_cast<const mitk::ColorProperty*>(property) )
  {
    // a color property
    return new QmitkColorPropertyView(prop, parent, name);
  }
  else if ( const mitk::BoolProperty* prop = dynamic_cast<const mitk::BoolProperty*>(property) )
  {
    // a bool property
    return new QmitkBoolPropertyView(prop, parent, name);
  }
  /*
  else if ( const mitk::GenericProperty<short>* prop = dynamic_cast<const mitk::GenericProperty<short>*>(property) )
  {
    // a number property
    return new QmitkNumberPropertyView(prop, parent, name);
  }
  */
  else if ( const mitk::IntProperty* prop = dynamic_cast<const mitk::IntProperty*>(property) )
  {
    // a number property
    return new QmitkNumberPropertyView(prop, parent, name);
  }
  else if ( const mitk::FloatProperty* prop = dynamic_cast<const mitk::FloatProperty*>(property) )
  {
    // a number property
    return new QmitkNumberPropertyView(prop, parent, name);
  }
  else if ( const mitk::DoubleProperty* prop = dynamic_cast<const mitk::DoubleProperty*>(property) )
  {
    // a number property
    return new QmitkNumberPropertyView(prop, parent, name);
  }
  else if ( property != NULL )
  {
    // some unknown property --> use the GetValueAsString() method to 
    return new QmitkBasePropertyView(prop, parent, name);
  }

  return NULL;
}

QWidget* QmitkPropertyViewFactory::CreateEditor(mitk::BaseProperty* property, unsigned int type, QWidget* parent, const char* name)
{
  if (!property) return NULL;

  if ( mitk::StringProperty* prop = dynamic_cast<mitk::StringProperty*>(property) )
  {
    switch (type)
    {
      case etON_DEMAND_EDIT:
        // a string property
        return new QmitkStringPropertyOnDemandEdit(prop, parent, name);
      default:
        // a string property
        return new QmitkStringPropertyEditor(prop, parent, name);
    }
  }
  else if ( mitk::ColorProperty* prop = dynamic_cast<mitk::ColorProperty*>(property) )
  {
    // a color property
    return new QmitkColorPropertyEditor(prop, parent, name);
  }
  else if ( mitk::BoolProperty* prop = dynamic_cast<mitk::BoolProperty*>(property) )
  {
    // a bool property
    return new QmitkBoolPropertyEditor(prop, parent, name);
  }
  /*
  else if ( mitk::GenericProperty<short>* prop = dynamic_cast<mitk::GenericProperty<short>*>(property) )
  {
    // a number property
    return new QmitkNumberPropertyEditor(prop, parent, name);
  }
  */
  else if ( mitk::IntProperty* prop = dynamic_cast<mitk::IntProperty*>(property) )
  {
    // a number property
    return new QmitkNumberPropertyEditor(prop, parent, name);
  }
  else if ( mitk::FloatProperty* prop = dynamic_cast<mitk::FloatProperty*>(property) )
  {
    // a number property
    QmitkNumberPropertyEditor* pe = new QmitkNumberPropertyEditor(prop, parent, name);
    pe->setDecimalPlaces(2);
    return pe;
  }
  else if ( mitk::DoubleProperty* prop = dynamic_cast<mitk::DoubleProperty*>(property) )
  {
    // a number property
    QmitkNumberPropertyEditor* pe = new QmitkNumberPropertyEditor(prop, parent, name);
    pe->setDecimalPlaces(2);
    return pe;
  }
  else
  {
    // some unknown property --> no editor possible
    return NULL;
  }
}

