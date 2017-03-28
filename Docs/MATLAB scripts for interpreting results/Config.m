classdef Config
    properties
        ParticlesCount = 10000;
        BoxDimensions = ones(1, Constants.DIMENSIONS);
        GenerationStart = 0;
        Seed = 0;
        StepsToWrite = 1000;
        BoundariesMode = 1;
        ContractionRate = 1e-5;
    end
    
    methods
        function this = Config(packing)
            if (nargin > 0)
                this.ParticlesCount = length(packing.ParticleDiameters);
                this.BoxDimensions = packing.BoxDimensions;
            end
        end
    end
end

