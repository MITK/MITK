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

#ifndef MITKPREFERENCELISTREADEROPTIONSFUNCTOR_H
#define MITKPREFERENCELISTREADEROPTIONSFUNCTOR_H

#include <MitkCoreExports.h>
#include <mitkIOUtil.h>

#include <vector>
#include <string>


namespace mitk
{
  /**
   * \ingroup IO
   *
   * \brief Option callback functor with a preference list/ black list option selection strategy.
   *
   * This functor can be used if a option selection should be done without user interaction.
   * Setting up the functor one can specify a preference and black lists of controller descriptions.
   * Any controller description on the black list will be ignored and never selected.
   * The first controller description found on the preference list will be selected.
   * Any controller listed on the black list is always ignored. Even if it is also
   * listed on the preference list.
   * If no preference listed controller is available, the functor will use the pre selected reader.
   * If no pre selected controller is available, the functor will use the first not black
   * listed reader.
   *
   * \see IOUtil
   */
  struct MITKCORE_EXPORT PreferenceListReaderOptionsFunctor : public IOUtil::ReaderOptionsFunctorBase
  {
    using ListType = std::vector<std::string>;

    bool operator()(IOUtil::LoadInfo &loadInfo) const override;

    PreferenceListReaderOptionsFunctor();
    PreferenceListReaderOptionsFunctor(const ListType& preference, const ListType& black);

  protected:
    ListType m_PreferenceList;
    ListType m_BlackList;
  };
}

#endif // MITKWHITELISTREADEROPTIONSFUNCTOR_H
