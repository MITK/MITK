/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPropertyListPopuph_included_dingeling
#define QmitkPropertyListPopuph_included_dingeling

#include "MitkQtWidgetsExtExports.h"
#include <QMenu>

#include "mitkProperties.h"
#include "mitkPropertyList.h"

class QmitkMaterialEditor;

/*!
  \class QmitkPropertyListPopUp

  @ingroup Widgets
  @ingroup ToolManagerEtAl
  @ingroup Visualization

  \brief Displays the properties from a mitk::PropertyList.

  This widget gets a mitk::PropertyList in its constructor, changes its elements like "color", "visible", "material"
  etc.
  When all changes are done successfully, clients are notified by the signal propertyListChangesDone.

*/
class MITKQTWIDGETSEXT_EXPORT QmitkPropertyListPopup : public QObject
{
  Q_OBJECT

public:
  QmitkPropertyListPopup(mitk::PropertyList *,
                         QObject *parent = 0,
                         bool disableBoolProperties = false,
                         bool fillMenuImmediatelty = true,
                         const char *name = 0);

  virtual ~QmitkPropertyListPopup();

  void popup(const QPoint &pos, QAction *action = 0);

  void fillPopup();

signals:

  void propertyListChangesDone();

protected slots:

  void onNameClicked();
  void onVisibleClicked();
  void onColorClicked();
  void onBoolPropertyClicked(int);
  virtual void MaterialEditorChangesAccepted(QmitkMaterialEditor *ed);
  virtual void popupAboutToHide();
  void popupMenuItemHovered(QAction *action);

protected:
  virtual bool AddMaterialPopup();
  virtual void UpdateNodeMaterialOnPopupHiding(bool &changes);
  QIcon createColorIcon(QColor color);
  QMenu *m_PopupMenu;

  mitk::PropertyList::Pointer m_PropertyList;
  std::vector<mitk::BoolProperty::Pointer> m_BoolProperties;
  QMenu *m_InfoPopup;

  QAction *m_NameMenuAction;
  QAction *m_VisibleMenuAction;
  QAction *m_ColorMenuAction;
  QAction *m_MaterialMenuAction;
  QAction *m_OpacityMenuAction;

  bool m_AcceptOnHide;

  mitk::FloatProperty::Pointer m_OriginalOpacity;

  bool m_DisableBoolProperties;
};

#endif
