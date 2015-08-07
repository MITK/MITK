#pragma once

#include "mitkPlanarFigure.h"
#include <MitkPlanarFigureExports.h>


namespace mitk
{

class PlaneGeometry;

/**
 * \brief Implementation of PlanarFigure representing a line
 * through two control points
 */
class MITKPLANARFIGURE_EXPORT PlanarComment : public PlanarFigure
{
public:
  mitkClassMacro(PlanarComment, PlanarFigure);

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  void setText(const std::string& comment);
  const std::string getText();

  /** \brief Place figure in its minimal configuration (a point at least)
   * onto the given 2D geometry.
   *
   * Must be implemented in sub-classes.
   */
  //virtual void Initialize();


  /** \brief Line has 2 control points per definition. */
  unsigned int GetMinimumNumberOfControlPoints() const
  {
    return 2;
  }


  /** \brief Line has 2 control points per definition. */
  unsigned int GetMaximumNumberOfControlPoints() const
  {
    return 2;
  }

  virtual bool Equals(const mitk::PlanarFigure& other) const;

protected:
  PlanarComment();
  virtual ~PlanarComment();

  mitkCloneMacro(Self);

  /** \brief Generates the poly-line representation of the planar figure. */
  virtual void GeneratePolyLine();

  /** \brief Generates the poly-lines that should be drawn the same size regardless of zoom.*/
  virtual void GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight);

  /** \evaluate annotation based on evaluating features. */
  virtual std::string EvaluateAnnotation();

  /** \brief Calculates feature quantities of the planar figure. */
  virtual void EvaluateFeaturesInternal();


  virtual void PrintSelf( std::ostream &os, itk::Indent indent ) const;

  // Feature identifiers
  const unsigned int FEATURE_ID_LENGTH;


private:

	std::string m_commentText;

};

} // namespace mitk