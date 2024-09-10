/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkFormsExampleView.h"
#include <QmitkForm.h>

#include <nlohmann/json.hpp>

#include <QFile>
#include <QTextStream>
#include <QVBoxLayout>

const std::string QmitkFormsExampleView::VIEW_ID = "org.mitk.views.example.forms";

namespace
{
  QString ReadFileAsString(const QString& path)
  {
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
      mitkThrow() << "Could not open file \"" << path << "\"!";

    QTextStream stream(&file);
    return stream.readAll();
  }
}

QmitkFormsExampleView::QmitkFormsExampleView()
  : m_Form(nlohmann::json::parse(ReadFileAsString(":/FormsExample/ExampleForm.json").toStdString()))
{
}

QmitkFormsExampleView::~QmitkFormsExampleView()
{
}

void QmitkFormsExampleView::CreateQtPartControl(QWidget* parent)
{
  auto layout = new QVBoxLayout(parent);
  layout->addWidget(new QmitkForm(m_Form));
  layout->addStretch();
}

void QmitkFormsExampleView::SetFocus()
{
}
