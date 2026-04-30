/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPreferenceListReaderOptionsFunctor_h
#define mitkPreferenceListReaderOptionsFunctor_h

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
   * If user options (non empty) are specified for the functor, the selected reader will be set
   * with these user options.
   *
   * \see IOUtil
   */
  struct MITKCORE_EXPORT PreferenceListReaderOptionsFunctor : public IOUtil::ReaderOptionsFunctorBase
  {
    using ListType = std::vector<std::string>;

    /** \brief Applies the preference/black list strategy to select and configure a reader.
     *
     * \param loadInfo The load info containing reader selector and options.
     * \return True if a reader was successfully selected.
     */
    bool operator()(IOUtil::LoadInfo &loadInfo) const override;

    /** \brief Default constructor with empty preference and black lists. */
    PreferenceListReaderOptionsFunctor();

    /** \brief Constructor with preference and black lists.
     *
     * \param preference List of preferred reader descriptions (first match wins).
     * \param black List of blacklisted reader descriptions (always excluded).
     */
    PreferenceListReaderOptionsFunctor(const ListType& preference, const ListType& black);

    /** \brief Constructor with preference list and reader options.
     *
     * \param preference List of preferred reader descriptions.
     * \param options Options to apply to the selected reader.
     */
    PreferenceListReaderOptionsFunctor(const ListType& preference, const IFileReader::Options& options);

    /** \brief Constructor with preference list, black list, and reader options.
     *
     * \param preference List of preferred reader descriptions.
     * \param black List of blacklisted reader descriptions.
     * \param options Options to apply to the selected reader.
     */
    PreferenceListReaderOptionsFunctor(const ListType& preference, const ListType& black, const IFileReader::Options& options);

  protected:
    const ListType m_PreferenceList;
    const ListType m_BlackList;
    const IFileReader::Options m_Options;
  };
}

#endif
