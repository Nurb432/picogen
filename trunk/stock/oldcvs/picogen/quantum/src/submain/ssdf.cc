/***************************************************************************
 *            ssdf.cc
 *
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

#include <iostream>
#include <limits>
#include <vector>
#include <stack>
#include <map>
#include <iostream>

#include <SDL/SDL.h>

#include <picogen/picogen.h>
#include <picogen/errors.h>
#include <picogen/misc/picossdf.h>
#include <picogen/graphics/objects/Instance.h>
#include <picogen/graphics/objects/templates/TriBIH.h>
#include <picogen/graphics/objects/QuadtreeHeightField.h>
#include <picogen/graphics/objects/CloudAdapter.h>




using ::picogen::real;
using ::picogen::geometrics::Vector3d;
using ::picogen::geometrics::Ray;
using ::picogen::graphics::color::Color;
using ::picogen::graphics::structs::intersection_t;

using ::picogen::geometrics::Transformation;

using ::picogen::graphics::objects::templates::TriBIH;

using ::picogen::graphics::material::abstract::IBRDF;
using ::picogen::graphics::material::abstract::IShader;
using ::picogen::graphics::objects::abstract::IScene;
using ::picogen::graphics::objects::Sphere;

using ::picogen::graphics::objects::Preetham;
using ::picogen::graphics::objects::CloudAdapter;
using ::picogen::misc::functional::Function_R2_R1;
using ::picogen::misc::functional::Function_R2_R1_Refcounted;


/*
struct t_triangle {
    private:
        Vector3d t[3];
    public:
        t_triangle (Vector3d a, Vector3d b, Vector3d c) {
            t[0] = a;
            t[1] = b;
            t[2] = c;
        }
        t_triangle() {
        }

        const Vector3d & operator [] ( int i ) const {
            return t[i];
        }
};
typedef TriBIH<t_triangle> genBIH;
static genBIH myGenBIH;
*/


namespace {
    const ::picogen::graphics::shaders::ConstantShader  red (picogen::graphics::color::Color (1.0, 0.3, 0.3));
    const ::picogen::graphics::shaders::ConstantShader  green (picogen::graphics::color::Color (0.3, 1.0, 0.3));
    const ::picogen::graphics::shaders::ConstantShader  blue (picogen::graphics::color::Color (0.3, 0.3, 1.0));
    const ::picogen::graphics::shaders::ConstantShader  white (picogen::graphics::color::Color (1.0, 1.0, 1.0));
}



template<class t_surface>
void
draw (
    SDL_Surface *p_target,
    const t_surface &surface,
    float scale,
    float exp_tone,
    float saturation,
    int aa_width
) {
    assert (0 != p_target);

    if (SDL_MUSTLOCK (p_target) && SDL_LockSurface (p_target) <0)
        return;

    int x,y;
    for (y=0; y<p_target->h; y++) {
        /// \todo FIX we are currently assuming a 32bit SDL buffer
        /// \todo get rid of pointer arithmetic
        Uint32 *bufp   = (Uint32*) p_target->pixels + y* (p_target->pitch>>2);
        for (x=0; x<p_target->w; x++) {
            /*float d = 1.0 / source->density;
            float r = source->color[0]*scale*d;
            float g = source->color[1]*scale*d;
            float b = source->color[2]*scale*d;*/
            real accu_r=real (0), accu_g=real (0), accu_b=real (0);
            for (register int u=0; u<aa_width; ++u) for (register int v=0; v<aa_width; ++v) {
                    real r,g,b;
                    ( (Color) surface (x*aa_width+u,y*aa_width+v)).to_rgb (r, g, b);
                    //((xrt::base_types::color)surface((unsigned)x,(unsigned)y)).to_rgb( r, g, b );
                    // tone map

                    /*
                    const float brightness = 0.299*r + 0.584*g + 0.144*b;
                    const float e = 1-exp(-brightness * exp_tone );
                    r = (r/brightness) * e;
                    g = (g/brightness) * e;
                    b = (b/brightness) * e;
                    //*/

                    // scale linearly
                    r *= scale;
                    g *= scale;
                    b *= scale;
                    // saturate color
                    const float sbrightness = 0.299*r + 0.584*g + 0.144*b;
                    r = saturation*r + (1-saturation) *sbrightness;
                    g = saturation*g + (1-saturation) *sbrightness;
                    b = saturation*b + (1-saturation) *sbrightness;
                    // final saturation
                    accu_r += r<0 ? 0 : r>1 ? 1 : r;
                    accu_g += g<0 ? 0 : g>1 ? 1 : g;
                    accu_b += b<0 ? 0 : b>1 ? 1 : b;
            }
            accu_r /= static_cast<real>(aa_width*aa_width);
            accu_g /= static_cast<real>(aa_width*aa_width);
            accu_b /= static_cast<real>(aa_width*aa_width);

            * (bufp++) =
                SDL_MapRGB (p_target->format,
                            (unsigned char) (255.0*accu_r),
                            (unsigned char) (255.0*accu_g),
                            (unsigned char) (255.0*accu_b)
                           );
        }
    }


    if (SDL_MUSTLOCK (p_target))
        SDL_UnlockSurface (p_target);
    SDL_Flip (p_target);
}




