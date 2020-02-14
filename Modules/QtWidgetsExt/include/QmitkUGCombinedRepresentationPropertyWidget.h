/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKUGCOMBINEDREPRESENTATIONPROPERTYWIDGET_H
#define QMITKUGCOMBINEDREPRESENTATIONPROPERTYWIDGET_H

#include "MitkQtWidgetsExtExports.h"

#include <QComboBox>

namespace mitk
{
  class GridVolumeMapperProperty;
  class GridRepresentationProperty;
  class BoolProperty;
}

class _UGCombinedEnumPropEditor;
class _UGCombinedBoolPropEditor;

/// @ingroup Widgets
class MITKQTWIDGETSEXT_EXPORT QmitkUGCombinedRepresentationPropertyWidget : public QComboBox
{
  Q_OBJECT

public:
  QmitkUGCombinedRepresentationPropertyWidget(QWidget *parent = nullptr);
  ~QmitkUGCombinedRepresentationPropertyWidget() override;

  void SetProperty(mitk::GridRepresentationProperty *gridRepresentation,
                   mitk::GridVolumeMapperProperty *volumeMapper,
                   mitk::BoolProperty *volumeProp);

protected slots:

  void OnIndexChanged(int index);

protected:
  friend class _UGCombinedEnumPropEditor;
  friend class _UGCombinedBoolPropEditor;

  void SetGridRepresentationId(int enumId);
  void SetGridVolumeId(int enumId);
  void IsVolumeChanged(bool volume);

  _UGCombinedEnumPropEditor *gridRepPropEditor;
  _UGCombinedEnumPropEditor *volumeMapperPropEditor;
  _UGCombinedBoolPropEditor *volumePropEditor;

  int m_GridRepIndex;
  int m_GridVolIndex;

  int m_FirstVolumeRepId;
  QHash<int, int> m_MapRepEnumToIndex;
  QHash<int, int> m_MapVolEnumToIndex;
};

#endif
