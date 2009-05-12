#include "Step6RegionGrowing.txx"

/// Instantiate the access function for 2D and all datatypes.
/// Instantiation for 3D is done in Step6RegionGrowing2.
/// Some compilers have memory problems without the explicit 
/// instantiation, some even need the separation in 2D/3D.
InstantiateAccessFunctionForFixedDimension_1(RegionGrowing, 2, Step6*) 

/**
\example Step6RegionGrowing1.cpp
*/