typedef enum surface_integrator_type_t {
    whitted_style,
    path_tracing
} surface_integrator_type;

class Scene {
    public:
        virtual ~Scene () {}
        virtual std::string getName() const = 0;
        virtual void initRenderer (int width, int height, int antiAliasingWidth, surface_integrator_type stype) = 0;
        virtual void flip (SDL_Surface *screen, float scale, float saturation) = 0;
        virtual void begin() = 0;
        virtual bool renderMore (int numPixels) = 0;  // i wanted to call it continue, but then continue is obviously a reserved word :P
        virtual void end() = 0;
};


class SSDFScene : public Scene, public SSDFBackend {
    protected:

        typedef ::picogen::common::LinearList LinearList;
        typedef ::picogen::common::TriBIH TriBIH;
        typedef ::picogen::graphics::objects::abstract::IScene IScene;
        typedef ::picogen::graphics::objects::abstract::IIntersectable IIntersectable;


        ::picogen::graphics::integrators::screen::XYIterator renderer;
        int width, height;


        LinearList list;
        ::picogen::graphics::objects::Preetham preetham;
        bool enablePreethamSky;
        real fogMaxDist, fogExp;

        ::picogen::graphics::objects::CloudAdapter cloudAdapter;
        bool enableCloudAdapter;
        //::picogen::common::AABox      box;

        ::picogen::generators::rng::MersenneTwister twister;


        enum SCENE_TYPE {
             LINEAR_LIST, TRI_BIH
        };

        struct Scene {
            union {
                LinearList *linearList;
                TriBIH *triBih;
            };
            SCENE_TYPE type;
        };
        typedef ::std::stack<Scene> SceneStack;
        typedef ::std::string string;

        Scene currentScene;
        SceneStack sceneStack;

        int antiAliasingWidth;

        ::picogen::graphics::objects::abstract::IAccelerationStructure *sceneRoot;
        IBRDF* currentBRDF;

        //::std::vector<IBRDF*> brdfStack;
        ::std::map<string,IBRDF*> brdfMap;

        PicoSSDF ssdf;

        ::std::string outputFilename;
        int writeOutputEveryNthLoop;
        int writeOutputEveryNthLoop_loopsLeft; // D'oh.

        // TODO: Still a hack, make Scene more OOPish
        void mergeStackTops () {
            IIntersectable *is = 0;
            switch (sceneStack.top().type) {
                case LINEAR_LIST: is = sceneStack.top().linearList; break;
                case TRI_BIH: is = sceneStack.top().triBih; break;
            };

            sceneStack.pop();
            if (0 < sceneStack.size()) {
                switch (sceneStack.top().type) {
                    case LINEAR_LIST: sceneStack.top().linearList->insert (is); break;
                    case TRI_BIH:
                        ::std::cerr << "The impossible becomes thruth: mergeStackTops() down to a tri-bih ("
                            << __FILE__ << "," << __LINE__ << ")";
                        exit (666);
                        break;
                };
            }
        }



    public:


        // --- SSDFBackend implementation -------------------------------

        // Init:
        virtual int initialize () {
            preetham.setTurbidity (1.9);
            preetham.setSunSolidAngleFactor (0.125);
            const real L = 0.25;
            preetham.setColorFilter (Color (1.0,1.0,1.0) *1.0*L);
            preetham.setSunColor (Color (1.0,0.9,0.8) *1600.0*L);
            preetham.setSunDirection (Vector3d (-1.7,2.5,3.3).normal());
            fogExp = 0.0;
            fogMaxDist = 100000000.0;
            preetham.enableFogHack (false, fogExp, fogMaxDist);
            preetham.enableSunFalloffHack (false);
            enablePreethamSky = true;
#ifndef PICOGEN_RELEASE
            //std::cout << "initialising cloudAdapter..." << std::endl;
            boost::intrusive_ptr <Function_R2_R1_Refcounted> f (
                //new Function_R2_R1_Refcounted ("(+ ([2 LayeredNoise frequency(0.00085) layercount(20) filter(cosine) persistence(0.4) seed(123)] xy) 0.3)"));
                new Function_R2_R1_Refcounted (
                    "(* 3 (+ "
                      " 0 "//*/"(*2([2 LayeredNoise filter(cosine) frequency(0.0003)]xy))"
                      "(- 1 (^ 0.3 (max 0 (+ 1.0 ([2 LayeredNoise filter(cosine) frequency(0.0013) persistence(0.5) layercount(18)]xy)))))))"
                )
            );
            cloudAdapter.setCloudFunction (f);
#endif // PICOGEN_RELEASE
            //std::cout << "cloudAdapter initialized" << std::endl;
            enableCloudAdapter = false;
            return 0;
        }

        virtual int finish () {
            return 0;
        }

