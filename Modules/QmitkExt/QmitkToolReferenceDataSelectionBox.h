/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-09-12 15:46:48 +0200 (Fr, 12 Sep 2008) $
Version:   $Revision: 15236 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkToolReferenceDataSelectionBox_h_Included
#define QmitkToolReferenceDataSelectionBox_h_Included

#include "mitkToolManager.h"
#include "mitkDataStorage.h"

#include <QWidget>
#include <QVBoxLayout>

class QmitkDataStorageComboBox;

/**
\brief Display the data selection of a ToolManager.

\sa mitk::ToolManager
\sa mitk::DataStorage

\ingroup Reliver
\ingroup Widgets

There is a separate page describing the general design of QmitkInteractiveSegmentation: \ref QmitkInteractiveSegmentationTechnicalPage

Shows the reference data of a ToolManager in a segmentation setting.

The reference image can be selected from a combobox, where all images of the scene are listed.

$Author: maleike $
*/
class QMITKEXT_EXPORT QmitkToolReferenceDataSelectionBox : public QWidget
{
  Q_OBJECT

public:

  /**
  * \brief What kind of items should be displayed.
  *
  * Every mitk::Tool holds a NodePredicateBase object, telling the kind of data that this
  * tool will successfully work with. There are two ways that this list box deals with
  * these predicates.
  *
  *   DEFAULT is: list data if ANY one of the displayed tools' predicate matches.
  * Other option: list data if ALL one of the displayed tools' predicate matches
  */
  enum DisplayMode { ListDataIfAllToolsMatch, ListDataIfAnyToolMatches};

  QmitkToolReferenceDataSelectionBox(QWidget* parent = 0);
  virtual ~QmitkToolReferenceDataSelectionBox();
 
  /// initialization with a data storage object
  void Initialize(mitk::DataStorage*);

  void UpdateDataDisplay();

  mitk::ToolManager* GetToolManager();
  void SetToolManager(mitk::ToolManager&); // no NULL pointer allowed here, a manager is required

  void OnToolManagerReferenceDataModified();

  /**
  * \brief No brief description.
  *
  * Should be called to restrict the number of tools that are
  * evaluated to build up the list. Default is to ask all tools for their predicate, by
  * setting the 'groups' string this can be restricted to certain groups of tools
  * or single tools.
  */
  void SetToolGroupsForFiltering(const std::string& groups);

  /**
  * \brief How the list contents is determined.
  * 
  * See also documentation of DisplayMode.
  *
  * \sa DisplayMode
  */
  void SetDisplayMode( DisplayMode mode );


signals:

  void ReferenceNodeSelected(const mitk::DataTreeNode*);

protected slots:

  void OnReferenceDataSelected(const mitk::DataTreeNode* node);

  void EnsureOnlyReferenceImageIsVisibile();

protected:

  mitk::DataStorage::SetOfObjects::ConstPointer GetAllPossibleReferenceImages();
  
  mitk::NodePredicateBase::ConstPointer GetAllPossibleReferenceImagesPredicate();

  mitk::ToolManager::Pointer m_ToolManager;

  QmitkDataStorageComboBox* m_ReferenceDataSelectionBox;

  bool m_SelfCall;

  DisplayMode m_DisplayMode;
  std::string m_ToolGroupsForFiltering;

  QVBoxLayout* m_Layout;
};

#endif
