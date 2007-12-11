/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef QmitkPropertyListPopuph_included_dingeling
#define QmitkPropertyListPopuph_included_dingeling

#include <qpopupmenu.h>

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
class QmitkPropertyListPopup : public QObject
{
  Q_OBJECT

  public:
    
    QmitkPropertyListPopup( mitk::PropertyList*, QObject* parent = 0, bool disableBoolProperties = false, bool fillMenuImmediatelty = true, const char* name = 0 );
    
    virtual ~QmitkPropertyListPopup();
    
    void popup( const QPoint& pos, int indexAtPoint = -1 );

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
    void popupMenuItemHighlighted(int id);

  protected:

    virtual bool AddMaterialPopup();
    virtual void UpdateNodeMaterialOnPopupHiding( bool& changes );

    QPopupMenu* m_PopupMenu;

    mitk::PropertyList::Pointer m_PropertyList;
    std::vector<mitk::BoolProperty::Pointer> m_BoolProperties;
    
    QPopupMenu* m_InfoPopup;

    QmitkMaterialEditor* m_MaterialEditor;
    
    int m_NameMenuID;
    int m_VisibleMenuID;
    int m_ColorMenuID;
    int m_MaterialMenuID;
    int m_OpacityMenuID;

    bool m_AcceptOnHide;

    mitk::MaterialProperty::Pointer m_OriginalMaterial;
    mitk::FloatProperty::Pointer m_OriginalOpacity;

    bool m_DisableBoolProperties;
};

#endif