        // Global:
        virtual int beginGlobalBlock () {
            return 0;
        }
        virtual int endGlobalBlock () {
            return 0;
        }


        // List:
        virtual int beginListBlock () {
            currentScene.type = LINEAR_LIST;
            currentScene.linearList = new LinearList;
            if (0 == sceneRoot) {
                sceneRoot = currentScene.linearList;
            }
            sceneStack.push (currentScene);
            return 0;
        }
        virtual int endListBlock () {
            currentScene.linearList->invalidate();

            mergeStackTops();

            if (0 < sceneStack.size()) {
                currentScene =  sceneStack.top();
            } else {
                // End of scene hierarchy.
            }

            return 0;
        }

        // TriBIH:
        virtual int beginTriBIHBlock () {
            currentScene.type = TRI_BIH;
            currentScene.triBih = new TriBIH;
            if (0 == sceneRoot) {
                sceneRoot = currentScene.triBih;
            }
            sceneStack.push (currentScene);
            return 0;
        }
        virtual int endTriBIHBlock () {
            currentScene.triBih->invalidate();
            //sceneStack.pop_back();
            mergeStackTops();

            if (0 < sceneStack.size()) {
                currentScene =  sceneStack.top();
            } else {
                // End of scene hierarchy.
            }

            return 0;
        }


        virtual int setBRDFToLambertian (::picogen::real reflectance) {
            using namespace std;
            string brdfId;
            stringstream ss;
            ss.precision (numeric_limits<real>::digits10);
            ss << string ("lambertian:") << reflectance;
            ss << reflectance;
            ss >> brdfId;

            // Effectively like gcc's -fmerge-all-constants.
            if (brdfMap.end() == brdfMap.find (brdfId)) {
                currentBRDF = new ::picogen::graphics::material::brdf::Lambertian(reflectance);
                currentBRDF->setRNG (&twister);
                brdfMap [brdfId] = currentBRDF;
            } else {
                currentBRDF = brdfMap [brdfId];
            }
            return 0;
        }
        virtual int setBRDFToSpecular (::picogen::real reflectance) {
            using namespace std;
            string brdfId;
            stringstream ss;
            ss.precision (numeric_limits<real>::digits10);
            ss << string ("specular:") << reflectance;
            ss >> brdfId;

            // Effectively like gcc's -fmerge-all-constants.
            if (brdfMap.end() == brdfMap.find (brdfId)) {
                currentBRDF = new ::picogen::graphics::material::brdf::Specular(reflectance);
                currentBRDF->setRNG (&twister);
                brdfMap [brdfId] = currentBRDF;
            } else {
                currentBRDF = brdfMap [brdfId];
            }
            return 0;
        }
        virtual int setBRDFToSpecular_DistortedHeight (::picogen::real reflectance, const std::string &code) {
            using namespace std;
            string brdfId;
            stringstream ss;
            ss.precision (numeric_limits<real>::digits10);
            ss << string ("specular_distorted-height:") << reflectance;
            ss << string (":") << code; // TODO: normalise code before using it as a key-part
            ss >> brdfId;

            // Effectively like gcc's -fmerge-all-constants.
            if (brdfMap.end() == brdfMap.find (brdfId)) {
                ::boost::intrusive_ptr< ::picogen::misc::functional::Function_R2_R1_Refcounted> fun =
                    new ::picogen::misc::functional::Function_R2_R1_Refcounted (code);

                currentBRDF = new ::picogen::graphics::material::brdf::Specular_DistortedHeight(reflectance, fun);
                currentBRDF->setRNG (&twister);
                brdfMap [brdfId] = currentBRDF;
            } else {
                currentBRDF = brdfMap [brdfId];
            }
            return 0;
        }


        // Preetham:
        virtual int preethamEnable (bool enable) {
            enablePreethamSky = enable;
            return 0;
        }

        virtual int preethamSetTurbidity (::picogen::real T)  {
            preetham.setTurbidity (T);
            return 0;
        }

        virtual int preethamSetSunSolidAngleFactor (::picogen::real f) {
            preetham.setSunSolidAngleFactor (f);
            return 0;
        }

        virtual int preethamSetColorFilter (const ::picogen::graphics::color::Color &color) {
            preetham.setColorFilter (color);
            return 0;
        }

        virtual int preethamSetSunColor (const ::picogen::graphics::color::Color &color) {
            preetham.setSunColor (color);
            return 0;
        }

        virtual int preethamSetSunDirection (const ::picogen::geometrics::Vector3d &direction) {
            preetham.setSunDirection (direction.computeNormal());
            return 0;
        }

        virtual int preethamSetFogExp (real exp) {
            fogExp = exp;
            return 0;
        }

        virtual int preethamSetFogMaxDist (real dist) {
            fogMaxDist = dist;
            return 0;
        }
        
        virtual int preethamSetSunFalloffParameters (bool enable, ::picogen::real a, ::picogen::real b, ::picogen::real c) {
            preetham.enableSunFalloffHack (enable);
            if (enable) {
                preetham.setSunFalloffHackParameters (a, b, c);
            }
            return 0;
        }

