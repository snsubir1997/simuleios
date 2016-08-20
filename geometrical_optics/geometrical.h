/*------------geometrical.h---------------------------------------------------//
*
* Purpose: Hold functions from geometrical.cpp
*
*-----------------------------------------------------------------------------*/

#ifndef GEOMETRICAL_H
#define GEOMETRICAL_H

#include<atomic>

/*----------------------------------------------------------------------------//
* STRUCTS / FUNCTIONS
*-----------------------------------------------------------------------------*/

// Spinlock class for parallelization
class spinlock {
    std::atomic_flag locked = ATOMIC_FLAG_INIT;
public:
    void lock() {
        while (locked.test_and_set(std::memory_order_acquire)) { }
    }
    void unlock() {
        locked.clear(std::memory_order_release);
    }
};

// Constants
const int NUM_LIGHTS = 20;
const int TIME_RES = 500000;

// The static inlines are our best bet to force the inlining of the functions
// without using platform-specific extensions
static inline vec& operator+=(vec& a, vec b) {
    a.x += b.x;
    a.y += b.y;
    return a;
}

static inline vec operator-(vec a) { return vec(-a.x, -a.y); }
static inline vec operator+(vec a, vec b) { return vec(a.x + b.x, a.y + b.y); }
static inline vec operator-(vec a, vec b) { return vec(a.x - b.x, a.y - b.y); }
static inline vec operator*(vec a, double b) { return vec(a.x * b, a.y * b); }
static inline vec operator*(double a, vec b) { return b * a; }

static inline vec operator/(vec a, double b) {
    double inv = 1.0 / b;
    return a * inv;
}

static inline double dot(vec a, vec b) { return a.x * b.x + a.y * b.y; }
static inline double length(vec a) { return sqrt(dot(a, a)); }
static inline vec normalize(vec a) { return a / length(a); }
static inline double distance(vec a, vec b) { return length(a - b); }
static inline bool is_null(vec a) { return a.x == 0.0 && a.y == 0.0; }

// This normally wouldn't store the previous index, but there's no
// ray-shape intersection code yet.
struct ray {
    vec p, v;
    double previous_index;
};

// A convenience shorthand so we don't have to write the full type everywhere
using ray_array = std::array<ray, NUM_LIGHTS>;

// A struct describing a simple lens. Add additional lenses by adding
// a new struct and overloading the corresponding functions (see below)
struct simple {
    double left, right;
    simple(double l, double r) : left(l), right(r) {}
};

// A simple struct for circular / spherical lens
struct sphere{
    double radius;
    vec origin;
    double index_param = 1.0;
    sphere(double rad, double x, double y) : radius(rad), origin(x, y) {}
};

// Struct for spherical lens with varying refractive index
struct funky_sphere {
    double radius, index_param;
    vec origin;
    funky_sphere(double rad, double x, double y, double index) :
                 radius(rad), origin(x, y), index_param(index) {}
};

// Add overloads for 'normal_at' and 'refractive_index_at' for your own stuff,
// example (you'll need a separate struct for the different lenses):
//
// vec normal_at(const circle& lens, vec p) { ... }
// double refractive_index_at(const circle& lens, vec p) { ... }
bool inside_of(const simple& lens, vec p);
bool inside_of(const sphere& lens, vec p);
bool inside_of(const funky_sphere& lens, vec p);
vec normal_at(const simple& lens, vec p);
vec normal_at(const sphere& lens, vec p);
vec normal_at(const funky_sphere& lens, vec p);
double refractive_index_at(const simple& lens, vec p);
double refractive_index_at(const sphere& lens, vec p);
double refractive_index_at(const funky_sphere lens, vec p);

// Templated so it can accept any lens type. Stuff will dispatch at compile
// time, so the performance will be good
template <typename T>
ray_array light_gen(vec dim, const T& lens, double max_vel, double angle,
                    double offset);

// Same as above
template <typename T>
void propagate(ray_array& rays, const T& lens,
               double step_size, double max_vel,
               frame &anim);

// Template / Function for sweeping a single ray across the lens
template <typename T>
void propagate_sweep(const T& lens,
                     double step_size, double max_vel,
                     frame &anim);

// Template / function for a modified refractive index during propagation
template <typename T>
void propagate_mod(T& lens,
                   double step_size, double max_vel,
                   frame &anim);

#endif
