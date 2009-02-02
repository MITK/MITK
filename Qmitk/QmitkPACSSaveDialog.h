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

#ifndef QmitkPACSSaveDialog_h_Included
#define QmitkPACSSaveDialog_h_Included

#include "QmitkPACSContentList.h"

#include <qdialog.h>
#include <qspinbox.h>
#include <qlineedit.h>

#include <vector>

/**
 * \brief Dialog for PACS data import
 *
 * \sa QmitkPACSContentList
 */
class QMITK_EXPORT QmitkPACSSaveDialog : public QDialog
{
  Q_OBJECT

  public:
    
    QmitkPACSSaveDialog(QWidget* parent = 0, const char* name = 0);

    virtual ~QmitkPACSSaveDialog();

    void SetDataTreeNodesToSave( std::vector<mitk::DataTreeNode*> nodes );

  signals:

  public slots:
  
    void exec();

    void setAllowSeveralObjectsIntoOneSeries(bool);

  protected slots:

    void UpdateSeriesLists();

    void SaveAsNewSeries();
    void SaveIntoSelectedSeries();

  protected:
  
    void PluginEventNewStudySelected(const itk::EventObject&);

    mitk::PACSPlugin* m_Plugin;
  
    unsigned long m_ObserverTag;
    
    QListView* m_SeriesList;
    std::map<QListViewItem*, mitk::PACSPlugin::SeriesInformation>   m_Series;
    
    /**
     * \warning This static solution is the only way I could make this work
     */
    static std::vector<mitk::DataTreeNode*> m_DataTreeNodesToSave;
  
    bool m_AllowSeveralObjectsIntoOneSeries;
};

class QMITK_EXPORT QmitkPACSSaveUserDialog : public QDialog
{
  Q_OBJECT

  public:
    
    QmitkPACSSaveUserDialog(QWidget* parent = 0, const char* name = 0);

    virtual ~QmitkPACSSaveUserDialog();

    std::pair< int, std::string > AskUserForSeriesDescriptionAndNumber();

  protected:

    QLineEdit* lineedit;
    QSpinBox* spinbox;
};

#endif

