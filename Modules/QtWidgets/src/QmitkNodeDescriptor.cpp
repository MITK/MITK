/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkNodeDescriptor.h>
#include <memory>
#include <mitkExtractSliceFilter.h>
#include <mitkImage.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateProperty.h>
#include <mitkPlaneGeometry.h>
#include <mitkProperties.h>

#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkMitkLevelWindowFilter.h>
#include <vtkSmartPointer.h>

#include <QImage>

QmitkNodeDescriptor::QmitkNodeDescriptor(const QString &_ClassName,
                                         const QString &_PathToIcon,
                                         mitk::NodePredicateBase *_Predicate,
                                         QObject *parent)
  : QmitkNodeDescriptor(_ClassName, QIcon(_PathToIcon), _Predicate, parent)
{
}

QmitkNodeDescriptor::QmitkNodeDescriptor(const QString &_ClassName,
                                         const QIcon &_Icon,
                                         mitk::NodePredicateBase *_Predicate,
                                         QObject *parent)
  : QObject(parent),
    m_ClassName(_ClassName),
    m_Icon(_Icon),
    m_Predicate(_Predicate),
    m_Separator(new QAction(this))
{
  m_Separator->setSeparator(true);
}

QString QmitkNodeDescriptor::GetNameOfClass() const
{
  return m_ClassName;
}

QIcon QmitkNodeDescriptor::GetIcon(const mitk::DataNode *) const
{
  return m_Icon;
}

QPixmap QmitkNodeDescriptor::GenerateThumbnail(const mitk::DataNode *node, int size) const
{
  if (nullptr == node)
    return QPixmap();

  QPixmap thumbnail;

  if (const auto *image = dynamic_cast<const mitk::Image *>(node->GetData()); nullptr != image)
  {
    mitk::LevelWindow levelWindow;
    node->GetLevelWindow(levelWindow);

    auto lookupTable = vtkSmartPointer<vtkLookupTable>::New();
    lookupTable->SetRange(levelWindow.GetLowerWindowBound(), levelWindow.GetUpperWindowBound());
    lookupTable->SetSaturationRange(0.0, 0.0);
    lookupTable->SetValueRange(0.0, 1.0);
    lookupTable->SetHueRange(0.0, 0.0);
    lookupTable->SetRampToLinear();

    thumbnail = RenderThumbnail(image, lookupTable, size);
  }

  return thumbnail.isNull()
    ? this->GetIcon(node).pixmap(size, size)
    : thumbnail;
}

QPixmap QmitkNodeDescriptor::RenderThumbnail(const mitk::Image *image, vtkLookupTable *lookupTable, int size)
{
  if (nullptr == image || !image->IsInitialized() || nullptr == lookupTable)
    return QPixmap();

  // The QImage below wraps the filter output buffer as ARGB32 without
  // conversion, so anything but a single component would be misread.
  if (image->GetPixelType().GetNumberOfComponents() != 1)
    return QPixmap();

  auto planeGeometry = mitk::PlaneGeometry::New();
  const int sliceNumber = image->GetDimension(2) / 2;
  planeGeometry->InitializeStandardPlane(image->GetGeometry(), mitk::AnatomicalPlane::Axial, sliceNumber);

  auto extractSliceFilter = mitk::ExtractSliceFilter::New();
  extractSliceFilter->SetInput(image);
  extractSliceFilter->SetInterpolationMode(mitk::ExtractSliceFilter::RESLICE_NEAREST);
  extractSliceFilter->SetResliceTransformByGeometry(image->GetGeometry());
  extractSliceFilter->SetWorldGeometry(planeGeometry);
  extractSliceFilter->SetOutputDimensionality(2);
  extractSliceFilter->SetVtkOutputRequest(true);
  extractSliceFilter->Update();

  vtkImageData *imageData = extractSliceFilter->GetVtkOutput();

  int dims[3];
  imageData->GetDimensions(dims);

  auto levelWindowFilter = vtkSmartPointer<vtkMitkLevelWindowFilter>::New();
  levelWindowFilter->SetLookupTable(lookupTable);
  levelWindowFilter->SetInputData(imageData);
  levelWindowFilter->SetMinOpacity(0.0);
  levelWindowFilter->SetMaxOpacity(1.0);
  double clippingBounds[] = { 0.0, static_cast<double>(dims[0]), 0.0, static_cast<double>(dims[1]) };
  levelWindowFilter->SetClippingBounds(clippingBounds);
  levelWindowFilter->Update();
  imageData = levelWindowFilter->GetOutput();

  QImage thumbnailImage(reinterpret_cast<const unsigned char *>(imageData->GetScalarPointer()), dims[0], dims[1], QImage::Format_ARGB32);

  thumbnailImage = dims[0] > dims[1]
    ? thumbnailImage.scaledToWidth(size, Qt::SmoothTransformation).rgbSwapped()
    : thumbnailImage.scaledToHeight(size, Qt::SmoothTransformation).rgbSwapped();

  return QPixmap::fromImage(thumbnailImage);
}

QList<QAction *> QmitkNodeDescriptor::GetActions() const
{
  return m_Actions;
}

bool QmitkNodeDescriptor::CheckNode(const mitk::DataNode *node) const
{
  if (m_Predicate.IsNotNull())
    return m_Predicate->CheckNode(node);
  return false;
}

void QmitkNodeDescriptor::AddAction(QAction *action, bool isBatchAction)
{
  if (!action)
    return;

  if (isBatchAction)
    m_BatchActions.push_back(action);
  else
    m_Actions.push_back(action);
  QObject::connect(action, SIGNAL(destroyed(QObject *)), this, SLOT(ActionDestroyed(QObject *)));
}

void QmitkNodeDescriptor::RemoveAction(QAction *_Action)
{
  int index = m_Actions.indexOf(_Action);
  int indexOfWidgetAction = m_BatchActions.indexOf(_Action);

  if (index != -1)
  {
    m_Actions.removeAt(index);
  }
  else if (indexOfWidgetAction != -1)
  {
    m_BatchActions.removeAt(indexOfWidgetAction);
  }

  if (_Action != nullptr)
  {
    QObject::disconnect(_Action, SIGNAL(destroyed(QObject *)), this, SLOT(ActionDestroyed(QObject *)));
  }
}

QmitkNodeDescriptor::~QmitkNodeDescriptor()
{
  // all children are destroyed here by Qt
}

QAction *QmitkNodeDescriptor::GetSeparator() const
{
  return m_Separator;
}

QList<QAction *> QmitkNodeDescriptor::GetBatchActions() const
{
  return m_BatchActions;
}

void QmitkNodeDescriptor::ActionDestroyed(QObject *obj /*= 0 */)
{
  this->RemoveAction(qobject_cast<QAction *>(obj));
}
