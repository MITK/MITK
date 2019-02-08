/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKABSTRACTSEMANTICRELATIONSSTORAGEINSPECTOR_H
#define QMITKABSTRACTSEMANTICRELATIONSSTORAGEINSPECTOR_H

// semantic relations UI module
#include "MitkSemanticRelationsUIExports.h"

// semantic relations module
#include "mitkSemanticTypes.h"

// qt widgets module
#include "QmitkAbstractDataStorageInspector.h"

/*
* @brief The QmitkAbstractSemanticRelationsStorageInspector is a QmitkAbstractDataStorageInspector that can be used to
*        show the currently available data of an (abstract) semantic relations storage model.
*/
class MITKSEMANTICRELATIONSUI_EXPORT QmitkAbstractSemanticRelationsStorageInspector : public QmitkAbstractDataStorageInspector
{
  Q_OBJECT

public:

  virtual ~QmitkAbstractSemanticRelationsStorageInspector();

  /**
  * @brief Extends the abstract base class to allow setting the current case ID which is needed to access the
  *        semantic relations storage. The function sets the case ID in the storage model.
  *
  * @param caseID    A case ID as string
  */
  virtual void SetCaseID(const mitk::SemanticTypes::CaseID& caseID) = 0;
  /**
  * @brief Extends the abstract base class to allow setting the current lesion.
  *        The function sets the lesion in the storage model.
  *
  *@param lesion   The selected lesion
  */
  virtual void SetLesion(const mitk::SemanticTypes::Lesion& lesion) = 0;

protected:

  QmitkAbstractSemanticRelationsStorageInspector(QWidget* parent = nullptr);
};

#endif // QMITKABSTRACTSEMANTICRELATIONSSTORAGEINSPECTOR_H
