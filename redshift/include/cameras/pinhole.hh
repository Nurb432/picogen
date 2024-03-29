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

#ifndef PINHOLE_HH_INCLUDED_20090312
#define PINHOLE_HH_INCLUDED_20090312

namespace redshift { namespace camera {        
        
        DefineFinalizer(Pinhole);

        class Pinhole : public Camera, DoFinalize(Pinhole) {
        public:

                Pinhole (shared_ptr<RenderTarget> film);
                virtual ~Pinhole () ;
                
                inline tuple<float,RayDifferential>
                           generateRay(Sample const &) const;                           
                
                
        private:                
                shared_ptr<RenderTarget> film;
                real_t invFilmWidth;
                real_t invFilmHeight;
        };
} }

#endif // PINHOLE_HH_INCLUDED_20090312
