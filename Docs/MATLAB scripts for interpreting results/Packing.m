classdef Packing < handle
    properties
        ParticleCoordinates;
        ParticleDiameters;
        BoxDimensions = zeros(1, Constants.DIMENSIONS);
    end
    
    methods
        function porosity = GetActualPorosity(this)
            particlesVolume = this.GetParticlesVolume();
            totalVolume = prod(this.BoxDimensions);
            
            porosity = 1 - particlesVolume / totalVolume;
        end
        
        function particlesVolume = GetParticlesVolume(this)
            if (Constants.DIMENSIONS == 3)
                particlesVolume = pi / 6 * sum(this.ParticleDiameters .^ 3);
            else
                particlesVolume = pi / 4 * sum(this.ParticleDiameters .^ 2);
            end
        end
        
        function [] = FillBoxDimensions(this, porosity, boxSizeRatios)
            particlesVolume = this.GetParticlesVolume();

            % porosity = 1 - particlesVolume / totalVolume;
            totalVolume = particlesVolume / (1 - porosity);
            % totalVolume = prod(BoxSizeRatios) * firstSize^3;
            % firstSize^3 = totalVolume / prod(BoxSizeRatios);
            firstSize = (totalVolume / prod(boxSizeRatios)) ^ (1 / Constants.DIMENSIONS);
            this.BoxDimensions = boxSizeRatios * firstSize;
        end
    end
end

