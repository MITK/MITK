/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkEthel.h"

#include <QmitkAbstractMultiWidget.h>
#include <QmitkIconTheme.h>
#include <QmitkRenderWindow.h>
#include <QmitkRenderWindowWidget.h>

#include <mitkCoreServices.h>
#include <mitkExceptionMacro.h>
#include <mitkFileReaderRegistry.h>
#include <mitkIMimeTypeProvider.h>
#include <mitkImage.h>
#include <mitkLog.h>
#include <mitkRenderingManager.h>
#include <mitkSmartPointerProperty.h>
#include <mitkSurface.h>

#include <QFile>
#include <QMenu>

#include <sstream>

namespace
{
  constexpr int TriggerCount = 10;

  // Shared by all editors so that the count survives closing and reopening one.
  int lightingMenuCount = 0;

  /** Reads a Qt resource through the file reader registered for its extension. */
  mitk::BaseData::Pointer LoadResourceData(const QString& path)
  {
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
      mitkThrow() << "Could not open resource " << path.toStdString() << '.';

    const auto bytes = file.readAll();
    std::istringstream stream(std::string(bytes.constData(), static_cast<size_t>(bytes.size())), std::ios::binary);

    const auto location = path.toStdString();
    mitk::CoreServicePointer<mitk::IMimeTypeProvider> mimeTypeProvider(mitk::CoreServices::GetMimeTypeProvider());
    const auto mimeTypes = mimeTypeProvider->GetMimeTypesForFile(location);

    if (mimeTypes.empty())
      mitkThrow() << "No mime type for resource " << location << '.';

    mitk::FileReaderRegistry readerRegistry;
    const auto readerReferences = readerRegistry.GetReferences(mimeTypes[0]);

    if (readerReferences.empty())
      mitkThrow() << "No reader for resource " << location << '.';

    auto* reader = readerRegistry.GetReader(readerReferences[0]);
    reader->SetInput(location, &stream);
    const auto data = reader->Read();

    if (data.empty() || data[0].IsNull())
      mitkThrow() << "Resource " << location << " did not yield any data.";

    return data[0];
  }

  mitk::DataNode::Pointer CreateEthelNode()
  {
    mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>(
      LoadResourceData(QStringLiteral(":/org.mitk.gui.qt.stdmultiwidgeteditor/ethel.vtp")).GetPointer());

    mitk::Image::Pointer texture = dynamic_cast<mitk::Image*>(
      LoadResourceData(QStringLiteral(":/org.mitk.gui.qt.stdmultiwidgeteditor/ethel.jpg")).GetPointer());

    if (surface.IsNull() || texture.IsNull())
      mitkThrow() << "The resources are not a surface and an image.";

    auto ethelNode = mitk::DataNode::New();
    ethelNode->SetData(surface);
    ethelNode->SetName("Ethel");

    // The surface mapper passes the image rows to VTK as they are, ITK top-down
    // versus VTK bottom-up, so the texture resource is stored upside down.
    ethelNode->SetProperty("Surface.Texture", mitk::SmartPointerProperty::New(texture));

    return ethelNode;
  }
}

QmitkEthel::QmitkEthel(QmitkAbstractMultiWidget* multiWidget)
  : QObject(multiWidget)
  , m_MultiWidget(multiWidget)
{
  for (const auto& [name, widget] : multiWidget->Get3DRenderWindowWidgets())
    connect(widget->GetRenderWindow(), &QmitkRenderWindow::LightingMenuAboutToShow, this, &QmitkEthel::OnLightingMenuAboutToShow);
}

QmitkEthel::~QmitkEthel() = default;

void QmitkEthel::OnLightingMenuAboutToShow(QMenu* menu)
{
  if (++lightingMenuCount != TriggerCount)
    return;

  // The menu is rebuilt every time it opens, so the entry lives for this
  // one opening only.
  menu->addSeparator();

  auto* action = menu->addAction(
    QmitkIconTheme::GetIcon(QStringLiteral(":/org.mitk.gui.qt.stdmultiwidgeteditor/ethel.svg")), QStringLiteral("Spawn Ethel"));

  connect(action, &QAction::triggered, this, &QmitkEthel::OnEthelTriggered);
}

void QmitkEthel::OnEthelTriggered()
{
  try
  {
    this->Show();
  }
  catch (const std::exception& e)
  {
    MITK_WARN << e.what();
  }
}

void QmitkEthel::Show()
{
  auto* dataStorage = m_MultiWidget->GetDataStorage();

  if (nullptr == dataStorage)
    return;

  if (m_EthelNode.IsNull() || !dataStorage->Exists(m_EthelNode))
  {
    m_EthelNode = CreateEthelNode();
    dataStorage->Add(m_EthelNode);
  }

  m_EthelNode->SetBoolProperty("color-cycling", true);
  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(dataStorage);

  // The rotation keeps the 3D window rendering, which is what advances the colors.
  for (const auto& [name, widget] : m_MultiWidget->Get3DRenderWindowWidgets())
    widget->GetRenderWindow()->SetAutoRotation(true);
}
