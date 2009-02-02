/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 14147 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#if !defined(QmitkPACS_H__INCLUDED)
#define QmitkPACS_H__INCLUDED

#include "QmitkFunctionality.h"

#include "mitkPACSPlugin.h"

class QmitkStdMultiWidget;

/*!
\brief QmitkPACS 

\sa QmitkFunctionality
\ingroup Functionalities
*/
class QmitkPACS : public QmitkFunctionality
{
  Q_OBJECT

  public:
  
    QmitkPACS(QObject *parent,
              const char *name, 
              QmitkStdMultiWidget *mitkStdMultiWidget, 
              mitk::DataTreeIteratorBase* dataIt);

  virtual ~QmitkPACS();

  virtual QWidget * CreateControlWidget(QWidget *parent);
  
  virtual QWidget * CreateMainWidget(QWidget * parent);

  virtual QAction * CreateAction(QActionGroup *parent);

  virtual void TreeChanged();

protected slots:

  void AbortPACSImport();

protected:

  mitk::PACSPlugin* m_Plugin;

  unsigned long m_ObserverTag;

  QmitkStdMultiWidget* m_MultiWidget;

  QWidget* m_Controls;

  void PluginEventNewStudySelected(const itk::EventObject&);

};
#endif // !defined(QmitkPACS_H__INCLUDED)

