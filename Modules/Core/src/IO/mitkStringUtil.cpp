/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkStringUtil.h>

#include <algorithm>
#include <cctype>

bool mitk::EndsWithCaseInsensitive(const std::string& str, const std::string& suffix)
{
  // Ends-with is a case-insensitive comparison of the trailing suffix.size()
  // characters, so defer to EqualsCaseInsensitive on that tail.
  return str.size() >= suffix.size() &&
         EqualsCaseInsensitive(str.substr(str.size() - suffix.size()), suffix);
}

bool mitk::EqualsCaseInsensitive(const std::string& lhs, const std::string& rhs)
{
  if (lhs.size() != rhs.size())
    return false;

  // std::tolower must be given an unsigned char to avoid undefined behavior on
  // negative char values.
  return std::equal(lhs.begin(), lhs.end(), rhs.begin(), [](char a, char b) {
    return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
  });
}

std::vector<std::string> mitk::Split(const std::string& str, char delimiter)
{
  std::vector<std::string> tokens;
  std::string::size_type start = 0;

  while (true)
  {
    const auto end = str.find(delimiter, start);
    tokens.push_back(str.substr(start, end - start));

    if (std::string::npos == end)
      break;

    start = end + 1;
  }

  return tokens;
}

void mitk::Trim(std::string& str)
{
  const auto notSpace = [](unsigned char c) { return !std::isspace(c); };
  str.erase(str.begin(), std::find_if(str.begin(), str.end(), notSpace));
  str.erase(std::find_if(str.rbegin(), str.rend(), notSpace).base(), str.end());
}

void mitk::ReplaceAll(std::string& str, const std::string& from, const std::string& to)
{
  if (from.empty())
    return;

  for (auto pos = str.find(from); pos != std::string::npos; pos = str.find(from, pos + to.size()))
    str.replace(pos, from.size(), to);
}
