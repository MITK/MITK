/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkUserManual.h"

#include <mitkLog.h>

#include <ctkPlugin.h>
#include <ctkPluginContext.h>

#include <QDir>
#include <QHelpEngineCore>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextDocument>

#include <algorithm>
#include <set>

namespace
{
  constexpr std::size_t MaxFullSections = 3;
  constexpr std::size_t MaxListedSections = 15;
  constexpr qsizetype MaxSearchSectionLength = 6000;
  constexpr qsizetype MaxReadSectionLength = 20000;

  /** Pages that Doxygen generates for navigation; they hold no part of the manual. */
  bool IsNavigationPage(const QString& fileName)
  {
    return fileName == "index.html" || fileName == "pages.html" || fileName == "doxygen_crawl.html" || fileName.startsWith("dir_");
  }
}

QString QmitkUserManual::Load(ctkPluginContext* context)
{
  if (m_Loaded)
    return QString();

  if (context == nullptr)
    return "The REST API plugin is not active.";

  // Start afresh, so that the help of plugins removed since does not linger.
  QDir directory(context->getDataFile("manual").absoluteFilePath());
  directory.removeRecursively();

  // The path names the user's profile, so it goes to the log and not into
  // the returned message, which a client may pass on.
  if (!directory.mkpath("."))
  {
    MITK_ERROR << "Cannot create " << directory.absolutePath().toStdString() << " for the user manual.";
    return "Cannot create the directory for the copy of the help files.";
  }

  QHelpEngineCore engine(directory.filePath("manual.qhc"));
  engine.setReadOnly(false);

  if (!engine.setupData())
  {
    MITK_ERROR << "Cannot open the help collection of the user manual: " << engine.error().toStdString();
    return "Cannot open the help collection.";
  }

  constexpr auto ReadableStates = ctkPlugin::RESOLVED | ctkPlugin::STARTING | ctkPlugin::ACTIVE | ctkPlugin::STOPPING;

  for (const auto& plugin : context->getPlugins())
  {
    // The system plugin and plugins that are not resolved have no archive to
    // read resources from; asking them crashes, like the help view knows.
    if (plugin->getPluginId() == 0 || (plugin->getState() & ReadableStates) == 0)
      continue;

    for (const auto& resource : plugin->findResources("/", "*.qch", true))
    {
      const auto path = directory.filePath(QString("%1_%2").arg(plugin->getPluginId()).arg(resource.section('/', -1)));
      QFile file(path);

      if (!file.open(QIODevice::WriteOnly))
        continue;

      file.write(plugin->getResource(resource));
      file.close();

      engine.registerDocumentation(path);
    }
  }

  for (const auto& namespaceName : engine.registeredDocumentations())
  {
    for (const auto& url : engine.files(namespaceName, QString(), "html"))
    {
      if (!IsNavigationPage(url.fileName()))
        this->AddPage(QString::fromUtf8(engine.fileData(url)));
    }
  }

  m_Loaded = true;
  return QString();
}

bool QmitkUserManual::IsLoaded() const
{
  return m_Loaded;
}

bool QmitkUserManual::IsEmpty() const
{
  return m_Entries.empty();
}

void QmitkUserManual::AddPage(const QString& html)
{
  QTextDocument document;
  document.setHtml(html);

  const auto page = document.metaInformation(QTextDocument::DocumentTitle).trimmed();
  Entry entry = { page, page, QString() };

  auto addEntry = [this, &entry]() {
    if (!entry.Text.trimmed().isEmpty())
      m_Entries.push_back(entry);
  };

  for (auto block = document.begin(); block.isValid(); block = block.next())
  {
    auto text = block.text();
    text.remove(QChar::ObjectReplacementCharacter);
    text = text.trimmed();

    if (block.blockFormat().headingLevel() > 0)
    {
      addEntry();
      entry = { page, text, QString() };
      continue;
    }

    if (!text.isEmpty())
      entry.Text += text + '\n';
  }

  addEntry();
}

QmitkUserManual::Section QmitkUserManual::MakeSection(std::size_t id, qsizetype maxLength) const
{
  const auto& entry = m_Entries[id];

  Section section;
  section.Id = id;
  section.Page = entry.Page;
  section.Title = entry.Title;

  if (maxLength > 0)
  {
    section.IsCut = entry.Text.size() > maxLength;

    // A cut between the halves of a surrogate pair would leave half a character.
    const auto length = section.IsCut && entry.Text.at(maxLength - 1).isHighSurrogate() ? maxLength - 1 : maxLength;
    section.Text = entry.Text.left(length);
  }

  return section;
}

QmitkUserManual::SearchResult QmitkUserManual::Search(const QString& query) const
{
  static const QRegularExpression separators("[^\\w]+", QRegularExpression::UseUnicodePropertiesOption);

  std::set<QString> words;

  for (const auto& word : query.toLower().split(separators, Qt::SkipEmptyParts))
  {
    if (word.size() >= 3)
      words.insert(word);
  }

  std::vector<std::pair<int, std::size_t>> scores;

  for (std::size_t i = 0; i < m_Entries.size() && !words.empty(); ++i)
  {
    // The first section of a page is titled like the page, whose name counts once.
    const auto& entry = m_Entries[i];
    const auto title = (entry.Title == entry.Page ? entry.Page : entry.Page + ' ' + entry.Title).toLower();
    const auto text = m_Entries[i].Text.toLower();
    int score = 0;

    for (const auto& word : words)
    {
      const auto inTitle = title.count(word);
      const auto inText = std::min<qsizetype>(text.count(word), 10);

      // A word that occurs at all outweighs most repetitions of another one,
      // so sections that contain more of the words tend to come first.
      if (inTitle + inText > 0)
        score += 20;

      score += static_cast<int>(5 * inTitle + inText);
    }

    if (score > 0)
      scores.emplace_back(score, i);
  }

  std::stable_sort(scores.begin(), scores.end(), [](const auto& a, const auto& b) { return a.first > b.first; });

  SearchResult result;
  result.WordCount = words.size();
  result.MatchCount = scores.size();

  for (std::size_t i = 0; i < std::min(MaxFullSections + MaxListedSections, scores.size()); ++i)
  {
    if (i < MaxFullSections)
    {
      result.Sections.push_back(this->MakeSection(scores[i].second, MaxSearchSectionLength));
    }
    else
    {
      result.OtherSections.push_back(this->MakeSection(scores[i].second, 0));
    }
  }

  return result;
}

std::optional<QmitkUserManual::Section> QmitkUserManual::GetSection(std::size_t id) const
{
  if (id >= m_Entries.size())
    return std::nullopt;

  return this->MakeSection(id, MaxReadSectionLength);
}
