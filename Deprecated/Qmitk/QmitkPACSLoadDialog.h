/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 15236 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef QmitkPACSLoadDialog_h_Included
#define QmitkPACSLoadDialog_h_Included

#include "QmitkPACSContentList.h"

#include <qdialog.h>

#include <vector>

/**
 * \brief Dialog for PACS data import
 *
 * \sa QmitkPACSContentList
 */
class QMITK_EXPORT QmitkPACSLoadDialog : public QDialog
{
  Q_OBJECT

  public:
    
    QmitkPACSLoadDialog(QWidget* parent = 0, bool mainAppFunctionalityIntegration = false,const char* name = 0);

    virtual ~QmitkPACSLoadDialog();

    std::vector<mitk::DataTreeNode*> GetImportedDataTreeNodes();

  signals:

  public slots:

    void SetDataStorageParent( mitk::DataTreeNode* node );
  
    void exec();

  protected slots:

    void ImportSeries();
    void ImportSeriesImages();
    void ImportSeriesDocuments(bool all = false, bool warnIfNoSelection = true);

    void ImportSeriesAndClose();
    void ImportSeriesImagesAndClose();
    void ImportSeriesDocumentsAndClose();


  protected:
  
    void PluginEventNewStudySelected(const itk::EventObject&);

    void DecorateNode(mitk::DataTreeNode* node);
 
    bool m_MainAppFunctionalityIntegration;

    mitk::PACSPlugin* m_Plugin;
  
    unsigned long m_ObserverTag;

    QmitkPACSContentList* m_PACSContentList;

    mitk::DataTreeNode::Pointer m_DataStorageParent;
    
    /**
     * \warning This static solution is the only way I could make this work
     */
    static std::vector<mitk::DataTreeNode*> m_ImportedDataTreeNodes;
};

#endif

