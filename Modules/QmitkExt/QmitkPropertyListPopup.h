/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-13 14:07:17 +0100 (Mi, 13 Feb 2008) $
Version:   $Revision: 13599 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef QmitkPropertyListPopuph_included_dingeling
#define QmitkPropertyListPopuph_included_dingeling

//!mm,update: QPopupMenu is deprecated, use QtMenu in Qt4
//#include <Q3PopupMenu>
#include <QMenu>
//!

#include "QmitkMaterialEditor.h" // by Max
#include "mitkMaterialProperty.h"

#include "mitkPropertyList.h"
#include "mitkProperties.h"

class QmitkMaterialEditor;

/*! 
  \class QmitkPropertyListPopUp

  @ingroup Widgets
  @ingroup Reliver
  @ingroup Visualization

  \brief Displays the properties from a mitk::PropertyList.

  This widget gets a mitk::PropertyList in its constructor, changes its elements like "color", "visible", "material" etc.
  When all changes are done successfully, clients are notified by the signal propertyListChangesDone.

*/
class QMITKEXT_EXPORT QmitkPropertyListPopup : public QObject
{
  Q_OBJECT

  public:
    
    QmitkPropertyListPopup( mitk::PropertyList*, QObject* parent = 0, bool disableBoolProperties = false, bool fillMenuImmediatelty = true, const char* name = 0 );
    
    virtual ~QmitkPropertyListPopup();
    
    //!mm
    //void popup( const QPoint& pos, int indexAtPoint = -1 );
    void popup( const QPoint& pos, QAction* action = 0 );
    //!

    void fillPopup();

  signals:

    void propertyListChangesDone();

  protected slots:

    void onNameClicked();
    void onVisibleClicked();
    void onColorClicked();
    void onBoolPropertyClicked(int);
    virtual void MaterialEditorChangesAccepted(QmitkMaterialEditor* ed);
    virtual void popupAboutToHide();
    //!mm
    //void popupMenuItemHighlighted(int id);
    void popupMenuItemHovered(QAction* action);
    //!

  protected:

    virtual bool AddMaterialPopup();
    virtual void UpdateNodeMaterialOnPopupHiding( bool& changes );
    //!mm
    QIcon createColorIcon(QColor color);
    //!
    //!mm,update: QPopupMenu is deprecated, use QtMenu in Qt4
    //Q3PopupMenu* m_PopupMenu;
    QMenu* m_PopupMenu;
    //!

    mitk::PropertyList::Pointer m_PropertyList;
    std::vector<mitk::BoolProperty::Pointer> m_BoolProperties;
    
    //!mm,update: QPopupMenu is deprecated, use QtMenu in Qt4
    //Q3PopupMenu* m_PopupMenu;
    QMenu* m_InfoPopup;
    //!

    QmitkMaterialEditor* m_MaterialEditor;
    
    //!mm
    //int m_NameMenuID;
    //int m_VisibleMenuID;
    //int m_ColorMenuID;
    //int m_MaterialMenuID;
    //int m_OpacityMenuID;
    QAction* m_NameMenuAction;
    QAction* m_VisibleMenuAction;
    QAction* m_ColorMenuAction;
    QAction* m_MaterialMenuAction;
    QAction* m_OpacityMenuAction;
    //!

    bool m_AcceptOnHide;

    mitk::MaterialProperty::Pointer m_OriginalMaterial;
    mitk::FloatProperty::Pointer m_OriginalOpacity;

    bool m_DisableBoolProperties;
};

#endif