        /*virtual int preethamEnableFogHack (real exponent, real maxDist) {
            preetham.enableFogHack (maxDist>0.0, exponent, maxDist);
            std::cout << "preetham: {" << exponent << ". " << maxDist << "}" << std::endl;
            return 0;
        }*/


        // Camera:
        virtual int cameraSetPositionYawPitchRoll (
            const ::picogen::geometrics::Vector3d &position,
            ::picogen::real yaw,
            ::picogen::real pitch,
            ::picogen::real roll
        ) {
            renderer.transformation() = //Transformation().setToTranslation (position);
                Transformation().setToRotationZ (roll) *
                Transformation().setToRotationX (pitch) *
                Transformation().setToRotationY (yaw) *
                Transformation().setToTranslation (position);
            //std::cout << "camera position={" << position[0] << ", " << position[1] << ", " << position[2] << "} ypr{" << yaw << ", " << pitch << ", " << roll << "}" << std::endl;
            return 0;
        }



        // Heightmap:
        virtual int addHeightfield (
            const ::boost::intrusive_ptr<picogen::misc::functional::Function_R2_R1_Refcounted> &fun,
            const IShader *shader, //::boost::intrusive_ptr<picogen::misc::functional::Function_R6_R1_Refcounted> &mat_fun,
            //const ::std::vector< ::picogen::graphics::material::abstract::IBRDF*> &brdfs,
            //const ::std::vector< ::picogen::graphics::material::abstract::IShader*> &shaders,
            unsigned int resolution,
            const ::picogen::geometrics::Vector3d &center,
            const ::picogen::geometrics::Vector3d &size
        ) {
            using namespace ::picogen::graphics::objects;
            using namespace ::picogen::misc::functional;
            using namespace ::picogen::geometrics;

            switch (currentScene.type) {
                case LINEAR_LIST:{
                    if (false) {//(0 == fork()) {
                        //currentScene.linearList->insert (heightField);
                    } else {
                        real accuracy = 10000.0 / static_cast<real>(resolution*resolution);
                        QuadtreeHeightField *heightField = QuadtreeHeightField::create(
                            resolution, fun, BoundingBox (center-size*0.5, (center+size*0.5)), accuracy, false
                        );

                        heightField->setBRDF (currentBRDF);

                        //::boost::intrusive_ptr<Function_R6_R1_Refcounted> fun (new Function_R6_R1_Refcounted ("(^ ([2 LayeredNoise frequency(2) layercount(15) persistence((+0.5(*0.2x))) filter(cosine) levelEvaluationFunction((- 0.5 (abs x))) seed(11)] x y) 4)"));
                        //HeightSlangShader *hs = new HeightSlangShader (mat_fun, shaders);
                        heightField->setShader (shader);
                        //heightField->setBox (center-size*0.5, center+size*0.5);
                        currentScene.linearList->insert (heightField);
                    }
                    // TODO URGENT !!!!one1 --> see LinearList.cc
                } break;

                case TRI_BIH:{
                    SimpleHeightField *heightField = new SimpleHeightField ();

                    heightField->setBRDF (currentBRDF);
                    heightField->setShader (shader);//&white);
                    heightField->setBox (center-size*0.5, center+size*0.5);

                    heightField->init (resolution, &*fun, 0.1, false);

                    currentScene.triBih->setBRDF (currentBRDF);
                    currentScene.triBih->setShader (&white);
                    heightField->feedTriScene (currentScene.triBih);
                    delete heightField;
                } break;
            };
            return 0;
        }



        // Heightmap:
        virtual int addImplicitHeightfield (
            ::picogen::misc::functional::Function_R2_R1 **fun,
            const ::picogen::geometrics::Vector3d &center,
            const ::picogen::geometrics::Vector3d &size
        ) {
            ::picogen::graphics::objects::ImplicitHeightField *heightField =
                new ::picogen::graphics::objects::ImplicitHeightField ();

            using namespace ::picogen::misc::functional;

            heightField->setBRDF (currentBRDF);
            heightField->setShader (&white);
            heightField->setBox (center-size*0.5, center+size*0.5);
            heightField->setFunction (*fun);
            //heightField->init (resolution, &fun, 0.1, false);
            switch (currentScene.type) {
                case LINEAR_LIST:{
                    currentScene.linearList->insert (heightField);
                    // TODO URGENT !!!!one1 --> see LinearList.cc
                } break;

                case TRI_BIH:{
                    ::std::cerr << "The impossible becomes thruth: addImplicitHeightfield() within TRI_BIH-Block ("
                        << __FILE__ << "," << __LINE__ << ")";
                    exit (666);
                } break;
            };
            return 0;
        }



