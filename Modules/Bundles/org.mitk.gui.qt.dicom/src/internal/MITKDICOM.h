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


#ifndef MITKDICOM_h
#define MITKDICOM_h

#include <berryISelectionListener.h>

#include <QmitkFunctionality.h>

#include "ui_MITKDICOMControls.h"


/*!
  \brief MITKDICOM

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkFunctionality
  \ingroup ${plugin_target}_internal
*/
class MITKDICOM : public QmitkFunctionality
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    MITKDICOM();
    virtual ~MITKDICOM();

    virtual void CreateQtPartControl(QWidget *parent);

  protected slots:

    /// \brief Called when series in TreeView is double clicked.
    void onSeriesModelSelected(const QModelIndex &index);

  protected:

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

    Ui::MITKDICOMControls m_Controls;

};

#endif // MITKDICOM_h

