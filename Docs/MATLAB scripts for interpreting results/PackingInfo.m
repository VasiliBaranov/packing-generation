classdef PackingInfo
    properties
        ParticlesCount = 10000;
        BoxDimensions = ones(1, Constants.DIMENSIONS);
        TheoreticalPorosity = 0;
        FinalPorosity = 0;
        SimulationTime = 0;
        TotalIterations = 0;
        RunsCount = 1;
    end
end

