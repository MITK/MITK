#include "Step6RegionGrowing.txx"

/// Instantiate the access function for 3D and all datatypes.
/// Instantiation for 2D is done in Step6RegionGrowing1.
/// Some compilers have memory problems without the explicit 
/// instantiation, some even need the separation in 2D/3D.
InstantiateAccessFunctionForFixedDimension_1(RegionGrowing, 3, Step6*) 

/**
\example Step6RegionGrowing2.cpp
*/

