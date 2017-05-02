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

#ifndef MITKWHITELISTREADEROPTIONSFUNCTOR_H
#define MITKWHITELISTREADEROPTIONSFUNCTOR_H

#include <MitkCoreExports.h>
#include <mitkIOUtil.h>

#include <vector>
#include <string>


namespace mitk
{
  /**
   * \ingroup IO
   *
   * \brief Option callback functor with a white list/ black list option selection strategy.
   *
   * This functor can be used if a option selection should be done without user interaction.
   * Setting up the functor one can specify a white and black lists of controller descriptions.
   * Any controller description on the black list will be ignored and never selected.
   * The first controller description found on the white list will be selected.
   *
   * \see IOUtil
   */
  struct MITKCORE_EXPORT WhiteListReaderOptionsFunctor : public IOUtil::ReaderOptionsFunctorBase
  {
    using ListType = std::vector<std::string>;

    virtual bool operator()(IOUtil::LoadInfo &loadInfo) const override;

    WhiteListReaderOptionsFunctor();
    WhiteListReaderOptionsFunctor(const ListType& white, const ListType& black);

  protected:
    ListType m_WhiteList;
    ListType m_BlackList;
  };
}

#endif // MITKWHITELISTREADEROPTIONSFUNCTOR_H
