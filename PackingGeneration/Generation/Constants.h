// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_Constants_h
#define Generation_Constants_h

#include <string>
#include "Core/Headers/Constants.h"

namespace Generation
{
    const Core::FLOAT_TYPE TOLERANCE = 1.0001;

    const std::string CONFIG_FILE_NAME = "generation.conf";
    const std::string PACKING_FILE_NAME = "packing.xyzd";
    const std::string PREVIOUS_PACKING_FILE_NAME = "packing_prev.xyzd";
    const std::string PACKING_HISTORY_FILE_NAME = "packing_hist.xyzd";
    const std::string PACKING_FILE_NAME_NFO = "packing.nfo";
    const std::string DIAMETERS_FILE_NAME = "diameters.txt";
    const std::string INIT_PACKING_FILE_NAME = "packing_init.xyzd";
    const std::string INSERTION_RADII_FILE_NAME = "insertion_radii.txt";
    const std::string DISTANCES_TO_CLOSEST_SURFACES_FOLDER_NAME = "distances_to_closest_surfaces";
    const std::string CONTACT_NUMBER_DISTRIBUTION_FILE_NAME = "contact_number_distribution.txt";
    const std::string CONTACTING_NEIGHBORS_FILE_NAME = "contacting_neighbors.txt";
    const std::string CONTACTING_NEIGHBOR_DISTANCES_FILE_NAME = "contacting_neighbor_distances.txt";
    const std::string ENTROPY_FILE_NAME = "entropy.txt";
    const std::string PARTICLE_DIRECTIONS_FILE_NAME = "particle_directions.txt";
    const std::string CONTRACTION_ENERGIES_FILE_NAME = "contraction_energies.txt";
    const std::string DISPLACEMENT_ENERGIES_FILE_NAME = "displacement_energy_freqs.txt";
    const std::string ORDER_FILE_NAME = "orders.txt";
    const std::string HESSIAN_EIGENVALUES_FILE_NAME = "hessian_eigenvalues.txt";
    const std::string PRESSURES_FILE_NAME = "pressures.txt";
    const std::string MOLECULAR_DYNAMICS_STATISTICS_FILE_NAME = "molecular_dynamics_statistics.txt";
    const std::string PAIR_CORRELATION_FUNCTION_FILE_NAME = "pair_correlation_function.txt";
    const std::string STRUCTURE_FACTOR_FILE_NAME = "structure_factor.txt";
    const std::string LOCAL_ORIENTATIONAL_DISORDER_FILE_NAME = "local_orientational_disorder.txt";
    const std::string IMMEDIATE_MOLECULAR_DYNAMICS_STATISTICS_FILE_NAME = "immediate_molecular_dynamics_statistics.txt";
    const std::string NEAREST_NEIGHBORS_FILE_NAME = "nearest_neighbors.txt";
    const std::string ACTIVE_GEOMETRY_FILE_NAME = "active_geometry.conf";
    const std::string IMMOBILE_PARTICLES_FILE_NAME = "immobileParticleIndexes.txt";
    const std::string SUCCESSFUL_PERMUTATION_PROBABILITY_FILE_NAME = "successful_permutation_probability.txt";
}

#endif /* Generation_Constants_h */

