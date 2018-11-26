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


#ifndef QMITK_INSPECTION_POSITION_WIDGET_H
#define QMITK_INSPECTION_POSITION_WIDGET_H

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

  QmitkInspectionPositionWidget(QWidget* parent = 0);
  ~QmitkInspectionPositionWidget();

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

#endif // QmitkInspectionPositionWidget_H
