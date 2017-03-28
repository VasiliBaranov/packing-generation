clear all;

folder = '..\Examples';
packingSerializer = PackingSerializer();

%%%%%%%%%%%%%%
% Reading the config

configFilePath = fullfile(folder, 'generation.conf');
config = packingSerializer.ReadConfig(configFilePath);

%%%%%%%%%%%%%%
% Reading the packing

packingFilePath = fullfile(folder, 'packing.xyzd');
[packing, floatDataType] = packingSerializer.ReadPacking(packingFilePath, config.ParticlesCount);

% One can also just call packing = packingSerializer.ReadPacking(packingFilePath);
% config.ParticlesCount helps to determine the real precision with which particles were stored (single or double), 
% otherwise it is assumed to be double. The C++ code on github also uses double precision.

%%%%%%%%%%%%%%
% IMPORTANT!!!
% Final diameters will usually be smaller than the ones you specify in the original packing 
% (they are linearly scaled at the beginning of generation by a small value (scaling factor) 
% and this value increases during generation, but usually not until 1). 
% The final packing (packing.xyzd) will store correct particle centers and the ORIGINAL diameters 
% (not the ones scaled with the final scaling factor)—for historical reasons. 
% To get the final scaling factor and correct final diameters, 
% you have to use the Final Porosity field from the packing.nfo. 
% The equation for the final scaling factor is 
% FinalDensity = 1 - FinalPorosity = N * pi/6 * (D_original * FinalScalingFactor)^3 / (Lx * Ly * Lz) = 
% TheoreticalDensity*FinalScalingFactor^3 = (1-TheoreticalPorosity)*FinalScalingFactor^3. 
% Thus, FinalScalingFactor = ((1 - FinalPorosity) / (1-TheoreticalPorosity))^(1/3).
% The equation is the same for polydisperse packings as well.
% Theoretical porosity is also written in packing.nfo.
% When you scale diameters with the final scaling factor, you don't need to scale the box size or particle centers.

% Reading packing.nfo
infoFilePath = fullfile(folder, 'packing.nfo');
packingInfo = packingSerializer.ReadPackingInfo(infoFilePath);

% Scaling the diameters. You can of course scale the coordinates and the box size instead (with 1/finalScalingFactor).
finalScalingFactor = ((1 - packingInfo.FinalPorosity) / (1 - packingInfo.TheoreticalPorosity))^(1/3);
packing.ParticleDiameters = packing.ParticleDiameters * finalScalingFactor;

% Now you can really use packing.ParticleCoordinates and packing.ParticleDiameters

packing.BoxDimensions = config.BoxDimensions; % just for the sake of completeness

% If you want to save the updated packing, use smth like
% packingSerializer.WritePacking(packingFilePath, packing, floatDataType);
% % packingSerializer.WriteConfig(configFilePath, config); % needed only if you scale particle centers and the box size instead of diameters
% You may also want to update packing.nfo and set TheoreticalPorosity to FinalPorosity to avoid scaling the packing once again 
% the next time you run this script. Or use another name for the scaled packing



