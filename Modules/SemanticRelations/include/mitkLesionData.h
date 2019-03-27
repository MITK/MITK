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

#ifndef MITKLESIONDATA_H
#define MITKLESIONDATA_H

#include <MitkSemanticRelationsExports.h>

// mitk semantic relations module
#include "mitkSemanticTypes.h"

// c++
#include <vector>

namespace mitk
{
  /**
  * @brief This class holds the data of each lesion in the lesion tree view.
  *
  */
  class MITKSEMANTICRELATIONS_EXPORT LesionData
  {
  public:
    /**
    * @brief sets the data members to their initial values
    */
    LesionData(const SemanticTypes::Lesion& lesion = SemanticTypes::Lesion());
    ~LesionData();

    SemanticTypes::Lesion GetLesion() const { return m_Lesion; };
    SemanticTypes::ID GetLesionUID() const { return m_Lesion.UID; }
    std::string GetLesionName() const { return m_Lesion.name; }

    const std::vector<bool>& GetLesionPresence() const { return m_LesionPresence; };
    const std::vector<double>& GetLesionVolume() const { return m_LesionVolume; };

    void SetLesion(const SemanticTypes::Lesion& lesion);
    void SetLesionPresence(const std::vector<bool>& lesionPresence);
    void SetLesionVolume(const std::vector<double>& lesionVolume);

  private:

    SemanticTypes::Lesion m_Lesion;
    std::vector<bool> m_LesionPresence;
    std::vector<double> m_LesionVolume;

  };
} // end namespace

#endif // MITKLESIONDATA_H
