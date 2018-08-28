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

#ifndef MITKLESIONITEMDATA_H
#define MITKLESIONITEMDATA_H

// mitk semantic relations module
#include <mitkSemanticTypes.h>

// qt
#include <QVariant>

namespace mitk
{
  /**
  * @brief This class holds the data of each lesion in the lesion tree view.
  *
  */
  class LesionItemData
  {

  public:
    /**
    * @brief sets the data members to their initial values
    */
    LesionItemData();
    ~LesionItemData();

    std::string GetLesionName() const { return m_Lesion.name; }
    void SetLesion(const mitk::SemanticTypes::Lesion& lesion);

  private:

    mitk::SemanticTypes::Lesion m_Lesion;

  };
} // end namespace

Q_DECLARE_METATYPE(mitk::LesionItemData)

#endif // MITKLESIONITEMDATA_H