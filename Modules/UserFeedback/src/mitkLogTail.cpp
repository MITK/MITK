/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkLogTail.h>

#include <mitkExceptionMacro.h>

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <string_view>

namespace
{
  /** Paths in messages as UTF-8: path::string() throws on Windows for
   *  characters outside the ANSI code page. */
  std::string ToUtf8(const std::filesystem::path& path)
  {
    const auto utf8 = path.u8string();
    return std::string(utf8.begin(), utf8.end());
  }

  bool IsUtf8Continuation(char byte)
  {
    return (static_cast<unsigned char>(byte) & 0xC0) == 0x80;
  }

  /** Position just after the \p n-th '\n' counted from the end of \p text,
   *  or npos if \p text has fewer. */
  std::size_t AfterNthNewlineFromEnd(std::string_view text, std::size_t n)
  {
    auto position = text.size();

    for (std::size_t found = 0; found < n; ++found)
    {
      if (position == 0)
        return std::string::npos;

      position = text.rfind('\n', position - 1);

      if (position == std::string::npos)
        return std::string::npos;
    }

    return position + 1;
  }
}

mitk::LogTail mitk::ReadLogTail(const std::filesystem::path& path, std::size_t maxLines, std::size_t maxBytes)
{
  std::ifstream file(path, std::ios::binary);

  if (!file.is_open())
    mitkThrow() << "Cannot open log file " << ToUtf8(path);

  file.seekg(0, std::ios::end);
  const std::streamoff fileSize = file.tellg();

  if (fileSize < 0)
    mitkThrow() << "Cannot read log file " << ToUtf8(path);

  LogTail tail;

  if (fileSize == 0 || maxLines == 0 || maxBytes == 0)
  {
    tail.truncated = fileSize != 0;
    return tail;
  }

  // The window ends at the size seen above, not at the end of the file: the
  // log of the current session is still being appended to. The byte before
  // the window tells whether the window starts at a line start.
  const auto size = static_cast<std::uintmax_t>(fileSize);
  const auto windowStart = size > maxBytes ? size - maxBytes : std::uintmax_t{0};
  const bool hasPredecessor = windowStart > 0;
  const auto readStart = hasPredecessor ? windowStart - 1 : windowStart;

  std::string buffer(static_cast<std::size_t>(size - readStart), '\0');
  file.seekg(static_cast<std::streamoff>(readStart));

  if (!file.read(buffer.data(), static_cast<std::streamsize>(buffer.size())))
    mitkThrow() << "Cannot read log file " << ToUtf8(path);

  const std::size_t windowBegin = hasPredecessor ? 1 : 0;

  // A final newline ends the last line instead of starting another.
  const auto lastLineEnd = buffer.back() == '\n' ? buffer.size() - 1 : buffer.size();
  const auto lines = std::string_view(buffer).substr(windowBegin, lastLineEnd - windowBegin);

  std::size_t keepFrom = windowBegin;
  const auto afterNth = AfterNthNewlineFromEnd(lines, maxLines);

  if (afterNth != std::string::npos)
  {
    keepFrom += afterNth;
    tail.truncated = true;
  }
  else if (hasPredecessor)
  {
    tail.truncated = true;

    // A line the byte cap cuts through is dropped. If the window has no line
    // end to cut at, it lies within one line, of which the end is kept.
    if (buffer.front() != '\n')
    {
      const auto firstLineEnd = lines.find('\n');

      if (firstLineEnd != std::string_view::npos)
      {
        keepFrom += firstLineEnd + 1;
      }
      else
      {
        // A UTF-8 sequence has at most three continuation bytes.
        const auto lastByte = buffer.size() - 1;

        for (int skipped = 0; skipped < 3 && keepFrom < lastByte && IsUtf8Continuation(buffer[keepFrom]); ++skipped)
          ++keepFrom;
      }
    }
  }

  buffer.erase(0, keepFrom);

  if (buffer.back() != '\n')
    buffer += '\n';

  tail.text.reserve(buffer.size());

  for (std::size_t i = 0; i < buffer.size(); ++i)
  {
    if (buffer[i] == '\r' && i + 1 < buffer.size() && buffer[i + 1] == '\n')
      continue;

    tail.text += buffer[i];
  }

  tail.linesTaken = static_cast<std::size_t>(std::count(tail.text.begin(), tail.text.end(), '\n'));

  return tail;
}
