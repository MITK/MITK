/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkIsoSurface.h"

// MITK headers
#include <mitkManualSegmentationToSurfaceFilter.h>
#include <mitkNodePredicateDataType.h>
#include <mitkSurface.h>
#include <mitkIPreferences.h>

// Qt-GUI headers
#include <QApplication>
#include <QCursor>
#include <QMessageBox>

QmitkIsoSurface::QmitkIsoSurface(QObject * /*parent*/, const char * /*name*/)
  : m_Controls(nullptr), m_MitkImage(nullptr), m_SurfaceCounter(0)
{
}

void QmitkIsoSurface::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    m_Controls = new Ui::QmitkIsoSurfaceControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    m_Controls->m_ImageSelector->SetDataStorage(this->GetDataStorage());
    m_Controls->m_ImageSelector->SetPredicate(mitk::NodePredicateDataType::New("Image"));

    auto* prefs = this->GetPreferences();
    if (prefs != nullptr)
      m_Controls->thresholdLineEdit->setText(QString::fromStdString(prefs->Get("defaultThreshold", "0")));
  }
}

void QmitkIsoSurface::SetFocus()
{
  m_Controls->m_ImageSelector->setFocus();
}

void QmitkIsoSurface::CreateConnections()
{
  if (m_Controls)
  {
    connect(m_Controls->m_ImageSelector,
            SIGNAL(OnSelectionChanged(const mitk::DataNode *)),
            this,
            SLOT(ImageSelected(const mitk::DataNode *)));
    connect(m_Controls->createSurfacePushButton, SIGNAL(clicked()), this, SLOT(CreateSurface()));
  }
}

void QmitkIsoSurface::ImageSelected(const mitk::DataNode *item)
{
  // nothing selected (nullptr selection)
  if (item == nullptr || item->GetData() == nullptr)
    return;

  m_MitkImage = dynamic_cast<mitk::Image *>(item->GetData());
}

void QmitkIsoSurface::CreateSurface()
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  if (m_MitkImage != nullptr)
  {
    // Value Gauss
    // float gsDev = 1.5;

    // Value for DecimatePro
    float targetReduction = 0.05;

    // ImageToSurface Instance
    mitk::DataNode::Pointer node = m_Controls->m_ImageSelector->GetSelectedNode();

    mitk::ManualSegmentationToSurfaceFilter::Pointer filter = mitk::ManualSegmentationToSurfaceFilter::New();
    if (filter.IsNull())
    {
      std::cout << "nullptr Pointer for ManualSegmentationToSurfaceFilter" << std::endl;
      return;
    }

    filter->SetInput(m_MitkImage);
    filter->SetGaussianStandardDeviation(0.5);
    filter->SetUseGaussianImageSmooth(true);
    filter->SetThreshold(getThreshold()); // if( Gauss ) --> TH manipulated for vtkMarchingCube

    filter->SetTargetReduction(targetReduction);

    int numOfPolys = filter->GetOutput()->GetVtkPolyData()->GetNumberOfPolys();
    if (numOfPolys > 2000000)
    {
      QApplication::restoreOverrideCursor();
      if (QMessageBox::question(nullptr,
                                "CAUTION!!!",
                                "The number of polygons is greater than 2 000 000. If you continue, the program might "
                                "crash. How do you want to go on?",
                                "Proceed anyway!",
                                "Cancel immediately! (maybe you want to insert an other threshold)!",
                                QString::null,
                                0,
                                1) == 1)
      {
        return;
      }
      QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    }
    mitk::DataNode::Pointer surfaceNode = mitk::DataNode::New();
    surfaceNode->SetData(filter->GetOutput());

    int layer = 0;

    ++m_SurfaceCounter;
    std::ostringstream buffer;
    buffer << m_SurfaceCounter;
    std::string surfaceNodeName = "Surface " + buffer.str();

    node->GetIntProperty("layer", layer);
    surfaceNode->SetIntProperty("layer", layer + 1);
    surfaceNode->SetProperty("Surface", mitk::BoolProperty::New(true));
    surfaceNode->SetProperty("name", mitk::StringProperty::New(surfaceNodeName));

    this->GetDataStorage()->Add(surfaceNode, node);

    // to show surfaceContur
    surfaceNode->SetColor(m_RainbowColor.GetNextColor());
    surfaceNode->SetVisibility(true);

    mitk::IRenderWindowPart *renderPart = this->GetRenderWindowPart();
    if (renderPart)
    {
      renderPart->RequestUpdate();
    }
  }

  QApplication::restoreOverrideCursor();
}

float QmitkIsoSurface::getThreshold()
{
  return m_Controls->thresholdLineEdit->text().toFloat();
}

QmitkIsoSurface::~QmitkIsoSurface()
{
  auto* prefs = this->GetPreferences();
  if (prefs != nullptr)
    prefs->Put("defaultThreshold", m_Controls->thresholdLineEdit->text().toStdString());
}
