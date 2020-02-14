/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkScalarBarOverlay.h"

#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkPropertyList.h"

#include <itkCommand.h>

#include <QLayout>

QmitkScalarBarOverlay::QmitkScalarBarOverlay(const char *id) : QmitkOverlay(id), m_ScalarBar(nullptr), m_ObserverTag(0)
{
  m_Widget = m_ScalarBar = new QmitkScalarBar();
  QmitkOverlay::AddDropShadow(m_ScalarBar);
}

QmitkScalarBarOverlay::~QmitkScalarBarOverlay()
{
  m_PropertyList->GetProperty(m_Id)->RemoveObserver(m_ObserverTag);
  m_PropertyList = nullptr;
}

void QmitkScalarBarOverlay::GenerateData(mitk::PropertyList::Pointer pl)
{
  if (pl.IsNull())
    return;

  m_PropertyList = pl->Clone();

  if (m_PropertyList.IsNotNull())
  {
    this->SetupCallback(m_PropertyList->GetProperty(m_Id));

    this->GetProperties(pl);
    this->SetScaleFactor();
  }
  else
  {
    MITK_DEBUG << "invalid propList";
  }
}

void QmitkScalarBarOverlay::SetScaleFactor()
{
  float scale = 2;
  if (m_PropertyList.IsNull() || !m_PropertyList->GetFloatProperty(m_Id, scale))
  {
    MITK_DEBUG << "Property " << m_Id << " could not be found";
  }

  if (m_ScalarBar != nullptr)
  {
    m_ScalarBar->SetScaleFactor(scale);
  }
}

void QmitkScalarBarOverlay::GetProperties(mitk::PropertyList::Pointer pl)
{
  if (pl.IsNull())
    return;

  QPen pen = QPen();

  mitk::PropertyList::Pointer propertyList = pl;
  QPalette palette = QPalette();

  // get the desired color of the textOverlays
  mitk::ColorProperty::Pointer colorProp =
    dynamic_cast<mitk::ColorProperty *>(propertyList->GetProperty("overlay.color"));

  if (colorProp.IsNull())
  {
    MITK_DEBUG << "creating new colorProperty";
    colorProp = mitk::ColorProperty::New(127.0, 196.0, 232.0);
  }

  mitk::Color color = colorProp->GetColor();
  pen.setColor(QColor(color[0], color[1], color[2], 255));
  pen.setStyle(Qt::SolidLine);
  pen.setCapStyle(Qt::FlatCap);
  pen.setJoinStyle(Qt::MiterJoin);

  m_ScalarBar->SetPen(pen);
}

void QmitkScalarBarOverlay::SetupCallback(mitk::BaseProperty::Pointer prop)
{
  if (m_ObservedProperty != prop && m_ObserverTag == 0)
  {
    if (prop.IsNotNull())
    {
      if (m_ObservedProperty.IsNotNull())
      {
        m_ObservedProperty->RemoveObserver(m_ObserverTag);
      }

      typedef itk::SimpleMemberCommand<QmitkScalarBarOverlay> MemberCommandType;
      MemberCommandType::Pointer propModifiedCommand;
      propModifiedCommand = MemberCommandType::New();
      propModifiedCommand->SetCallbackFunction(this, &QmitkScalarBarOverlay::SetScaleFactor);
      m_ObserverTag = prop->AddObserver(itk::ModifiedEvent(), propModifiedCommand);
    }

    m_ObservedProperty = prop;
  }
  else
  {
    MITK_DEBUG << "invalid property";
  }
}

QSize QmitkScalarBarOverlay::GetNeededSize()
{
  return m_Widget->size();
}