        //
        virtual int addSphereTerminal (
            ::picogen::real radius,
            const ::picogen::geometrics::Vector3d &center,
            const ::picogen::graphics::color::Color &color
        ) {
            using namespace ::std;
            using namespace ::picogen::graphics::material::brdf;



            switch (currentScene.type) {
                case LINEAR_LIST:{
                    Sphere *sphere = new Sphere;
                    sphere->setColor (color);
                    sphere->setRadius (radius);
                    sphere->setPosition (center);
                    sphere->setBRDF (currentBRDF);
                    currentScene.linearList->insert (sphere);
                    // TODO URGENT !!!!one1 --> see LinearList.cc
                } break;
                case TRI_BIH:{
                    ::std::cerr << "The impossible becomes thruth: addSphereTerminal() within TRI_BIH-Block ("
                        << __FILE__ << "," << __LINE__ << ")";
                    exit (666);
                } break;
            };
            return 0;
        }
        // --------------------------------------------------------------


        explicit SSDFScene(const std::string &filename, const std::string &outputFilename, int outputEveryNthLoop)
        : preetham ()
        , cloudAdapter (
                preetham
                //, boost::intrusive_ptr <Function_R2_R1_Refcounted> (new Function_R2_R1_Refcounted ("([2 LayeredNoise frequency(11.0) layercount(12) filter(cosine) persistence(0.5) seed(104)] (+x0.3)y)"))
            )
        , sceneRoot(0)
        , currentBRDF(0)
        , ssdf(filename, this)
        , outputFilename (outputFilename)
        , writeOutputEveryNthLoop (outputEveryNthLoop)
        , writeOutputEveryNthLoop_loopsLeft (outputEveryNthLoop)
        {
            // Setup default camera transform.
            //renderer.transformation() = Transformation().setToTranslation (Vector3d (0.5,2.0,-5.0));
                /*Transformation().setToRotationX (3.14159*0.0) *
                Transformation().setToRotationY (3.14159*0.0) *
                Transformation().setToTranslation (Vector3d (0.0,0.0,-1.5) *1.0)
                ;*/
        }



        virtual ~SSDFScene() {
            using namespace std;

            if (0 != sceneRoot)
                delete sceneRoot;

            ::std::map<string,IBRDF*>::iterator it;
            for (it=brdfMap.begin(); it!=brdfMap.end(); ++it) {
                delete it->second;
            }
            brdfMap.clear();

            if (0 != renderer.getCamera())
                delete renderer.getCamera();
            renderer.setCamera (0);

            if (0 != renderer.getFilm())
                delete renderer.getFilm();
            renderer.setFilm (0);

            if (0 != renderer.getSky())
                delete renderer.getSky();
            renderer.setSky (0);

            if (0 != renderer.getIntersectable())
                delete renderer.getIntersectable();
            renderer.setIntersectable (0);

            if (0 != renderer.getSurfaceIntegrator())
                delete renderer.getSurfaceIntegrator();
            renderer.setSurfaceIntegrator (0);
        }



        virtual std::string getName() const {
            return std::string ("ssdf-parser+renderer");
        }



        virtual void initRenderer (int width, int height, int antiAliasingWidth, surface_integrator_type stype) {
            using namespace picogen;

            this->antiAliasingWidth = antiAliasingWidth;
            this->width  = width;
            this->height = height;
            // setup screen and camera
            //renderer.surface().reset (width*antiAliasingWidth, height*antiAliasingWidth);


            // setup and recognize sky
            preetham.enableFogHack (true, fogExp, fogMaxDist);
            preetham.invalidate();
            if (enableCloudAdapter) {
                renderer.setSky (&cloudAdapter);
            } else if (enablePreethamSky) {
                renderer.setSky (&preetham);
            } else {
                renderer.setSky (0);
            }

            ::picogen::graphics::cameras::FromPointToRect *fptr = new ::picogen::graphics::cameras::FromPointToRect;
            fptr->defineCamera ( (real) width/ (real) height, 1.0, 0.85);
            renderer.setCamera (fptr);

            ::picogen::graphics::film::abstract::IFilm *film = new ::picogen::graphics::film::SimpleColorFilm (width*antiAliasingWidth, height*antiAliasingWidth);
            renderer.setFilm (film);

            // setup boxen
            //box.enableFace (box.y_positive, false);

            // recognize scene
            /*list.insert (&box);
            list.insert (sceneRoot);
            // invalidate and recognize object-list
            list.invalidate();*/
            if (0 == sceneRoot) { // Well, we allow to just render atmosphere. So we put in a dummy list.
                sceneRoot = new LinearList;
                sceneRoot->invalidate ();
            }
            //renderer.path_integrator().setIntersectable (sceneRoot);

            renderer.setIntersectable (sceneRoot);

            ::picogen::graphics::integrators::surface::ISurfaceIntegrator *surfaceIntegrator = 0;
            switch (stype) {
                case whitted_style:
                    surfaceIntegrator = new ::picogen::graphics::integrators::surface::Whitted();
                    break;
                case path_tracing:
                    surfaceIntegrator = new ::picogen::graphics::integrators::surface::Path();
                    break;
            };
            assert (0 != surfaceIntegrator);
            renderer.setSurfaceIntegrator (surfaceIntegrator);
        }

