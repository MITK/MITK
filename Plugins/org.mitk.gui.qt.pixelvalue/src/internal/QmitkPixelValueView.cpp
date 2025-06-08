/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkPixelValueView.h"
#include <QmitkStyleManager.h>

#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkImage.h>
#include <mitkCompositePixelValueToString.h>
#include <mitkPixelTypeMultiplex.h>
#include <mitkImagePixelReadAccessor.h>

#include <QClipboard>

#include <ui_QmitkPixelValueView.h>

#include <regex>

namespace
{
  void CopyCoordsToClipboard(const QLineEdit* x, const QLineEdit* y, const QLineEdit* z)
  {
    auto text = QStringList() << x->text() << y->text();

    if (const auto zText = z->text(); !zText.isEmpty())
      text << zText;

    QApplication::clipboard()->setText(text.join(", "));
  }
}

const std::string QmitkPixelValueView::VIEW_ID = "org.mitk.views.pixelvalue";

QmitkPixelValueView::QmitkPixelValueView(QObject*)
  : m_Ui(new Ui::QmitkPixelValueView)
{
}

QmitkPixelValueView::~QmitkPixelValueView()
{
}

void QmitkPixelValueView::CreateQtPartControl(QWidget* parent)
{
  m_Ui->setupUi(parent);

  auto clipboardIcon = QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/clipboard.svg"));
  m_Ui->copyIndexCoordButton->setIcon(clipboardIcon);
  m_Ui->copyWorldCoordButton->setIcon(clipboardIcon);

  connect(m_Ui->copyIndexCoordButton, &QToolButton::clicked, [this]() {
    CopyCoordsToClipboard(m_Ui->indexXLineEdit, m_Ui->indexYLineEdit, m_Ui->indexZLineEdit);
  });

  connect(m_Ui->copyWorldCoordButton, &QToolButton::clicked, [this]() {
    CopyCoordsToClipboard(m_Ui->worldXLineEdit, m_Ui->worldYLineEdit, m_Ui->worldZLineEdit);
  });

  this->m_SliceNavigationListener.RenderWindowPartActivated(this->GetRenderWindowPart());
  connect(&m_SliceNavigationListener, &QmitkSliceNavigationListener::SelectedPositionChanged, this, &QmitkPixelValueView::OnSelectedPositionChanged);
  connect(&m_SliceNavigationListener, &QmitkSliceNavigationListener::SelectedTimePointChanged, this, &QmitkPixelValueView::OnSelectedTimePointChanged);

  this->Update();
}

void QmitkPixelValueView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  this->m_SliceNavigationListener.RenderWindowPartActivated(renderWindowPart);
}

void QmitkPixelValueView::RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart)
{
  this->m_SliceNavigationListener.RenderWindowPartDeactivated(renderWindowPart);
}

void QmitkPixelValueView::RenderWindowPartInputChanged(mitk::IRenderWindowPart* renderWindowPart)
{
  this->m_SliceNavigationListener.RenderWindowPartInputChanged(renderWindowPart);
}

void QmitkPixelValueView::OnSelectedPositionChanged(const mitk::Point3D&)
{
  this->Update();
}

void QmitkPixelValueView::OnSelectedTimePointChanged(const mitk::TimePointType&)
{
  this->Update();
}

void QmitkPixelValueView::NodeChanged(const mitk::DataNode*)
{
  this->Update();
}

