/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef __QmitkStdMultiWidgetState_h
#define __QmitkStdMultiWidgetState_h

#include <QmitkStdMultiWidget.h>

static bool EqualsWithTolerance(const mitk::Vector2D& cursorPosition1, const mitk::Vector2D& cursorPosition2, double tolerance = 0.001)
{
  return std::abs(cursorPosition1[0] - cursorPosition2[0]) < tolerance
      && std::abs(cursorPosition1[1] - cursorPosition2[1]) < tolerance;
}

static bool EqualsWithTolerance(const std::vector<mitk::Vector2D>& cursorPositions1, const std::vector<mitk::Vector2D>& cursorPositions2, double tolerance = 0.001)
{
  for (int i = 0; i < 3; ++i)
  {
    if (!::EqualsWithTolerance(cursorPositions1[i], cursorPositions2[i], tolerance))
    {
      return false;
    }
  }
  return true;
}

class QmitkStdMultiWidgetState : public itk::Object
{
public:

  mitkClassMacro(QmitkStdMultiWidgetState, itk::Object);
  mitkNewMacro1Param(QmitkStdMultiWidgetState, const QmitkStdMultiWidget*);
  mitkNewMacro1Param(QmitkStdMultiWidgetState, Self::Pointer);

  // TODO: Put getters and setters here.

  bool operator==(const QmitkStdMultiWidgetState& otherState) const
  {
    // TODO
    return 1;
  }

  inline bool operator!=(const QmitkStdMultiWidgetState& otherState) const
  {
    return !(*this == otherState);
  }

  void PrintDifference(QmitkStdMultiWidgetState::Pointer otherState, std::ostream & os = std::cout, itk::Indent indent = 0) const
  {
    // TODO
  }

protected:

  /// \brief Constructs a QmitkStdMultiWidgetState object that stores the current state of the specified viewer.
  QmitkStdMultiWidgetState(const QmitkStdMultiWidget* viewer)
  : itk::Object()
  // TODO
  {
  }

  /// \brief Constructs a QmitkStdMultiWidgetState object as a copy of another state object.
  QmitkStdMultiWidgetState(Self::Pointer otherState)
  : itk::Object()
  // TODO
  {
  }

  /// \brief Destructs a QmitkStdMultiWidgetState object.
  virtual ~QmitkStdMultiWidgetState()
  {
  }

  /// \brief Prints the collected signals to the given stream or to the standard output if no stream is given.
  virtual void PrintSelf(std::ostream & os, itk::Indent indent) const
  {
    // TODO
  }

private:

  // TODO Declare members here.

};

#endif
