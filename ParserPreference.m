classdef ParserPreference
   
    methods (Static)
        function preference = getPreference

            preference.skipBasebandSignals = false; % If true, remove the parsed baseband signals, which saves a lot of memory
            preference.doInterpolation = true; % If false, skip the interpolation of CSI
        end

    end
end
