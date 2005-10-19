#ifndef STEP7_H
#define STEP7_H

#include "Step6.h"

#ifndef DOXYGEN_IGNORE
class Step7 : public Step6
{
  Q_OBJECT
public:
  Step7( int argc, char* argv[], QWidget *parent=0, const char *name=0 );
  ~Step7() {};

protected:
protected slots:
  virtual void StartRegionGrowing();
};
#endif // DOXYGEN_IGNORE

#endif // STEP7_H

/**
\example Step7.h
*/
