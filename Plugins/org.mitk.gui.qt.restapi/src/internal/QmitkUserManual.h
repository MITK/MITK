/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkUserManual_h
#define QmitkUserManual_h

#include <QString>

#include <optional>
#include <vector>

class ctkPluginContext;

/**
 * \brief The user manual of the application, searchable by section.
 *
 * Reads the Qt Help files that the plugins ship, the same ones the help
 * view shows, and splits their pages into sections at the headings.
 */
class QmitkUserManual
{
public:
  struct Section
  {
    /** \brief Stable while the application runs. */
    std::size_t Id = 0;

    QString Page;
    QString Title;

    /** \brief The text, possibly cut off; empty in the other matches of a search. */
    QString Text;

    bool IsCut = false;
  };

  struct SearchResult
  {
    std::size_t MatchCount = 0;

    /** \brief The best matches, with their text. */
    std::vector<Section> Sections;

    /** \brief The next matches, without their text. */
    std::vector<Section> OtherSections;
  };

  /**
   * \brief Read the help files of all plugins, unless they are read already.
   *
   * Runs in the GUI thread.
   *
   * \param context The context of a plugin, whose data directory takes a
   *        private copy of the help files.
   * \return An error message, or an empty string on success.
   */
  QString Load(ctkPluginContext* context);

  bool IsLoaded() const;

  /**
   * \brief The sections that match the words of the query best.
   *
   * Words shorter than three letters do not count.
   */
  SearchResult Search(const QString& query) const;

  std::optional<Section> GetSection(std::size_t id) const;

private:
  void AddPage(const QString& html);
  Section MakeSection(std::size_t id, qsizetype maxLength) const;

  struct Entry
  {
    QString Page;
    QString Title;
    QString Text;
  };

  std::vector<Entry> m_Entries;
  bool m_Loaded = false;
};

#endif