        virtual void flip (SDL_Surface *screen, float scale, float saturation) {            
            ::draw (screen,*renderer.getFilm(), scale, 1.0, saturation, antiAliasingWidth);//scale, exp_tone, saturation);
            ::SDL_Flip (screen);
        }

        virtual void begin() {
            renderer.beginRender();
        }

        virtual bool renderMore (int numPixels) {
            renderer.setNumPixelsPerContinue (numPixels);
            bool c = renderer.renderMore();
            if (c) {
                if (--writeOutputEveryNthLoop_loopsLeft <= 0) {
                    writeOutputEveryNthLoop_loopsLeft = writeOutputEveryNthLoop;
                    saveOutput();
                }
                renderer.oneMoreRun();
            }
            return c;
        }

        virtual void end() {
        }
        
        void saveOutput() {
            SDL_Surface *surface = SDL_CreateRGBSurface (SDL_SWSURFACE, width, height, 32, 0,0,0,0);
            if (0 != surface) {
                ::draw (surface, *renderer.getFilm(), 1.0, 1.0, 1.0, antiAliasingWidth);
                SDL_SaveBMP (surface, outputFilename.c_str());
                SDL_FreeSurface (surface);
            }
        }
};



static ::picogen::error_codes::code_t loop (SDL_Surface *screen, SSDFScene *scene, const int width, const int height, const int aa, const int loopCount) {
    //scene->flip( screen );
    // + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
    // prepare and run loop
    // + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
    unsigned int pixelsPerContinue = (width*height) / 32;
    if (pixelsPerContinue <= 0)
        pixelsPerContinue = width;

    int runCount = 0;
    //clock_t startTime = clock();
    bool done = false;
    bool renderFinished = false;
    bool queryingForReallyQuit = false;
    unsigned int numManDumbs = 0;
    float scale = 1;
    float exp_tone = 1.0;
    float saturation = 1.0;

    srand (time (0));


    //renderer.SetNumPixelsPerContinue( pixelsPerContinue );//*(HEIGHT/1) );
    //renderer.BeginRender();
    scene->begin();
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent (&event)) {
            if (event.type == SDL_QUIT) {
                done = true;
            }
            if (event.type == SDL_KEYDOWN) {
                if (queryingForReallyQuit) {
                    if (event.key.keysym.sym == SDLK_y) {
                        done = true;
                        queryingForReallyQuit = false;
                    } else if (event.key.keysym.sym == SDLK_n) {
                        done = false;
                        queryingForReallyQuit = false;
                    }
                } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    if (renderFinished) {
                        done = true;
                    } else {
                        queryingForReallyQuit = 1;
                    }
                }
                if (event.key.keysym.sym == SDLK_d) {
                    char s[256];
                    sprintf (s, "screendump.%.4d.bmp", numManDumbs++);
                    SDL_SaveBMP (screen, s);
                }
                if (event.key.keysym.sym == SDLK_y) {
                    pixelsPerContinue-=100*width; /*renderer.SetNumPixelsPerContinue( pixelsPerContinue );*/
                }
                if (event.key.keysym.sym == SDLK_x) {
                    pixelsPerContinue-=10*width; /*renderer.SetNumPixelsPerContinue( pixelsPerContinue );*/
                }
                if (event.key.keysym.sym == SDLK_c) {
                    pixelsPerContinue-=width; /*renderer.SetNumPixelsPerContinue( pixelsPerContinue );*/
                }
                if (event.key.keysym.sym == SDLK_v) {
                    pixelsPerContinue+=width; /*renderer.SetNumPixelsPerContinue( pixelsPerContinue );*/
                }
                if (event.key.keysym.sym == SDLK_b) {
                    pixelsPerContinue+=10*width; /*renderer.SetNumPixelsPerContinue( pixelsPerContinue );*/
                }
                if (event.key.keysym.sym == SDLK_n) {
                    pixelsPerContinue+=100*width; /*renderer.SetNumPixelsPerContinue( pixelsPerContinue );*/
                }

                if (event.key.keysym.sym == SDLK_5) {
                    scale = 1.0;
                }
                if (event.key.keysym.sym == SDLK_6) {
                    scale += 0.01;
                }
                if (event.key.keysym.sym == SDLK_7) {
                    scale += 0.1;
                }
                if (event.key.keysym.sym == SDLK_8) {
                    scale += 1.0;
                }
                if (event.key.keysym.sym == SDLK_9) {
                    scale += 10.0;
                }
                if (event.key.keysym.sym == SDLK_4) {
                    scale -= 0.01;
                }
                if (event.key.keysym.sym == SDLK_3) {
                    scale -= 0.1;
                }
                if (event.key.keysym.sym == SDLK_2) {
                    scale -= 1.0;
                }
                if (event.key.keysym.sym == SDLK_1) {
                    scale -= 10.0;
                }

                if (event.key.keysym.sym == SDLK_t) {
                    exp_tone = 1.0;
                }
                if (event.key.keysym.sym == SDLK_z) {
                    exp_tone += 0.01;
                }
                if (event.key.keysym.sym == SDLK_u) {
                    exp_tone += 0.1;
                }
                if (event.key.keysym.sym == SDLK_i) {
                    exp_tone += 1.0;
                }
                if (event.key.keysym.sym == SDLK_o) {
                    exp_tone += 10.0;
                }
                if (event.key.keysym.sym == SDLK_r) {
                    exp_tone -= 0.01;
                }
                if (event.key.keysym.sym == SDLK_e) {
                    exp_tone -= 0.1;
                }
                if (event.key.keysym.sym == SDLK_w) {
                    exp_tone -= 1.0;
                }
                if (event.key.keysym.sym == SDLK_q) {
                    exp_tone -= 10.0;
                }

                if (event.key.keysym.sym == SDLK_g) {
                    saturation -= 0.1;
                }
                if (event.key.keysym.sym == SDLK_h) {
                    saturation -= 0.01;
                }
                if (event.key.keysym.sym == SDLK_j) {
                    saturation = 1.0;
                }
                if (event.key.keysym.sym == SDLK_k) {
                    saturation += 0.01;
                }
                if (event.key.keysym.sym == SDLK_l) {
                    saturation += 0.1;
                }

            }
        }

        if (!renderFinished) {
            // Argh, clean this up!
            bool cont = (loopCount==-1 || runCount<loopCount) && scene->renderMore (pixelsPerContinue);
            if (loopCount!=-1 && runCount == loopCount) {
                std::stringstream s;
                s << "Finished (press Escape)! Samples per pixel: " << (runCount*aa*aa) << " (including antialiasing)" << std::flush;
                SDL_WM_SetCaption (s.str().c_str(), "rendered!");
                renderFinished = true;
            } else if (cont) {
                runCount++;
                //if( 0 == (runCount%3) )
                //    renderer.surface().saveBinaryToFile( checkpointFile );
            } else if (queryingForReallyQuit) {
                SDL_WM_SetCaption ("Really Quit? (Y/N) ", "Really Quit? (Y/N) ");
            } else {
                char s[256];
                sprintf (s, "%uspp;color.{*%.2f,t%.2f,s%.2f};refresh{%.2fl}", runCount, scale, exp_tone, saturation, (float) pixelsPerContinue/ (float) width);
                fprintf (stderr, "\rrendering pass %u (%s)", 1+runCount, s);
                SDL_WM_SetCaption (s, "rendering ...");
            }
        }
        
        scene->flip (screen, scale, saturation);        
    }
    scene->saveOutput();
    scene->end();
    return picogen::error_codes::ssdf_okay;
}



