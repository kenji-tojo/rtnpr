#include "viewer.h"

#include <iostream>

#include <Eigen/Dense>

int main()
{
    using namespace std;
    using namespace Eigen;
    using namespace rtnpr;

    test_log();
    Vector3f v(1,1,1);
    cout << v << endl;
}