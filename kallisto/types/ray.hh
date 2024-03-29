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

#ifndef RAY_H_INCLUDED_20090223
#define RAY_H_INCLUDED_20090223

namespace kallisto {
        template <typename point_t, typename direction_t> struct ray_t {                
                proto_ray_t<point_t, direction_t> proto_ray;
                typename traits::get_scalar_type<direction_t>::type minT;
                typename traits::get_scalar_type<direction_t>::type maxT;
        };
}



namespace kallisto {
        template <typename point_t, typename direction_t> class Ray
        : public ProtoRay<point_t, direction_t> {
        public:
        
                typename traits::get_scalar_type<direction_t>::type minT;
                typename traits::get_scalar_type<direction_t>::type maxT;

                Ray ()
                : ProtoRay<point_t, direction_t>(), minT(), maxT()                
                {}
                
                Ray (point_t const &pos, direction_t const &dir)
                : ProtoRay<point_t, direction_t>(pos, dir), minT(), maxT()
                {}
                
                point_t operator () (
                        typename traits::get_scalar_type<direction_t>::type f
                ) const {
                        // create a compatible vector-type for our point_t
                        typedef Vector<
                           static_cast<coordinate_space_t>
                                (traits::get_coordinate_space<point_t>::value),
                           typename traits::get_scalar_type<point_t>::type
                        > PV;
                        
                        PV      const tmp = vector_cast<PV>(this->direction*f);
                        point_t const tmp2= this->position;
                        return this->position + tmp;
                }
        };
}



namespace kallisto { namespace unit_test {
        class test_ray_t {
        private:
                const static test_ray_t test;
                test_ray_t () {
                        UNIT_TEST_REPORT_BEGIN("");
                        UNIT_TEST_WARN("no tests defined!");
                }
        };
} }

#endif // RAY_H_INCLUDED_20090223