static ::picogen::error_codes::code_t grind (int width, int height, int antiAliasingWidth, int loopCount, SSDFScene *scene, surface_integrator_type stype) {
    using namespace std;

    switch (stype) {
        case whitted_style:
            loopCount = 1;
            break;
        case path_tracing:
            break;
    };

    scene->initRenderer (width, height, antiAliasingWidth, stype);

    if (SDL_Init (SDL_INIT_VIDEO) < 0) {
        cerr << "Unable to init SDL: " << SDL_GetError() << endl;
        return picogen::error_codes::ssdf_sdl_not_initialised;
    }
    atexit (SDL_Quit);
    SDL_Surface *screen = SDL_SetVideoMode (width,height,32,SDL_HWSURFACE|SDL_DOUBLEBUF);
    if (0 == screen) {
        cerr << "Unable to set video-mode: " << SDL_GetError() << endl;
        return picogen::error_codes::ssdf_set_video_mode_failed;
    }

    //scene->flip();
    return loop (screen, scene, width, height, antiAliasingWidth, loopCount);  /// \todo check if SDL cleans up surface resource
}



static void printUsage() {
    std::cout
        << "Invocation: picogen ssdf <options>\n"
        << "In the version of picogen you have installed, the following options are possible:\n"
        << "\n"
        << "-f | --filename <filename> : name of the ssdf file to render\n"
        << "-o | --output <filename> : name of the bmp file to render to\n"
        << "-w | --width <number> : width of the output in pixels, default 320\n"
        << "-h | --height <number> : height of the output in pixels, default 320\n"
        << "-a | --AA-width <n> : set anti-aliasing-size to n*n pixels, default 1\n"
        << "\n"
        << "-s | --surface-integrator <si> : set the surface integrator, either \n"
        << "                                 'whitted-style' or 'ws', or\n"
        << "                                 'path-tracing' or 'pt',\n"
        << "                                 default is 'pt'\n"
    << std::endl;
}



