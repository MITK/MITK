/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkCmdLineModuleGui.h"
#include "QmitkUiLoader.h"
#include <QVariant>
#include <QIODevice>
#include <QFile>
#include <QScopedPointer>
#include <QWidget>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QmitkCustomVariants.h>
#include <ctkCmdLineModuleXslTransform.h>
#include <ctkCmdLineModuleParameter.h>
#include <ctkCmdLineModuleDescription.h>
#include <ctkCollapsibleGroupBox.h>
#include "mitkDataStorage.h"
#include <mitkImage.h>

//-----------------------------------------------------------------------------
struct QmitkCmdLineModuleGuiPrivate
{
  QmitkCmdLineModuleGuiPrivate(const mitk::DataStorage* dataStorage)
    : m_DataStorage(dataStorage), m_Loader(NULL), m_Transform(NULL), m_TopLevelWidget(NULL)
  {
  }
  const mitk::DataStorage* m_DataStorage;
  mutable QScopedPointer<QUiLoader> m_Loader;
  mutable QScopedPointer<ctkCmdLineModuleXslTransform> m_Transform;
  mutable QScopedPointer<QWidget> m_TopLevelWidget;
};


//-----------------------------------------------------------------------------
QmitkCmdLineModuleGui::QmitkCmdLineModuleGui(const mitk::DataStorage* dataStorage, const ctkCmdLineModuleReference& moduleRef)
  : ctkCmdLineModuleFrontendQtGui(moduleRef)
, d(new QmitkCmdLineModuleGuiPrivate(dataStorage))
{
  qRegisterMetaType<mitk::DataNode::Pointer>();
  qRegisterMetaType<mitkDataNodePtr>();
}


//-----------------------------------------------------------------------------
QmitkCmdLineModuleGui::~QmitkCmdLineModuleGui()
{
}


//-----------------------------------------------------------------------------
QUiLoader* QmitkCmdLineModuleGui::uiLoader() const
{
  // Here we are creating a QUiLoader locally, so when this method
  // is called, it overrides the one in the base class, so the base
  // class one is never constructed, and this one is constructed as
  // a replacement.
  if (d->m_Loader == NULL)
  {
    d->m_Loader.reset(new QmitkUiLoader(d->m_DataStorage));
  }
  return d->m_Loader.data();
}


//-----------------------------------------------------------------------------
ctkCmdLineModuleXslTransform* QmitkCmdLineModuleGui::xslTransform() const
{
  // This is a virtual getter, overriding the one in the base class.
  // However, we want to use the transform in the base class, and just append to it.
  // So we call the base class one, modify it by adding some stuff, and then return
  // the pointer to the one in the base class.
  ctkCmdLineModuleXslTransform *transform = ctkCmdLineModuleFrontendQtGui::xslTransform();
  if (transform != NULL)
  {
    transform->bindVariable("imageInputWidget", QVariant(QString("QmitkDataStorageComboBoxWithSelectNone")));
    transform->bindVariable("imageInputValueProperty", "currentValue");
    transform->bindVariable("imageInputSetProperty", ""); // Don't need this, as we are connected to DataStorage.
  }
  return transform;
}


