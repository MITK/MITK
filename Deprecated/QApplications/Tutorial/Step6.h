#ifndef STEP6_H
#define STEP6_H

#include <qmainwindow.h>
#include <mitkDataTree.h>
#include <mitkImage.h>
#include <mitkPointSet.h>

#include <itkImage.h>

#ifndef DOXYGEN_IGNORE

class QLineEdit;

class Step6 : public QMainWindow
{
  Q_OBJECT
public:
  Step6( int argc, char* argv[], QWidget *parent=0, const char *name=0 );
  ~Step6() {};

  virtual void Initialize();

  virtual int GetThresholdMin();
  virtual int GetThresholdMax();

protected:
  void Load(int argc, char* argv[]);
  virtual void SetupWidgets();

  template < typename TPixel, unsigned int VImageDimension >
    friend void RegionGrowing( itk::Image<TPixel, VImageDimension>* itkImage, Step6* step6);

  mitk::DataTree::Pointer m_Tree;
  mitk::Image::Pointer m_FirstImage;
  mitk::PointSet::Pointer m_Seeds;

  mitk::Image::Pointer m_ResultImage;
  mitk::DataTreeNode::Pointer m_ResultNode;
  QWidget *m_TopLevelWidget;

  QLineEdit *m_LineEditThresholdMin;
  QLineEdit *m_LineEditThresholdMax;

protected slots:
  virtual void StartRegionGrowing();
};
#endif // DOXYGEN_IGNORE

#endif // STEP6_H

/**
\example Step6.h
*/

