classdef ParserPreference
   
    methods (Static)
        function preference = getPreference

            preference.skipBasebandSignals = true;  % If true, remove the parsed baseband signals, which saves a lot of memory
            preference.forceBundledParsing = false; % If true, force all Rx frames to have the same CSI size/parameters to activate bundled parsing.

        end

    end
end
