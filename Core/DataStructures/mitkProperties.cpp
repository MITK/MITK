#include "mitkProperties.h"

int operator!=(const Point3<int>& left,
                    const Point3<int>& right) {
       return ! right.equals(left);
}
int operator!=(const Point3d& left,
                    const Point3d& right) {
       return ! right.equals(left);
} 

