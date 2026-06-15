/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkUGCombinedRepresentationPropertyWidget_h
#define QmitkUGCombinedRepresentationPropertyWidget_h

#include <MitkQtWidgetsExtExports.h>

#include <QComboBox>

namespace mitk
{
  class GridVolumeMapperProperty;
  class GridRepresentationProperty;
  class BoolProperty;
}

class _UGCombinedEnumPropEditor;
class _UGCombinedBoolPropEditor;

/**
 * \brief Combined combo box editor for unstructured grid representation and volume mapper properties.
 *
 * Merges grid representation options (wireframe, surface, etc.) and volume
 * rendering mapper options into a single combo box. Volume mapper entries
 * are prefixed with "Volume (...)". Selecting a surface representation
 * disables volume mode; selecting a volume entry enables it.
 *
 * \sa mitk::GridRepresentationProperty, mitk::GridVolumeMapperProperty
 */
class MITKQTWIDGETSEXT_EXPORT QmitkUGCombinedRepresentationPropertyWidget : public QComboBox
{
  Q_OBJECT

public:
  /**
   * \brief Construct the widget.
   * \param[in] parent The parent widget.
   */
  QmitkUGCombinedRepresentationPropertyWidget(QWidget *parent = nullptr);

  /** \brief Destructor. */
  ~QmitkUGCombinedRepresentationPropertyWidget() override;

  /**
   * \brief Bind the widget to grid representation, volume mapper, and volume toggle properties.
   * \param[in] gridRepresentation The grid representation enumeration property.
   * \param[in] volumeMapper The grid volume mapper enumeration property.
   * \param[in] volumeProp The boolean property toggling volume rendering on/off.
   */
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
