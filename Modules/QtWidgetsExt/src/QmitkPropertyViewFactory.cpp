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

#include "QmitkPropertyViewFactory.h"

// the different view and editor classes
#include "QmitkBasePropertyView.h"
#include "QmitkBoolPropertyWidget.h"
#include "QmitkStringPropertyView.h"
#include "QmitkStringPropertyEditor.h"
#include "QmitkStringPropertyOnDemandEdit.h"
#include "QmitkColorPropertyView.h"
#include "QmitkColorPropertyEditor.h"
#include "QmitkNumberPropertyView.h"
#include "QmitkNumberPropertyEditor.h"
#include "QmitkEnumerationPropertyWidget.h"

#include <mitkEnumerationProperty.h>

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
QWidget* QmitkPropertyViewFactory::CreateView(const mitk::BaseProperty* property, unsigned int, QWidget* parent)
{
  if ( const mitk::StringProperty* prop = dynamic_cast<const mitk::StringProperty*>(property) )
  {
    // a string property
    return new QmitkStringPropertyView(prop, parent);
  }
  else if ( const mitk::ColorProperty* prop = dynamic_cast<const mitk::ColorProperty*>(property) )
  {
    // a color property
    return new QmitkColorPropertyView(prop, parent);
  }
  else if ( const mitk::BoolProperty* prop = dynamic_cast<const mitk::BoolProperty*>(property) )
  {
    // a bool property
    // TODO fix after refactoring
    QmitkBoolPropertyWidget* widget = new QmitkBoolPropertyWidget(parent);
    widget->SetProperty(const_cast<mitk::BoolProperty*>(prop));
    return widget;
  }
  else if ( const mitk::IntProperty* prop = dynamic_cast<const mitk::IntProperty*>(property) )
  {
    // a number property
    return new QmitkNumberPropertyView(prop, parent);
  }
  else if ( const mitk::FloatProperty* prop = dynamic_cast<const mitk::FloatProperty*>(property) )
  {
    // a number property
    return new QmitkNumberPropertyView(prop, parent);
  }
  else if ( const mitk::DoubleProperty* prop = dynamic_cast<const mitk::DoubleProperty*>(property) )
  {
    // a number property
    return new QmitkNumberPropertyView(prop, parent);
  }
  else if ( property != NULL )
  {
    // some unknown property --> use the GetValueAsString() method to
    return new QmitkBasePropertyView(prop, parent);
  }

  return NULL;
}

QWidget* QmitkPropertyViewFactory::CreateEditor(mitk::BaseProperty* property, unsigned int type, QWidget* parent)
{
  if (!property) return NULL;

  if ( mitk::StringProperty* prop = dynamic_cast<mitk::StringProperty*>(property) )
  {
    switch (type)
    {
      case etON_DEMAND_EDIT:
        // a string property
        return new QmitkStringPropertyOnDemandEdit(prop, parent);
      default:
        // a string property
        return new QmitkStringPropertyEditor(prop, parent);
    }
  }
  else if ( mitk::ColorProperty* prop = dynamic_cast<mitk::ColorProperty*>(property) )
  {
    // a color property
    return new QmitkColorPropertyEditor(prop, parent);
  }
  else if ( mitk::BoolProperty* prop = dynamic_cast<mitk::BoolProperty*>(property) )
  {
    // a bool property
    // TODO fix after refactoring
    QmitkBoolPropertyWidget* widget = new QmitkBoolPropertyWidget(parent);
    widget->SetProperty(prop);
    return widget;
  }
  else if ( mitk::IntProperty* prop = dynamic_cast<mitk::IntProperty*>(property) )
  {
    // a number property
    return new QmitkNumberPropertyEditor(prop, parent);
  }
  else if ( mitk::FloatProperty* prop = dynamic_cast<mitk::FloatProperty*>(property) )
  {
    // a number property
    QmitkNumberPropertyEditor* pe = new QmitkNumberPropertyEditor(prop, parent);
    pe->setDecimalPlaces(2);
    return pe;
  }
  else if ( mitk::DoubleProperty* prop = dynamic_cast<mitk::DoubleProperty*>(property) )
  {
    // a number property
    QmitkNumberPropertyEditor* pe = new QmitkNumberPropertyEditor(prop, parent);
    pe->setDecimalPlaces(2);
    return pe;
  }
  else if ( mitk::EnumerationProperty* prop = dynamic_cast<mitk::EnumerationProperty*>(property) )
  {
    // a enumeration property
    QmitkEnumerationPropertyWidget* pe = new QmitkEnumerationPropertyWidget(parent);
    pe->SetProperty(prop);
    return pe;
  }
  else
  {
    // some unknown property --> no editor possible
    return NULL;
  }
}

