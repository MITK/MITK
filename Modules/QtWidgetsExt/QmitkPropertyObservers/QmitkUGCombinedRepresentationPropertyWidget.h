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
class MitkQtWidgetsExt_EXPORT QmitkUGCombinedRepresentationPropertyWidget : public QComboBox
{
  Q_OBJECT

public:

  QmitkUGCombinedRepresentationPropertyWidget(QWidget *parent = 0);
  ~QmitkUGCombinedRepresentationPropertyWidget();

  void SetProperty(mitk::GridRepresentationProperty* gridRepresentation,
                   mitk::GridVolumeMapperProperty* volumeMapper,
                   mitk::BoolProperty* volumeProp);

protected slots:

  void OnIndexChanged(int index);

protected:

  friend class _UGCombinedEnumPropEditor;
  friend class _UGCombinedBoolPropEditor;

  void SetGridRepresentationId(int enumId);
  void SetGridVolumeId(int enumId);
  void IsVolumeChanged(bool volume);

  _UGCombinedEnumPropEditor* gridRepPropEditor;
  _UGCombinedEnumPropEditor* volumeMapperPropEditor;
  _UGCombinedBoolPropEditor* volumePropEditor;

  int m_GridRepIndex;
  int m_GridVolIndex;

  int m_FirstVolumeRepId;
  QHash<int, int> m_MapRepEnumToIndex;
  QHash<int, int> m_MapVolEnumToIndex;

};

#endif