picogen::error_codes::code_t main_ssdf (int argc, char *argv[]) {
    using namespace std;

    if (argc == 0) {
        printUsage();
        return picogen::error_codes::ssdf_okay;
    }


    //++ Options.
    string filename ("");
    int width = 320;
    int height = 320;
    int aaWidth = 1;
    int writeOutputEveryNthLoop = 1;
    int totalLoops = 0;
    surface_integrator_type stype = path_tracing;
    string outputFilename = "";
    //-- Options.

    while (0<argc) {
        const string primary = argv[0];
        --argc;
        ++argv;

        if (primary == string ("-f") || primary == string ("--filename")) {
            if (argc == 0) {   // check if any argument is given, we need at least one remaining
                printUsage();
                return picogen::error_codes::ssdf_missing_source_file_name;
            }
            const std::string secondary = argv [0];
            filename = argv [0];
            --argc;
            ++argv;
        } else if (primary == string ("-w") || primary == string ("--width")) {
            if (argc == 0) {   // check if any argument is given, we need at least one remaining
                printUsage();
                return picogen::error_codes::ssdf_missing_argument_to_option_width;
            }
            stringstream ss;
            ss << argv [0];
            ss >> width;

            --argc;
            ++argv;
        } else if (primary == string ("-o") || primary == string ("--output")) {
            if (argc == 0) {   // check if any argument is given, we need at least one remaining
                printUsage();
                return picogen::error_codes::ssdf_missing_argument_to_option_output;
            }
            stringstream ss;
            ss << argv [0];
            outputFilename = ss.str();
            --argc;
            ++argv;
        } else if (primary == string ("-h") || primary == string ("--height")) {
            if (argc == 0) {   // check if any argument is given, we need at least one remaining
                printUsage();
                return picogen::error_codes::ssdf_missing_argument_to_option_height;
            }
            stringstream ss;
            ss << argv [0];
            ss >> height;

            --argc;
            ++argv;
        } else if (primary == string ("-a") || primary == string ("--AA-width")) {
            if (argc == 0) {   // check if any argument is given, we need at least one remaining
                printUsage();
                return picogen::error_codes::ssdf_missing_argument_to_option_aa;
            }
            stringstream ss;
            ss << argv [0];
            ss >> aaWidth;
            if (0 >= aaWidth) {
                aaWidth = 1;
            }

            --argc;
            ++argv;
        } else if (primary == string ("-n") || primary == string ("--write-output-each-n-loops")) {
            if (argc == 0) {   // check if any argument is given, we need at least one remaining
                printUsage();
                return picogen::error_codes::ssdf_missing_argument_to_option_n;
            }
            stringstream ss;
            ss << argv [0];
            ss >> writeOutputEveryNthLoop;
            if (0 >= writeOutputEveryNthLoop) {
                writeOutputEveryNthLoop = 1;
            }

            --argc;
            ++argv;
        } else if (primary == string ("-l") || primary == string ("--total-loops")) {
            if (argc == 0) {   // check if any argument is given, we need at least one remaining
                printUsage();
                return picogen::error_codes::ssdf_missing_argument_to_option_l;
            }
            stringstream ss;
            ss << argv [0];
            ss >> totalLoops;
            if (0 >= totalLoops) {
                totalLoops = -1;
            }

            --argc;
            ++argv;
        } else if (primary == string ("-s") || primary == string ("--surface-integrator")) {
            if (argc == 0) {   // check if any argument is given, we need at least one remaining
                printUsage();
                return picogen::error_codes::ssdf_missing_argument_to_option_surface_integrator;
            }
            stringstream ss;
            ss << argv [0];
            string tmp;
            ss >> tmp;

            if ("whitted-style" == tmp || "ws" == tmp) {
                stype = whitted_style;
            } else if ("path-tracing" == tmp || "pt" == tmp) {
                stype = path_tracing;
            } else {
                cerr << "Unknown surface integrator: '" << tmp << "'" << endl;
            }

            --argc;
            ++argv;
        } else if (primary == string ("-?") || primary == string ("?") || primary == string ("--help")) {
            printUsage ();
            return picogen::error_codes::ssdf_okay;
        } else {
            cerr << "Parameter '" << primary << "' unknown." << endl;
            return picogen::error_codes::ssdf_unknown_option;
        }
    }

    SSDFScene *grindScene;
    try {
        grindScene = new SSDFScene (filename, outputFilename, writeOutputEveryNthLoop);
        return grind (width, height, aaWidth, totalLoops, grindScene, stype);
    } catch (const PicoSSDF::exception_file_not_found &e) {
        cerr << "doh, exception_file_not_found." << endl;
        return picogen::error_codes::ssdf_file_not_found;
    } catch (const PicoSSDF::exception_unknown &e) {
        cerr << "doh, exception_unknown." << endl;
        return picogen::error_codes::ssdf_unknown_error;
    } catch (const PicoSSDF::exception_syntax_error &e) {
        cerr << "" << e.file << ""
             << ":" << e.line
             << ":error:" << e.reason << ""
             //<< " (near \"" << e.curr << "\")"
             << endl;
        return picogen::error_codes::ssdf_syntax_error;
    } catch (const picogen::misc::functional::functional_general_exeption &e) {
        cerr << "error:" << e.getMessage() << endl;
        return picogen::error_codes::ssdf_syntax_error;
    } catch (...) {
        cerr << "doh, ..." << endl;
        return picogen::error_codes::ssdf_unknown_error;
    }
    return picogen::error_codes::ssdf_okay;
}