void QmitkPixelValueView::Update()
{
  const auto position = m_SliceNavigationListener.GetCurrentSelectedPosition();
  const auto timePoint = m_SliceNavigationListener.GetCurrentSelectedTimePoint();
  
  auto isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
  auto isSegmentation = mitk::NodePredicateDataType::New("MultiLabelSegmentation");

  auto predicate = mitk::NodePredicateAnd::New();
  predicate->AddPredicate(mitk::NodePredicateNot::New(isSegmentation));
  predicate->AddPredicate(isImage);

  auto nodes = GetDataStorage()->GetSubset(predicate);

  if (nodes.IsNull())
  {
    this->Clear();
    return;
  }

  mitk::Image::Pointer image;
  mitk::DataNode::Pointer topParentNode;

  int component = 0;

  auto node = mitk::FindTopmostVisibleNode(nodes, position, timePoint, nullptr);

  if (node.IsNull())
  {
    this->Clear();
    return;
  }

  bool isBinary = false;
  node->GetBoolProperty("binary", isBinary);

  if (isBinary)
  {
    auto parentNodes = GetDataStorage()->GetSources(node, nullptr, true);

    if (!parentNodes->empty())
      topParentNode = FindTopmostVisibleNode(nodes, position, timePoint, nullptr);

    if (topParentNode.IsNotNull())
    {
      image = dynamic_cast<mitk::Image*>(topParentNode->GetData());
      topParentNode->GetIntProperty("Image.Displayed Component", component);
    }
    else
    {
      image = dynamic_cast<mitk::Image*>(node->GetData());
      node->GetIntProperty("Image.Displayed Component", component);
    }
  }
  else
  {
    image = dynamic_cast<mitk::Image*>(node->GetData());
    node->GetIntProperty("Image.Displayed Component", component);
  }

  if (image.IsNotNull())
  {
    m_Ui->imageNameLineEdit->setText(QString::fromStdString(node->GetName()));

    itk::Index<3> index;
    image->GetGeometry()->WorldToIndex(position, index);

    auto pixelType = image->GetChannelDescriptor().GetPixelType().GetPixelType();

    if (pixelType == itk::IOPixelEnum::RGB || pixelType == itk::IOPixelEnum::RGBA)
    {
      m_Ui->pixelValueLineEdit->setText(QString::fromStdString(mitk::ConvertCompositePixelValueToString(image, index)));
    }
    else if (pixelType == itk::IOPixelEnum::DIFFUSIONTENSOR3D || pixelType == itk::IOPixelEnum::SYMMETRICSECONDRANKTENSOR)
    {
      m_Ui->pixelValueLineEdit->setText(QStringLiteral("See ODF Details view."));
    }
    else
    {
      mitk::ScalarType pixelValue = 0.0;

      mitkPixelTypeMultiplex5(
        mitk::FastSinglePixelAccess,
        image->GetChannelDescriptor().GetPixelType(),
        image,
        image->GetVolumeData(image->GetTimeGeometry()->TimePointToTimeStep(timePoint)),
        index,
        pixelValue,
        component);

      std::ostringstream stream;
      stream.imbue(std::locale::classic());
      stream.precision(2);

      if (fabs(pixelValue) > 1000000 || fabs(pixelValue) < 0.01)
      {
        stream << std::scientific;
      }
      else
      {
        stream << std::fixed;
      }

      stream << pixelValue;

      m_Ui->pixelValueLineEdit->setText(QString::fromStdString(stream.str()));
    }

    this->UpdateCoords(image, index, position);
  }
  else
  {
    this->Clear();
  }
}

void QmitkPixelValueView::UpdateCoords(const mitk::Image* image, const itk::Index<3>& index, const mitk::Point3D& position)
{
  const auto dimension = image->GetDimension();

  this->UpdateIndexCoord(index, dimension);
  this->UpdateWorldCoord(position, dimension);

  const auto geometry = image->GetTimeGeometry();

  this->UpdateTimeStep(geometry);
}

void QmitkPixelValueView::UpdateIndexCoord(const itk::Index<3>& index, unsigned int dimension)
{
  m_Ui->indexXLineEdit->setText(QString::number(index[0]));
  m_Ui->indexYLineEdit->setText(QString::number(index[1]));

  if (dimension > 2)
  {
    m_Ui->indexZLineEdit->setText(QString::number(index[2]));
  }
  else
  {
    m_Ui->indexZLineEdit->clear();
  }

  m_Ui->copyIndexCoordButton->setEnabled(true);
}

void QmitkPixelValueView::UpdateWorldCoord(const mitk::Point3D& position, unsigned int dimension)
{
  m_Ui->worldXLineEdit->setText(QString::number(position[0], 'f', 2));
  m_Ui->worldYLineEdit->setText(QString::number(position[1], 'f', 2));

  if (dimension > 2)
  {
    m_Ui->worldZLineEdit->setText(QString::number(position[2], 'f', 2));
  }
  else
  {
    m_Ui->worldZLineEdit->clear();
  }

  m_Ui->copyWorldCoordButton->setEnabled(true);
}

void QmitkPixelValueView::UpdateTimeStep(const mitk::TimeGeometry* geometry)
{
  const auto* renderingManager = mitk::RenderingManager::GetInstance();
  const auto* timeNavController = renderingManager->GetTimeNavigationController();
  const auto timePoint = timeNavController->GetSelectedTimePoint();

  if (geometry->CountTimeSteps() <= 1 || !geometry->IsValidTimePoint(timePoint))
  {
    this->ShowTimeStep(false);
    return;
  }

  const auto timeStep = geometry->TimePointToTimeStep(timePoint);
  m_Ui->timeStepLineEdit->setText(QString::number(timeStep));

  this->ShowTimeStep(true);
}

void QmitkPixelValueView::ShowTimeStep(bool show)
{
  m_Ui->timeStepLabel->setVisible(show);
  m_Ui->timeStepLineEdit->setVisible(show);
}

void QmitkPixelValueView::Clear()
{
  m_Ui->imageNameLineEdit->clear();
  m_Ui->pixelValueLineEdit->clear();

  this->ClearCoords();
}

void QmitkPixelValueView::ClearCoords()
{
  m_Ui->indexXLineEdit->clear();
  m_Ui->indexYLineEdit->clear();
  m_Ui->indexZLineEdit->clear();

  m_Ui->copyIndexCoordButton->setEnabled(false);

  m_Ui->worldXLineEdit->clear();
  m_Ui->worldYLineEdit->clear();
  m_Ui->worldZLineEdit->clear();

  m_Ui->copyWorldCoordButton->setEnabled(false);

  m_Ui->timeStepLineEdit->clear();

  this->ShowTimeStep(false);
}

void QmitkPixelValueView::SetFocus()
{
  m_Ui->pixelValueLineEdit->setFocus();
}
