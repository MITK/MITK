/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkFeedbackMarkdown.h"

#include <optional>
#include <string_view>

namespace
{
  /** The marker character, length and remainder of a line that starts with
   *  a code fence marker. */
  struct FenceMarker
  {
    char character;
    std::size_t length;
    std::string_view rest;
  };

  std::size_t RunLength(std::string_view text, std::size_t start)
  {
    std::size_t end = start;

    while (end < text.size() && text[end] == text[start])
      ++end;

    return end - start;
  }

  std::optional<FenceMarker> ParseFenceMarker(std::string_view line)
  {
    std::size_t indent = 0;

    while (indent < line.size() && line[indent] == ' ')
      ++indent;

    if (indent > 3 || indent == line.size() || (line[indent] != '`' && line[indent] != '~'))
      return std::nullopt;

    const auto length = RunLength(line, indent);

    if (length < 3)
      return std::nullopt;

    return FenceMarker{ line[indent], length, line.substr(indent + length) };
  }

  bool OpensFence(const FenceMarker& marker)
  {
    // A backtick run followed by another backtick on the same line is not a
    // fence; the line is inline text.
    return marker.character == '~' || marker.rest.find('`') == std::string_view::npos;
  }

  bool ClosesFence(const FenceMarker& marker, const FenceMarker& open)
  {
    return marker.character == open.character && marker.length >= open.length &&
           marker.rest.find_first_not_of(" \t\r") == std::string_view::npos;
  }

  /** Start of the first run of exactly \p length backticks at or after
   *  \p from, or npos. */
  std::size_t FindClosingRun(std::string_view line, std::size_t from, std::size_t length)
  {
    std::size_t position = from;

    while (position < line.size())
    {
      if (line[position] != '`')
      {
        ++position;
        continue;
      }

      const auto run = RunLength(line, position);

      if (run == length)
        return position;

      position += run;
    }

    return std::string_view::npos;
  }

  bool StartsHtml(std::string_view line, std::size_t position)
  {
    if (position >= line.size())
      return false;

    const char c = line[position];

    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '/' || c == '!' || c == '?';
  }

  void EscapeLine(std::string_view line, std::string& escaped)
  {
    std::size_t position = 0;

    while (position < line.size())
    {
      const char c = line[position];

      // A '<' that is already escaped must not get a second backslash, which
      // would turn the first one into a literal backslash and leave the '<'
      // live.
      if (c == '\\' && position + 1 < line.size() &&
          (line[position + 1] == '\\' || line[position + 1] == '`' || line[position + 1] == '<'))
      {
        escaped.append(line.substr(position, 2));
        position += 2;
      }
      else if (c == '`')
      {
        // A span is looked for within the line only. Over a line end, a blank
        // line or a block start may cut it, and the text taken for a span
        // would then shield a live '<'. A '<' inside a real multi-line span
        // is at worst escaped needlessly.
        const auto length = RunLength(line, position);
        const auto closing = FindClosingRun(line, position + length, length);
        const auto end = closing == std::string_view::npos ? position + length : closing + length;

        escaped.append(line.substr(position, end - position));
        position = end;
      }
      else if (c == '<' && StartsHtml(line, position + 1))
      {
        escaped.append("\\<");
        ++position;
      }
      else
      {
        escaped += c;
        ++position;
      }
    }
  }
}

std::string mitk::EscapeMarkdownHtml(const std::string& text)
{
  const std::string_view all(text);

  std::string escaped;
  escaped.reserve(text.size());

  std::optional<FenceMarker> openFence;
  std::size_t lineStart = 0;

  while (lineStart < all.size())
  {
    const auto newline = all.find('\n', lineStart);
    const auto lineEnd = newline == std::string_view::npos ? all.size() : newline;
    const auto line = all.substr(lineStart, lineEnd - lineStart);
    const auto marker = ParseFenceMarker(line);

    if (openFence)
    {
      escaped.append(line);

      if (marker && ClosesFence(*marker, *openFence))
        openFence.reset();
    }
    else if (marker && OpensFence(*marker))
    {
      escaped.append(line);
      openFence = marker;
    }
    else
    {
      EscapeLine(line, escaped);
    }

    if (newline != std::string_view::npos)
      escaped += '\n';

    lineStart = lineEnd + 1;
  }

  return escaped;
}
