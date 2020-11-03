/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkPropertyViewFactory.h"

// the different view and editor classes
#include "QmitkBasePropertyView.h"
#include "QmitkBoolPropertyWidget.h"
#include "QmitkColorPropertyEditor.h"
#include "QmitkColorPropertyView.h"
#include "QmitkEnumerationPropertyWidget.h"
#include "QmitkNumberPropertyEditor.h"
#include "QmitkNumberPropertyView.h"
#include "QmitkStringPropertyEditor.h"
#include "QmitkStringPropertyOnDemandEdit.h"
#include "QmitkStringPropertyView.h"

#include <mitkEnumerationProperty.h>

QmitkPropertyViewFactory *QmitkPropertyViewFactory::GetInstance()
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
QWidget *QmitkPropertyViewFactory::CreateView(const mitk::BaseProperty *property, unsigned int, QWidget *parent)
{
  if (const mitk::StringProperty *prop = dynamic_cast<const mitk::StringProperty *>(property))
  {
    // a string property
    return new QmitkStringPropertyView(prop, parent);
  }
  else if (const mitk::ColorProperty *prop = dynamic_cast<const mitk::ColorProperty *>(property))
  {
    // a color property
    return new QmitkColorPropertyView(prop, parent);
  }
  else if (const mitk::BoolProperty *prop = dynamic_cast<const mitk::BoolProperty *>(property))
  {
    // a bool property
    // TODO fix after refactoring
    auto widget = new QmitkBoolPropertyWidget(parent);
    widget->SetProperty(const_cast<mitk::BoolProperty *>(prop));
    return widget;
  }
  else if (const mitk::IntProperty *prop = dynamic_cast<const mitk::IntProperty *>(property))
  {
    // a number property
    return new QmitkNumberPropertyView(prop, parent);
  }
  else if (const mitk::FloatProperty *prop = dynamic_cast<const mitk::FloatProperty *>(property))
  {
    // a number property
    return new QmitkNumberPropertyView(prop, parent);
  }
  else if (const mitk::DoubleProperty *prop = dynamic_cast<const mitk::DoubleProperty *>(property))
  {
    // a number property
    return new QmitkNumberPropertyView(prop, parent);
  }
  else if (property != nullptr)
  {
    // some unknown property --> use the GetValueAsString() method to
    return new QmitkBasePropertyView(prop, parent);
  }

  return nullptr;
}

QWidget *QmitkPropertyViewFactory::CreateEditor(mitk::BaseProperty *property, unsigned int type, QWidget *parent)
{
  if (!property)
    return nullptr;

  if (mitk::StringProperty *prop = dynamic_cast<mitk::StringProperty *>(property))
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
  else if (mitk::ColorProperty *prop = dynamic_cast<mitk::ColorProperty *>(property))
  {
    // a color property
    return new QmitkColorPropertyEditor(prop, parent);
  }
  else if (mitk::BoolProperty *prop = dynamic_cast<mitk::BoolProperty *>(property))
  {
    // a bool property
    // TODO fix after refactoring
    auto widget = new QmitkBoolPropertyWidget(parent);
    widget->SetProperty(prop);
    return widget;
  }
  else if (mitk::IntProperty *prop = dynamic_cast<mitk::IntProperty *>(property))
  {
    // a number property
    return new QmitkNumberPropertyEditor(prop, parent);
  }
  else if (mitk::FloatProperty *prop = dynamic_cast<mitk::FloatProperty *>(property))
  {
    // a number property
    auto pe = new QmitkNumberPropertyEditor(prop, parent);
    pe->setDecimalPlaces(2);
    return pe;
  }
  else if (mitk::DoubleProperty *prop = dynamic_cast<mitk::DoubleProperty *>(property))
  {
    // a number property
    auto pe = new QmitkNumberPropertyEditor(prop, parent);
    pe->setDecimalPlaces(2);
    return pe;
  }
  else if (mitk::EnumerationProperty *prop = dynamic_cast<mitk::EnumerationProperty *>(property))
  {
    // a enumeration property
    auto pe = new QmitkEnumerationPropertyWidget(parent);
    pe->SetProperty(prop);
    return pe;
  }
  else
  {
    // some unknown property --> no editor possible
    return nullptr;
  }
}
