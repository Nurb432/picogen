/***************************************************************************
 *            Snow.h
 *
 *  Tue Apr 18 18:40:00 2008
 *  Copyright  2008  Sebastian Mach
 *  seb@greenhybrid.net
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 3 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


#include <picogen/picogen.h>
#include <picogen/experimental/Snow.h>

static ::picogen::graphics::material::brdf::Lambertian snowBRDF;



using picogen::real;
using picogen::geometrics::Vector3d;
using picogen::geometrics::Ray;
using picogen::geometrics::BoundingBox;
using picogen::graphics::material::abstract::IBRDF;
using picogen::graphics::structs::intersection_t;
using picogen::graphics::color::Color;


namespace picogen {
    namespace graphics {
        namespace objects {

            Snow::Snow() {
                fprintf (stderr, "creating Snow-object{\n");
                real raise = 1.5;
                real range = 10.0, minDist = 0.0, maxDist = range;
                real freq = 10.0;
                numSnowSpheres = 12;
                snowSpheres = new SnowSphere[numSnowSpheres];
                for (unsigned int u=0; u<numSnowSpheres; ++u) {
                    snowSpheres[u].snowMap.setPersistence (0.96);
                    snowSpheres[u].snowMap.setBaseFrequency (1.0);
                    snowSpheres[u].snowMap.setOctaves (3);
                    snowSpheres[u].snowMap.setDomainScale (freq*1.0);
                    snowSpheres[u].snowMap.setRangeScale (1.0);
                    snowSpheres[u].snowMap.setMode (snowSpheres[u].snowMap.normal);
                    snowSpheres[u].snowMap.setMod (snowSpheres[u].snowMap.mod_normal);
                    snowSpheres[u].snowMap.setEvalScale (1.9);
                    snowSpheres[u].snowMap.seed (11142);
                    snowSpheres[u].minDistance = minDist;
                    snowSpheres[u].maxDistance = maxDist;

                    fprintf (stderr, "  snow-sphere nb. %u) min/max/freq: %.2f/%.2f/%.2f\n", u, (float) minDist, (float) maxDist, (float) freq);

                    freq  *= raise;
                    range *= raise;
                    minDist  = maxDist;
                    maxDist += range;
                }

                /*distanceMap.setPersistence(0.96);
                distanceMap.setBaseFrequency( 128.0 );
                distanceMap.setOctaves(3);
                distanceMap.setDomainScale(1.0);
                distanceMap.setRangeScale(1.0);
                distanceMap.setMode( distanceMap.normal );
                distanceMap.setMod( distanceMap.mod_normal );
                distanceMap.setEvalScale( 1.9 );
                distanceMap.seed( 42 );*/
                fprintf (stderr, "  done.\n}\n");
            }

            Snow::~Snow() {
                delete [] snowSpheres;
                numSnowSpheres = 0;
            }

            bool Snow::enableRussianRoulette (bool) const {
                return true;
            }

            bool Snow::intersect (param_out (intersection_t,intersection), param_in (Ray,ray)) const {

                //Vector3d v = velocity * ( powf( real(rand())/real(RAND_MAX), timeexp ) );
                for (unsigned int u=0; u<numSnowSpheres; ++u) {
                    const real snow = fabs (
                                          snowSpheres[u].snowMap.f (
                                              ray.w() [0],
                                              ray.w() [1],
                                              ray.w() [2]
                                          )
                                      );
                    if (snow < 0.57)
                        continue;

                    const real t = snow*snowSpheres[u].minDistance + (1.0-snow) *snowSpheres[u].maxDistance;
                    intersection.color = Color (2.5,2.5,2.5);// * (0.5*(float)u/(float)(numSnowSpheres-1));
                    intersection.t     = t;
                    intersection.side  = constants::outside;
                    intersection.normal = Vector3d (0.0,1.0,0.0);
                    intersection.brdf  = &snowBRDF;
                    intersection.L_e   = 0.0;
                    return true;
                }
                return false;
            }

        };
    };
};