// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_Headers_BaseConjugateGradientStep_h
#define Generation_PackingGenerators_Headers_BaseConjugateGradientStep_h

#ifdef GSL_AVAILABLE

#include <gsl/gsl_multimin.h>
#include "BasePackingStep.h"
namespace PackingServices { class IPairPotential; }
namespace PackingServices { class IEnergyService; }

namespace PackingGenerators
{
    // Implements a base conjugate gradient optimization step
    class BaseConjugateGradientStep : public BasePackingStep
    {
    protected:
        PackingServices::IEnergyService* energyService;
        PackingServices::IPairPotential* pairPotential;
        std::vector<Core::SpatialVector> particleForces;

        // Minimization parameters
        gsl_multimin_function_fdf functionToMinimize;
        gsl_vector* coordinates;
        gsl_multimin_fdfminimizer* minimizer;
        const gsl_multimin_fdfminimizer_type* algorithmType;

        static const Core::FLOAT_TYPE potentialNormalizer;
        static const Core::FLOAT_TYPE initialStep;
        static const Core::FLOAT_TYPE gradientOrthogonalityTolerance;

    public:
        void SetParticles(Model::Packing* particles);

        void SetContext(const Model::ModellingContext& context);

        ~BaseConjugateGradientStep();

        // Methods for integration with GSL conjugate gradient
        void UpdateParticleCoordinates(const gsl_vector& x);

        virtual Core::FLOAT_TYPE GetPotentialEnergy();

        virtual void FillParticleForces(std::vector<Core::SpatialVector>* particleForces);

        void FillEnergyGradient(gsl_vector* g);

        void ResetGeneration();

    protected:
        BaseConjugateGradientStep(PackingServices::GeometryService* geometryService,
                PackingServices::INeighborProvider* neighborProvider,
                PackingServices::MathService* mathService,
                PackingServices::IPairPotential* pairPotential,
                PackingServices::IEnergyService* energyService);

        void InitializeMinimization();

        virtual void ResetOuterDiameterRatio() = 0;

    private:
        void InitializeMinimizationAlgorithm();

        void FreeGenerationData();

        void FillCoordinates(const Model::Packing& particles, gsl_vector* coordinates);
    };

    double GetPotentialEnergyGlobal(const gsl_vector* x, void * params);

    void FillEnergyGradientGlobal(const gsl_vector* x, void* params, gsl_vector* g);

    void FillEnergyAndGradientGlobal(const gsl_vector* x, void* params, double* f, gsl_vector* g);
}

#endif // GSL

#endif /* Generation_PackingGenerators_Headers_BaseConjugateGradientStep_h */
