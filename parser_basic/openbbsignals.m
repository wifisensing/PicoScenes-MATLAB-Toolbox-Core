function openbbsignals(filePath, varargin)

    if isempty(varargin)
        startRatio = 0;
        endRatio = 1;
    else
        startRatio = varargin{1};
    
        if isscalar(varargin)
            endRatio = 1;
        else
            endRatio = varargin(2);
        end
        
    end

    if exist('BBSignalsFileLoader', 'file')
        BBSignalsFileLoader(filePath);
    else
        LoadBBSignals(filePath, startRatio, endRatio);
    end

end