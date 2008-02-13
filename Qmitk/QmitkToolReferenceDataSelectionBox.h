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

#ifndef QmitkToolReferenceDataSelectionBox_h_Included
#define QmitkToolReferenceDataSelectionBox_h_Included

#include <qvbox.h>

#include "mitkToolManager.h"
#include "mitkDataTreeFilter.h"

class QmitkDataTreeComboBox;

/**
  \brief Display the data selection of a ToolManager.

  \sa mitk::ToolManager
  \sa mitk::DataStorage

  \ingroup Reliver
  \ingroup Widgets

  There is a separate page describing the general design of QmitkSliceBasedSegmentation: \ref QmitkSliceBasedSegmentationTechnicalPage

  Shows the reference data of a ToolManager in a segmentation setting.

  The reference image can be selected from a combobox, where all images of the scene are listed.

  $Author$
*/
class QMITK_EXPORT QmitkToolReferenceDataSelectionBox : public QVBox
{
  Q_OBJECT

  public:
    
    QmitkToolReferenceDataSelectionBox(QWidget* parent = 0, const char* name = 0);
    virtual ~QmitkToolReferenceDataSelectionBox();

    void Initialize(mitk::DataTreeBase*); // just needed because combobox relies on data tree

    /**
     TODO this is not a nice place for this function (bug 735)
    */
    void SetToolGroups(const char* toolGroups = 0);
    void UpdateDataDisplay();

    mitk::ToolManager* GetToolManager();
    void SetToolManager(mitk::ToolManager&); // no NULL pointer allowed here, a manager is required

    void OnToolManagerReferenceDataModified(const itk::EventObject&);

  signals:

    void ReferenceNodeSelected(const mitk::DataTreeNode*);

  public slots:
  
  protected slots:

    void OnReferenceDataSelected(const mitk::DataTreeFilter::Item* item);

    void EnsureOnlyReferenceImageIsVisibile();

  protected:

    mitk::DataStorage::SetOfObjects::ConstPointer GetAllPossibleReferenceImages();

    mitk::ToolManager::Pointer m_ToolManager;

    QmitkDataTreeComboBox* m_ReferenceDataSelectionBox;

    unsigned long m_ToolReferenceDataChangedObserverTag;

    bool m_SelfCall;
};

#endif

