// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/BaseConjugateGradientStep.h"
#include "Generation/PackingServices/DistanceServices/Headers/DistanceService.h"
#include "Generation/PackingServices/EnergyServices/Headers/IEnergyService.h"
#include "Generation/PackingServices/EnergyServices/Headers/IPairPotential.h"
#include "Generation/Model/Headers/Config.h"
#include "Generation/Geometries/Headers/IGeometry.h"

#ifdef GSL_AVAILABLE

using namespace std;
using namespace Core;
using namespace Model;
using namespace PackingServices;

namespace PackingGenerators
{
        const FLOAT_TYPE BaseConjugateGradientStep::potentialNormalizer = 1e4;
        const FLOAT_TYPE BaseConjugateGradientStep::initialStep = 1e-6;
        const FLOAT_TYPE BaseConjugateGradientStep::gradientOrthogonalityTolerance = 1e-5;

    BaseConjugateGradientStep::BaseConjugateGradientStep(GeometryService* geometryService,
            INeighborProvider* neighborProvider,
            MathService* mathService,
            IPairPotential* pairPotential,
            IEnergyService* energyService) :
            BasePackingStep(geometryService, neighborProvider, NULL, mathService)
    {
        this->pairPotential = pairPotential;
        minimizer = NULL;

        // For some reason GSL optimization methods stop too early, when energy is computed without multiplying by 10000
        // (probably it's too low, or the gradients are too low, and the internal termination criteria fire).
        this->energyService = energyService;
        this->energyService->SetPotentialNormalizer(potentialNormalizer);
    }

    BaseConjugateGradientStep::~BaseConjugateGradientStep()
    {
        FreeGenerationData();
    }

    void BaseConjugateGradientStep::SetContext(const ModellingContext& context)
    {
        BasePackingStep::SetContext(context);
        energyService->SetContext(context);
    }

    void BaseConjugateGradientStep::SetParticles(Packing* particles)
    {
        FreeGenerationData();

        BasePackingStep::SetParticles(particles);
        energyService->SetParticles(*particles);

        DistanceService distanceService(mathService, geometryService, neighborProvider);
        distanceService.SetContext(*context);
        distanceService.SetParticles(*particles);
        ParticlePair closestPair = distanceService.FindClosestPair();
        innerDiameterRatio = sqrt(closestPair.normalizedDistanceSquare);

        particleForces.resize(config->particlesCount); // Config may have changed since previous generation

        InitializeMinimizationAlgorithm();

        // It's very important to set up an outer diameter ratio before calling InitializeMinimization(),
        // as potential energy will be estimated there by the minimization routine, and will be much lower than any estimation during actual minimization,
        // so algorithm will never continue. Still, to estimate theoreticalPorosity we need to call BaseConjugateGradientStep::SetParticles(particles) first.
        ResetOuterDiameterRatio();
    }

    void BaseConjugateGradientStep::ResetGeneration()
    {
        ResetOuterDiameterRatio();
    }

    void BaseConjugateGradientStep::InitializeMinimizationAlgorithm()
    {
        functionToMinimize.n = config->particlesCount * DIMENSIONS;
        functionToMinimize.f = GetPotentialEnergyGlobal;
        functionToMinimize.df = FillEnergyGradientGlobal;
        functionToMinimize.fdf = FillEnergyAndGradientGlobal;
        functionToMinimize.params = this;

        coordinates = gsl_vector_alloc(config->particlesCount * DIMENSIONS);
        FillCoordinates(*particles, coordinates);

        // algorithmType = gsl_multimin_fdfminimizer_conjugate_fr; // Fletcher-Reeves
        // algorithmType = gsl_multimin_fdfminimizer_vector_bfgs2; // Broyden-Fletcher-Goldfarb-Shanno (BFGS)
        algorithmType = gsl_multimin_fdfminimizer_conjugate_pr; // Polak-Ribiere. Much better than Fletcher-Reeves, and even better than BFGS. But for very dense packings bfgs2 is still better.
        // algorithmType = gsl_multimin_fdfminimizer_steepest_descent;

        minimizer = gsl_multimin_fdfminimizer_alloc(algorithmType, config->particlesCount * DIMENSIONS);
    }

