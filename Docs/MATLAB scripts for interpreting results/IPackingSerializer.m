classdef IPackingSerializer < handle
    methods (Abstract)
        [packing floatDataType] = ReadPacking(this, packingFilePath, particlesCount);
        [] = WritePacking(this, packingFilePath, packing, floatDataType);
        
        config = ReadDefaultConfig(this, folder);
        [] = WriteDefaultConfig(this, folder, config);
        
        config = ReadConfig(this, configFilePath);
        [] = WriteConfig(this, configFilePath, config);
    end
end

