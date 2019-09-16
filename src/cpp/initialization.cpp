#include <cstddef>
#include <cmath>
extern "C" {
  #include "functions.h"
  #include "data.h"
}
#include "initialization.h"
#include "config.h"

// Simulation data structures.
extern Particle *particles;
extern ParticleProperties *properties;
extern Contact *contacts_buffer;
extern double *normal_forces;
extern double *tangent_forces;
extern Vector *forces;
extern Vector *accelerations;
extern Vector *velocities;
extern Vector *displacements;

#ifndef M_PI
  #define M_PI 3.141592653589793
#endif

/**
 * Computes the mass of a particle, given its radius.
 */
double compute_mass(const Config *config) {
  return config->rho * config->thickness * M_PI * config->radius * config->radius;
}


/**
 * Initialize all simulation data structures,
 * according to the simulation size.
 * Returns the number of initialized particles.
 *
 * Note: Except for the particles,
 * all structures are effectively initialized with zeros.
 */
size_t initialize(const Config *config) {
  const double diameter = 2 * config->radius;
  const double mass = compute_mass(config);

  // Maximum of particles on each limit
  const unsigned int max_in_x = floor(config->x_limit / diameter);
  const unsigned int max_in_y = floor(config->y_limit / diameter);

  // The number of particles is equal to the product
  // of the particles that can fill each dimension,
  // plus the falling particle (the first one).
  const size_t num_particles = (max_in_x * max_in_y) +  1;

  // Allocate the memory for all the data structures.
  particles = (Particle*) calloc(num_particles, sizeof(Particle));
  properties = (ParticleProperties*) calloc(num_particles, sizeof(ParticleProperties));
  contacts_buffer = (Contact*) calloc(size_triangular_matrix(num_particles), sizeof(Contact));
  normal_forces = (double*) calloc(num_particles * num_particles, sizeof(double));
  tangent_forces = (double*) calloc(num_particles * num_particles, sizeof(double));
  forces = (Vector*) calloc(num_particles, sizeof(Vector));
  accelerations = (Vector*) calloc(num_particles, sizeof(Vector));
  velocities = (Vector*) calloc(num_particles, sizeof(Vector));
  displacements = (Vector*) calloc(num_particles, sizeof(Vector));

  // Initialize the particles.
  double x = config->radius;
  double y = config->radius;
  for (size_t i = 1; i < num_particles; ++i) {
    particles[i].x_coordinate = x;
    particles[i].y_coordinate = y;
    particles[i].radius = config->radius;
    properties[i].mass = mass;
    properties[i].kn = config->kn;
    properties[i].ks = config->ks;

    // Check if this particle is the last one for this row...
    if ((i % max_in_x) == 0) {
      // If it is, reset the x value and increment y.
      x = config->radius;
      y += diameter;
    } else {
      // If not, increment the x value.
      x += diameter;
    }
  }

  // Initialize the falling particle.
  particles[0].x_coordinate = config->x_limit / 2;
  particles[0].y_coordinate = config->y_limit + (4 * config->radius);
  particles[0].radius = config->radius;
  properties[0].mass = mass;
  properties[0].kn = config->kn;
  properties[0].ks = config->ks;
  velocities[0].y_component = config->v0;

  // Return the number of initialized particles.
  return num_particles;
}
