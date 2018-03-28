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

#include <mitkSplitParameterToVector.h>

#include <sstream>

std::vector<double> mitk::cl::splitDouble(std::string str, char delimiter) {
  std::vector<double> internal;
  std::stringstream ss(str); // Turn the string into a stream.
  std::string tok;
  double val;
  while (std::getline(ss, tok, delimiter)) {
    std::stringstream s2(tok);
    s2 >> val;
    internal.push_back(val);
  }

  return internal;
}

std::vector<int> mitk::cl::splitInt(std::string str, char delimiter) {
  std::vector<int> internal;
  std::stringstream ss(str); // Turn the string into a stream.
  std::string tok;
  int val;
  while (std::getline(ss, tok, delimiter)) {
    std::stringstream s2(tok);
    s2 >> val;
    internal.push_back(val);
  }

  return internal;
}

std::vector<std::string> mitk::cl::splitString(std::string str, char delimiter) {
  std::vector<std::string> internal;
  std::stringstream ss(str); // Turn the string into a stream.
  std::string tok;
  std::string val;
  while (std::getline(ss, tok, delimiter)) {
    std::stringstream s2(tok);
    s2 >> val;
    internal.push_back(val);
  }

  return internal;
}