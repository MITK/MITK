/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkInspectionPositionWidget_h
#define QmitkInspectionPositionWidget_h

#include "mitkModelFitInfo.h"
#include "mitkPointSet.h"

#include "MitkModelFitUIExports.h"

#include "ui_QmitkInspectionPositionWidget.h"
#include <QWidget>


class QmitkFitParameterModel;

/**
* \class QmitkInspectionPositionWidget
* \brief Widget that allows manage the positions that should be used to inspect fits.
*/
class MITKMODELFITUI_EXPORT QmitkInspectionPositionWidget : public QWidget
{
  Q_OBJECT

public:
  using FitVectorType = std::vector<mitk::modelFit::ModelFitInfo::ConstPointer>;

  QmitkInspectionPositionWidget(QWidget* parent = nullptr);
  ~QmitkInspectionPositionWidget() override;

  mitk::Point3D GetCurrentPosition() const;

  const mitk::PointSet* GetPositionBookmarks() const;

  /** @brief assign a point set (contained in a node of DataStorage) for observation */
  void SetPositionBookmarkNode(mitk::DataNode *newNode);
  mitk::DataNode *GetPositionBookmarkNode();

public Q_SLOTS:

  void SetCurrentPosition(const mitk::Point3D& currentPos);
  void OnPointListChanged();
  void OnAddCurrentPositionClicked();

Q_SIGNALS:
  /** Is emitted as soon as the position bookmarks changed.*/
  void PositionBookmarksChanged();

protected:

  mitk::Point3D m_CurrentPosition;

  Ui::QmitkInspectionPositionWidget m_Controls;

};

#endif