    void BaseConjugateGradientStep::InitializeMinimization()
    {
        // int gsl_multimin_fdfminimizer_set (gsl_multimin_fdfminimizer* s, gsl_multimin_function_fdf* fdf, const gsl_vector& x, double step_size, double tol)
        // The size of the first trial step is given by step_size.
        // The last parameter is tolerance of a single iteration (line minimization).
        // The line minimization is considered successful if the gradient of the function g is orthogonal to the current search direction p to a relative accuracy of tol,
        // where dot(p,g) < tol |p| |g|.
        gsl_multimin_fdfminimizer_set(minimizer, &functionToMinimize, coordinates, initialStep * potentialNormalizer, gradientOrthogonalityTolerance * potentialNormalizer);
    }

    void BaseConjugateGradientStep::FillCoordinates(const Model::Packing& particles, gsl_vector* coordinates)
    {
        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            const DomainParticle& particle = particles[particleIndex];
            for (int i = 0; i < DIMENSIONS; ++i)
            {
                gsl_vector_set(coordinates, particleIndex * DIMENSIONS + i, particle.coordinates[i]);
            }
        }
    }

    void BaseConjugateGradientStep::FreeGenerationData()
    {
        if (minimizer != NULL)
        {
            gsl_vector_free(coordinates);
            gsl_multimin_fdfminimizer_free(minimizer);
        }
    }

    void BaseConjugateGradientStep::UpdateParticleCoordinates(const gsl_vector& x)
    {
        Model::Packing& particlesRef = *particles;
        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            DomainParticle& particle = particlesRef[particleIndex];
            energyService->StartMove(particle.index);

            for (int i = 0; i < DIMENSIONS; ++i)
            {
                particle.coordinates[i] = gsl_vector_get(&x, particleIndex * DIMENSIONS + i);
            }
            geometry->EnsureBoundaries(particle, &particle, innerDiameterRatio);

            energyService->EndMove();
        }
    }

    FLOAT_TYPE BaseConjugateGradientStep::GetPotentialEnergy()
    {
        vector<FLOAT_TYPE> contractionRatios(1, 1.0 / outerDiameterRatio);
        vector<const IPairPotential*> pairPotentials(1, pairPotential);
        IEnergyService::EnergiesResult result = energyService->GetContractionEnergies(contractionRatios, pairPotentials);
        innerDiameterRatio = sqrt(result.closestPair.normalizedDistanceSquare);

        return result.contractionEnergies[0];
    }

    void BaseConjugateGradientStep::FillParticleForces(std::vector<Core::SpatialVector>* particleForces)
    {
        ParticlePair closestPair = energyService->FillParticleForces(1.0 / outerDiameterRatio, *pairPotential, particleForces);
        innerDiameterRatio = sqrt(closestPair.normalizedDistanceSquare);
    }

    void BaseConjugateGradientStep::FillEnergyGradient(gsl_vector* g)
    {
        FillParticleForces(&particleForces);
        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            for (int i = 0; i < DIMENSIONS; ++i)
            {
                gsl_vector_set(g, particleIndex * DIMENSIONS + i, - particleForces[particleIndex][i]);
            }
        }
    }

    double GetPotentialEnergyGlobal(const gsl_vector* x, void* params)
    {
        BaseConjugateGradientStep* step = (BaseConjugateGradientStep*)params;
        step->UpdateParticleCoordinates(*x);
        FLOAT_TYPE potentialEnergy = step->GetPotentialEnergy();

        // This funtion is used in line minimization and will be called very often
        printf("Called GetPotentialEnergyGlobal. Energy %g\n",  potentialEnergy);

        return potentialEnergy;
    }

    void FillEnergyGradientGlobal(const gsl_vector* x, void* params, gsl_vector* g)
    {
        // printf("FillEnergyGradientGlobal\n");
        BaseConjugateGradientStep* step = (BaseConjugateGradientStep*)params;
        step->UpdateParticleCoordinates(*x);
        step->FillEnergyGradient(g);

        printf("Called FillEnergyGradientGlobal\n");
    }

    void FillEnergyAndGradientGlobal(const gsl_vector* x, void* params, double* f, gsl_vector* g)
    {
        BaseConjugateGradientStep* step = (BaseConjugateGradientStep*)params;
        step->UpdateParticleCoordinates(*x);
        *f = step->GetPotentialEnergy();
        step->FillEnergyGradient(g);

        printf("Called FillEnergyAndGradientGlobal. Energy %g\n", *f);
    }
}

#endif