//-----------------------------------------------------------------------------
QVariant QmitkCmdLineModuleGui::value(const QString &parameter, int role) const
{
  if (role == UserRole)
  {
    ctkCmdLineModuleParameter param = this->moduleReference().description().parameter(parameter);
    if (param.channel() == "input" && param.tag() == "image")
    {
      return this->customValue(parameter, "SelectedNode");
    }
    return QVariant();
  }
  return ctkCmdLineModuleFrontendQtGui::value(parameter, role);
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleGui::setValue(const QString& parameter, const QVariant& value, int role)
{
  if (role == UserRole)
  {
    ctkCmdLineModuleParameter param = this->moduleReference().description().parameter(parameter);
    if (param.channel() == "input" && param.tag() == "image")
    {
      this->setCustomValue(parameter, value, "SelectedNode");
    }
    else
    {
      ctkCmdLineModuleFrontendQtGui::setValue(parameter, value, role);
    }
  }
  else
  {
    ctkCmdLineModuleFrontendQtGui::setValue(parameter, value, role);
  }
}

//-----------------------------------------------------------------------------
QWidget* QmitkCmdLineModuleGui::getGui()
{
  if (!d->m_TopLevelWidget)
  {
    // Construct additional widgets to contain full GUI.
    QWidget *aboutBoxContainerWidget = new QWidget();

    ctkCollapsibleGroupBox *aboutBox = new ctkCollapsibleGroupBox(aboutBoxContainerWidget);
    aboutBox->setTitle("About");

    QTextBrowser *aboutBrowser = new QTextBrowser(aboutBox);
    aboutBrowser->setReadOnly(true);
    aboutBrowser->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    aboutBrowser->setOpenExternalLinks(true);
    aboutBrowser->setOpenLinks(true);

    QVBoxLayout *aboutLayout = new QVBoxLayout(aboutBox);
    aboutLayout->addWidget(aboutBrowser);

    QVBoxLayout *aboutBoxContainerWidgetLayout = new QVBoxLayout(aboutBoxContainerWidget);
    aboutBoxContainerWidgetLayout->addWidget(aboutBox);

    QWidget *helpBoxContainerWidget = new QWidget();

    ctkCollapsibleGroupBox *helpBox = new ctkCollapsibleGroupBox();
    helpBox->setTitle("Help");

    QTextBrowser *helpBrowser = new QTextBrowser(helpBox);
    helpBrowser->setReadOnly(true);
    helpBrowser->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    helpBrowser->setOpenExternalLinks(true);
    helpBrowser->setOpenLinks(true);

    QVBoxLayout *helpLayout = new QVBoxLayout(helpBox);
    helpLayout->addWidget(helpBrowser);

    QVBoxLayout *helpBoxContainerWidgetLayout = new QVBoxLayout(helpBoxContainerWidget);
    helpBoxContainerWidgetLayout->addWidget(helpBox);

    QObject* guiHandle = this->guiHandle();
    QWidget* generatedGuiWidgets = qobject_cast<QWidget*>(guiHandle);

    QWidget *topLevelWidget = new QWidget();

    QGridLayout *topLevelLayout = new QGridLayout(topLevelWidget);
    topLevelLayout->setContentsMargins(0,0,0,0);
    topLevelLayout->setSpacing(0);
    topLevelLayout->addWidget(aboutBoxContainerWidget, 0, 0);
    topLevelLayout->addWidget(helpBoxContainerWidget, 1, 0);
    topLevelLayout->addWidget(generatedGuiWidgets, 2, 0);

    ctkCmdLineModuleDescription description = this->moduleReference().description();

    QString helpString = "";

    if (!description.title().isEmpty())
    {
      QString titleHtml = "<h1>" + description.title() + "</h1>";
      helpString += titleHtml;
    }

    if (!description.description().isEmpty())
    {
      QString descriptionHtml = "<p>" + description.description() + "</p>";
      helpString += descriptionHtml;
    }

    if (!description.documentationURL().isEmpty())
    {
      QString docUrlHtml = "<p>For more information please see <a href=\"" + description.documentationURL()
          + "\">" + description.documentationURL() + "</a></p>";
      helpString += docUrlHtml;
    }

    QString aboutString = "";

    if (!description.title().isEmpty())
    {
      QString titleHtml = "<h1>" + description.title() + "</h1>";
      aboutString += titleHtml;
    }

    if (!description.contributor().isEmpty())
    {
      QString contributorHtml = "<h2>Contributed By</h2><p>" + description.contributor() + "</p>";
      aboutString += contributorHtml;
    }

    if (!description.license().isEmpty())
    {
      QString licenseHtml = "<h2>License</h2><p>" + description.license() + "</p>";
      aboutString += licenseHtml;
    }

    if (!description.acknowledgements().isEmpty())
    {
      QString acknowledgementsHtml = "<h2>Acknowledgements</h2><p>" + description.acknowledgements() + "</p>";
      aboutString += acknowledgementsHtml;
    }

    helpBrowser->clear();
    helpBrowser->setHtml(helpString);
    helpBox->setCollapsed(true);

    aboutBrowser->clear();
    aboutBrowser->setHtml(aboutString);
    aboutBox->setCollapsed(true);

    d->m_TopLevelWidget.reset(topLevelWidget);
  }

  return d->m_TopLevelWidget.data();
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleGui::copyParameters(QmitkCmdLineModuleGui& another)
{

  // This copies "display" parameter types.
  // See ctkCmdLineModuleFrontend::DisplayRole
  this->setValues(another.values());

  // For custom types, we must manually copy the values.
  // In our case, we copy image types manually, to pass across the mitk::DataNode pointer.
  QList<ctkCmdLineModuleParameter> parameters = another.parameters("image", ctkCmdLineModuleFrontend::Input);
  foreach (ctkCmdLineModuleParameter parameter, parameters)
  {
    QString parameterName = parameter.name();

    QVariant tmp = another.value(parameterName, ctkCmdLineModuleFrontend::UserRole);
    mitk::DataNode::Pointer node = tmp.value<mitk::DataNode::Pointer>();

    if (node.IsNotNull())
    {
      mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());
      if (image != NULL)
      {
        this->setValue(parameterName, tmp, ctkCmdLineModuleFrontend::UserRole);
      }
    }
  }
}
