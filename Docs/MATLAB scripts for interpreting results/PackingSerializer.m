classdef PackingSerializer < IPackingSerializer
    methods
        function [packing, floatDataType] = ReadPacking(this, packingFilePath, particlesCount)
            floatsPerParticle = Constants.DIMENSIONS + 1;
            if (nargin == 3) % particlesCount specified
                % We can determine precision of the packing.xyzd (either single or double) dynamically now.
                % particlesCount can be read from config
                packingData = dir(packingFilePath);
                packingSize = packingData.bytes;
                bytesPerFloat = packingSize / particlesCount / floatsPerParticle;
                if (bytesPerFloat == 4)
                    floatDataType = 'float32';
                elseif (bytesPerFloat == 8)
                    floatDataType = 'float64';
                else
                    error('Unexpected packing file format');
                end
            else % particlesCount not specified. Assume double precision by default (it is used in the C++ code on github as well)
                floatDataType = 'float64';
            end

            input = fopen(packingFilePath , 'r');
            skipNothing = 0;
            packingArray = fread(input, Inf, floatDataType, skipNothing, 'ieee-le');
            fclose(input);
            
            numberOfSpheres = length(packingArray) / floatsPerParticle;

            packingArray = reshape(packingArray, floatsPerParticle, numberOfSpheres);
            packingArray = packingArray';

            packing = Packing();
            packing.ParticleCoordinates = packingArray(:, 1 : Constants.DIMENSIONS);
            packing.ParticleDiameters = packingArray(:, Constants.DIMENSIONS + 1);
        end

        function [] = WritePacking(this, packingFilePath, packing, floatDataType)
            if (nargin == 3)
                floatDataType = 'float64';
            end
            
            particlesCount = length(packing.ParticleDiameters);
            packingArray = zeros(Constants.DIMENSIONS + 1, particlesCount);
            packingArray(1 : Constants.DIMENSIONS, :) =  packing.ParticleCoordinates';
            packingArray(Constants.DIMENSIONS + 1, :) = packing.ParticleDiameters;
            packingFile = fopen(packingFilePath, 'wb', 'ieee-le');
            fwrite(packingFile, packingArray, floatDataType);
            fclose(packingFile);
        end

        function packing = ReadDefaultPacking(this, folder, particlesCount)
            packingFilePath = fullfile(folder, 'packing.xyzd');
            if (nargin == 3)
                packing = this.ReadPacking(packingFilePath, particlesCount);
            else
                packing = this.ReadPacking(packingFilePath);
            end
        end
        
        function config = ReadDefaultConfig(this, folder)
            config = this.ReadConfig(fullfile(folder, 'generation.conf'));
        end
        
        function [] = WriteDefaultPacking(this, folder, packing, floatDataType)
            packingFilePath = fullfile(folder, 'packing.xyzd');
            if (nargin == 3)
                this.WritePacking(packingFilePath, packing);
            else
                this.WritePacking(packingFilePath, packing, floatDataType);
            end
        end

        function [] = WriteDefaultConfig(this, folder, config)
            this.WriteConfig(fullfile(folder, 'generation.conf'), config)
        end

        function config = ReadConfig(this, configFilePath)
            if (isempty(configFilePath))
                error('No config file provided');
            end
            filePointer = fopen(configFilePath, 'r');
            if (filePointer == -1)
                error('Invalid config file provided');
            else
                data = fscanf(filePointer, ['Particles count: %i\n' ...
                'Packing size: %f %f %f\n' ...
                'Generation start: %i\n' ...
                'Seed: %i\n' ...
                'Steps to write: %i\n' ...
                'Boundaries mode: %i\n' ...
                'Contraction rate: %g\n'], 9);
                fclose(filePointer);
                
                if (isempty(data))
                    disp(configFilePath);
                end

                config = Config();
                config.ParticlesCount = data(1);
                config.BoxDimensions = data(2 : 2 + Constants.DIMENSIONS - 1);
                config.GenerationStart = data(5);
                config.Seed = data(6);
                config.StepsToWrite = data(7);
                config.BoundariesMode = data(8);
                config.ContractionRate = data(9);
            end
        end

        function [] = WriteConfig(this, configFilePath, config)
            generationConf = fopen(configFilePath, 'w');
            packingSize = zeros(1, 3);
            packingSize(1 : Constants.DIMENSIONS) = config.BoxDimensions(1 : Constants.DIMENSIONS);
            fprintf(generationConf, ['Particles count: %i\n' ...
                'Packing size: %f %f %f\n' ...
                'Generation start: %i\n' ...
                'Seed: %i\n' ...
                'Steps to write: %i\n' ...
                'Boundaries mode: %i\n' ...
                'Contraction rate: %g\n' ...
                '1. boundaries mode: 1 - bulk; 2 - ellipse (inscribed in XYZ box, Z is length of an ellipse); 3 - rectangle\n' ...
                '2. generationMode = 1 (Poisson, R) or 2 (Poisson in cells, S)'], ...
                config.ParticlesCount, ...
                packingSize(1), packingSize(2), packingSize(3), ...
                config.GenerationStart, ...
                config.Seed, ...
                config.StepsToWrite, ...
                config.BoundariesMode, ...
                config.ContractionRate);
            fclose(generationConf);
        end
        
        function packingInfo = ReadPackingInfo(this, infoFilePath)
            if (isempty(infoFilePath))
                error('No info file provided');
            end
            filePointer = fopen(infoFilePath, 'r');
            if (filePointer == -1)
                error('Invalid info file provided');
            else
                data = fscanf(filePointer, ['N: %i\n' ...
                ' Dimensions: %f %f %f\n' ...
                ' Theoretical Porosity: %f\n' ...
                'Final Porosity: %f (Tolerance: 1.000100)\n' ...
                'Total Simulation Time: %f\n' ...
                'Total Iterations: %i\n' ...
                'Runs count: %i\n'], 9);
                fclose(filePointer);

                packingInfo = PackingInfo();
                packingInfo.ParticlesCount = data(1);
                packingInfo.BoxDimensions = data(2 : 2 + Constants.DIMENSIONS - 1);
                packingInfo.TheoreticalPorosity = data(5);
                packingInfo.FinalPorosity = data(6);
                packingInfo.SimulationTime = data(7);
                packingInfo.TotalIterations = data(8);
                
                if (length(data) == 9)
                    runsCount = data(9);
                else
                    runsCount = 1;
                end
                packingInfo.RunsCount = runsCount;
            end
        end
    end
end

