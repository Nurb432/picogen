//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Copyright (C) 2009  Sebastian Mach (*1983)
// * mail: phresnel/at/gmail/dot/com
// * http://phresnel.org
// * http://picogen.org
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef SETUP_HH_INCLUDED_20090303
#define SETUP_HH_INCLUDED_20090303

#include <kallisto/common.hh>

#define DefineFinalizer(x) \
        class x##Finalize { \
        private: \
                friend class x ; \
                x##Finalize() {} \
                x##Finalize(x##Finalize const &) {} \
        }
#define DoFinalize(x) virtual x##Finalize

namespace redshift {

        enum {
                ALLOW_LOGSPAM = 1
        };

        typedef float real_t;
        typedef kallisto::fixed_point_t<int64_t,16>          fixed_point_t;        

        // Cartesian Types
        typedef kallisto::Point<kallisto::CARTESIAN,
                                fixed_point_t>               CartesianPoint;
        typedef kallisto::Normal<kallisto::CARTESIAN,real_t> CartesianNormal;
        typedef kallisto::Vector<kallisto::CARTESIAN,real_t> CartesianVector;


        // Std-Types
        typedef CartesianPoint  Point;
        typedef CartesianNormal Normal;
        typedef CartesianVector Vector;

        typedef kallisto::Sphere<Point,long double>  Sphere;
        typedef kallisto::Ray<Point,Vector>          Ray;
        typedef kallisto::OrthonormalBasis<Vector>   OrthonormalBasis;

        class Rgb;
        typedef Rgb Color;
        struct IScene {};
}

namespace redshift {
        namespace constants {
                template<typename T> inline T km2m (T const &v) {
                        return T(1000) * v;
                }
                static double const earth_radius_m     = km2m (12756.0);
                static double const sun_radius_m       = km2m (1.392 * 10.0e9);
                static double const jupiter_radius_m   = km2m (133708.0);
                //static float const sun_radius_km 1,3914
        }
}

#endif // SETUP_HH_INCLUDED_20090303