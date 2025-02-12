function openbbsignals(filePath, varargin)

    if isempty(varargin)
        startRatio = 0;
        endRatio = 1;
    else
        startRatio = varargin{1};
    
        if numel(varargin) >=2
            endRatio = varargin{2};
        end
        
    end

    fileInfo = dir(filePath);
    disp(['File size: ', num2str(fileInfo.bytes / 1e6), ' MiB']);

    % Use BBSignalsFileLoader if exists and file size is larger than 200 MiB.
    if exist('BBSignalsFileLoader', 'file') && fileInfo.bytes > 200e6
        BBSignalsFileLoader(filePath);
    else
        LoadBBSignals(filePath, startRatio, endRatio);
    end

end