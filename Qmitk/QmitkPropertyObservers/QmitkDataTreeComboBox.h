#ifndef QMITK_DATATREECOMBOBOX_H_INCLUDED_DTdD
#define QMITK_DATATREECOMBOBOX_H_INCLUDED_DTdD

/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include <qcombobox.h>
#include <mitkDataTreeFilter.h>
#include <vector>

/// @brief Displays items of a mitk::DataTree
/// @ingroup widgets
class QmitkDataTreeComboBox : public QComboBox
{
  Q_OBJECT;

  public:

    typedef mitk::DataTreeFilter::PropertyList PropertyList;

    QmitkDataTreeComboBox(QWidget* parent = 0, const char* name = 0);
    QmitkDataTreeComboBox(mitk::DataTreeFilter* filter,QWidget* parent = 0, const char* name = 0);
    QmitkDataTreeComboBox(mitk::DataTreeBase* filter,QWidget* parent = 0, const char* name = 0);
    QmitkDataTreeComboBox(mitk::DataTreeIteratorBase* filter,QWidget* parent = 0, const char* name = 0);
    ~QmitkDataTreeComboBox();

    void SetDataTree(mitk::DataTreeBase*);
    void SetDataTree(mitk::DataTreeIteratorBase*);

    void SetFilter(mitk::DataTreeFilter*);
    mitk::DataTreeFilter* GetFilter();

    void SetDisplayedProperty(std::string);

    // handler for event notification
    void removeItemHandler( const itk::EventObject& e );
    void removeChildrenHandler( const itk::EventObject& e );
    void removeAllHandler( const itk::EventObject& e );
    void selectionChangedHandler( const itk::EventObject& e );
    void itemAddedHandler( const itk::EventObject& e );
    void updateAllHandler( const itk::EventObject& e );
   
  signals:
    void activated(const mitk::DataTreeFilter::Item*);
    //void selectionCleared();

  protected:

    void initialize();
    void generateItems();

    void clearItems();

  protected slots:
    void onActivated(int);
    
  private:

    void QmitkDataTreeComboBox::AddItemsToList(const mitk::DataTreeFilter::ItemList* items,
                                               const mitk::DataTreeFilter::PropertyList& visibleProps,
                                               int level);

    void connectNotifications();
    void disconnectNotifications();

    void determineDisplayedProperty();
    
    mitk::DataTreeFilter* m_DataTreeFilter;

    mitk::DataTreeFilter::Pointer m_PrivateFilter;
    const mitk::DataTreeFilter::Item* m_SkipItem;
    const mitk::DataTreeFilter::Item* m_SkipItemParent;

    unsigned long  m_RemoveItemConnection;
    unsigned long  m_RemoveChildrenConnection;
    unsigned long  m_RemoveAllConnection;
    unsigned long  m_SelectionChangedConnection;
    unsigned long  m_ItemAddedConnection;
    unsigned long  m_UpdateAllConnection;

    bool m_SelfCall;

    std::vector<const mitk::DataTreeFilter::Item*> m_Items;
    
    std::string m_UserSetDisplayedProperty;
    std::string m_DisplayedProperty;
};

#endif

