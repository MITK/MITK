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

/**
 @brief Displays items of a DataTreeFilter
 @ingroup Widgets

 \attention This class is meant as a replacement for QmitkTreeNodeSelector, as it provides the same functionality but better separation of GUI and non-GUI tasks.  
            Please read the documentation of mitk::DataTreeFilter before you read this page!
 
 \section sectionQmitkDataTreeComboBoxOverview Overview
 
 This class provides a graphical view of a mitk::DataTreeFilter. It displays all the contained items in a combo box 
 (indicating the hierarchie only by dashes) and allows for single-selections.

 We turned the default behaviour of this class to NOT update everytime something in the data tree changes, because
 that would lead to problems with functionalities that are deactivated. So if you use this class inside your functionality,
 you have to call Update() every now and then (e.g. in the functionality's TreeChanged() method).  You can turn the 
 automatic update on by calling SetAutoUpdate(true).

 \section sectionQmitkDataTreeComboBoxUsage Usage

 There are two principal ways to initialize this class. Either you create a
 DataTreeFilter and then tell the QmitkDataTreeComboBox about it, or you let the
 view itself create a tree filter by giving it a pointer to the data tree. You
 can always access (and modify) the underlying DataTreeFilter through
 GetFilter().

 \subsection sectionQmitkDataTreeComboBoxInit1 Initialization 1: using a ready-made DataTreeFilter

 \e You own both the tree filter and the combo box widget:
\code
  mitk::DataTreeFilter* treeFilter = bar();  // see documentation of DataTreeFilter 
                                            //  on how to configure this class

  if ( ) // you are just creating your widgets
  {
    QmitkDataTreeComboBox* comboBox = new QmitkDataTreeComboBox(treeFilter, this); // create a new Qt widget
  }
  else   // widgets already exists
  {
    comboBox->SetFilter( treeFilter );
  }
\endcode
 
 \subsection sectionQmitkDataTreeComboBoxInit2 Initialization 2: implicit creation of a DataTreeFilter

 You own the view widget, the view widget owns the tree filter:
\code
  if ( ) // you are just creating your widgets
  {
    QmitkDataTreeListView* comboBox = new QmitkDataTreeListView( GetDataTree(), this); // create a new Qt widget
  }
  else   // widgets already exists
  {
    comboBox->SetDataTree( GetDataTree() );
  }
\endcode

  \warning With this usage, the default is, that the content of the combobox only updates when you ask explicitly for it (call Update()).
           You can turn ON automatic updates by calling GetFilter()->SetAutoUpdate(true)
 
\subsection sectionQmitkDataTreeComboBoxInit5 Signals and when they are emited

 - \e activated(item, bool) is emitted, whenever the selection of any item in the list changes. This can occur as
      a result of user interaction with this widget, of due to a change in the underlying DataTreeFilter or the DataTree itself.
      The bool parameter will tell you if the affected item is now selected or not.
 
 - \e clicked(item, bool) is emitted, whenever the selection of any item in the list changes due to
      user interaction with this widget. The bool parameter will tell you if the affected item is now selected or not.

 \todo{This class is still work in progress: if further signals are required by anyone, file bugs or mail your requirements.}
 
 \section sectionQmitkDataTreeComboBoxImplementation Implementation
 \subsection sectionQmitkDataTreeComboBoxImplementation1 Synchronization with the tree filter

 All communication from the DataTreeFilter to this view is done via itk::Events. I.e. the QmitkDataTreeComboBox
 installs a handful of listeners on a given DataTreeFilter and reacts to changes (see the ...Handler() methods).

 The only communication from this view to the DataTreeFilter is the selection state of single items. To change this,
 mitk::DataTreeFilter::Item::SetSelected() is called on the appropriate items.

 \subsection sectionQmitkDataTreeComboBoxImplementation2 Determining what is displayed

 As this widget is basically a combo box, it can display only a single property of the DataTreeNodes. 
 You can have full control over what is displayed (if you want that): use SetDisplayedProperty to set which property is
 used to get a string describing each DataTreeNode. If you omit calling this function, the logic to
 determine a property is as follows:

 - Ask the contained DataTreeFilter about visible properties
 - Iterate over all visible properties of the \e last item in the current item
   list of the filter. For each property of this last item, test whether it is a 
   StringProperty (by doing a dynamic_cast).
 - Stop when the first StringProperty is found, and use this property on all items to fill the combo box.
 - When no StringProperty is found, try the "name" property

*/

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

    void SetAutoUpdate(bool);
    /// Regenerates contents
    void Update();
   
  signals:
    void activated(const mitk::DataTreeFilter::Item*);
    void selectionCleared();

  protected:

    void initialize();
    void generateItems();

    void clearItems();

  protected slots:
    void onActivated(int);
    
  private:

    void AddItemsToList(const mitk::DataTreeFilter::ItemList* items,
                                               const mitk::DataTreeFilter::PropertyList& visibleProps,
                                               int level);

    void connectNotifications();
    void disconnectNotifications();

    void determineDisplayedProperty();
    
    mitk::DataTreeFilter::Pointer m_DataTreeFilter;

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
    
    const mitk::DataTreeNode* m_CurrentNode;

    bool m_AutoUpdate;
};

#endif

