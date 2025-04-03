function writeBBSignals(signal, bbFileName, storageMajority, precision, useV3Format, maxRowsPerBlock)
    if nargin < 3
        % Default to row-major storage (counter-intuitive for MATLAB) for compatibility with LoadBBSignals
        storageMajority = SignalStorageMajority.RowMajor;
    end
    
    if nargin < 4
        precision = 'int16';
    end

    if nargin < 5
        useV3Format = true;
    end

    if useV3Format
        maxRowsPerBlock = 10000; % Default to 10000 rows per block
        writeBBSignalsV3(signal, bbFileName, storageMajority, precision, maxRowsPerBlock);
    else
        writeBBSignalsV2(signal, bbFileName, storageMajority, precision);
    end
end

function writeBBSignalsV2(signal, bbFileName, storageMajority, precision)
    bbFilePath = [bbFileName '.bbsignals'];
    fid = fopen(bbFilePath, 'w');
    fwrite(fid, 'BB', 'char');
    fwrite(fid, 'v2', 'char');
    fwrite(fid, uint8(numel(size(signal))), 'uint8');
    for i = 1 : numel(size(signal))
        fwrite(fid, uint64(size(signal, i)), 'uint64');
    end

    if isreal(signal)
        fwrite(fid, 'R', 'char');
    else
        fwrite(fid, 'C', 'char');
    end

    if ~isempty(precision)
        if isfloat(signal)
            if strcmpi(precision, 'int16')
                scaleFactor = 32768;
            elseif strcmpi(precision, 'int8')
                scaleFactor = 256;
            else
                scaleFactor = 1; % No scaling
            end
        else
            scaleFactor = 1; % No scaling for non-float data types
        end
        scaleResponse = input(['Apply default scale factor ', num2str(scaleFactor), '? Y/N/<custom scale factor> (Y=default, case ignored): '], 's');
        if isempty(scaleResponse) || strcmpi(scaleResponse, 'Y')
            signal = cast(signal * scaleFactor, precision);
            disp(['Scale factor applied: ', num2str(scaleFactor)]);
        elseif strcmpi(scaleResponse, 'N')
            signal = cast(signal, precision);
        else
            customScaleFactor = str2double(scaleResponse);
            if isnan(customScaleFactor)
                disp('Invalid scale factor entered. No scaling applied.');
                signal = cast(signal, precision);
            else
                signal = cast(signal * customScaleFactor, precision);
                disp(['Custom scale factor applied: ', num2str(customScaleFactor)]);
            end
        end
    end
    
    if isa(signal, 'double')
        fwrite(fid, 'D', 'char');
    elseif isfloat(signal)
        fwrite(fid, 'F', 'char');
    elseif islogical(signal)
        fwrite(fid, 'L', 'char');
    elseif ischar(signal)
        fwrite(fid, 'C', 'char');
    elseif isinteger(signal)
        typename = class(signal);
        if typename(1) == 'u'
            fwrite(fid, 'U', 'char');
        elseif typename(1) == 'i'
            fwrite(fid, 'I', 'char');
        end
    end

    if isempty(precision)
        precision = class(signal);
    end
    fwrite(fid, sizeof(precision) * 8, 'uint8');
    fwrite(fid, uint8(storageMajority), 'uint8');
        
    if storageMajority == SignalStorageMajority.ColumnMajor
        if ~isreal(signal)
            signalC = cat(ndims(signal) + 1, real(signal), imag(signal));
            signal = permute(signalC, [ndims(signal) + 1 1 : ndims(signal)]);
        end
        signal = reshape(signal, [], 1);
    else
        if ~isreal(signal)
            signal = cat(ndims(signal) + 1, real(signal), imag(signal));
        end

        signal = permute(signal, flip(1:ndims(signal)));
        signal = reshape(signal, [], 1);
    end
    
    fwrite(fid, signal, precision);
    fclose(fid);

    disp(['.bbsignals file written (v2): ' bbFilePath]);
    fileInfo = dir(bbFilePath);
    disp(['File size: ' num2str(fileInfo.bytes) ' bytes']);
end

function writeBBSignalsV3(signal, bbFileName, storageMajority, precision, maxRowsPerBlock)
    bbFilePath = [bbFileName '.bbsignals'];
    fid = fopen(bbFilePath, 'w');
    
    % Write file header
    fwrite(fid, 'BB', 'char');
    fwrite(fid, 'v3', 'char');
    fwrite(fid, uint8(numel(size(signal))), 'uint8');
    fwrite(fid, int64(-1), 'int64');
    for i = 2 : numel(size(signal))
        fwrite(fid, uint64(size(signal, i)), 'uint64');
    end

    if isreal(signal)
        fwrite(fid, 'R', 'char');
    else
        fwrite(fid, 'C', 'char');
    end

    % Handle precision and scaling
    if ~isempty(precision)
        if isfloat(signal)
            if strcmpi(precision, 'int16')
                scaleFactor = 32768;
            elseif strcmpi(precision, 'int8')
                scaleFactor = 256;
            else
                scaleFactor = 1; % No scaling
            end
        else
            scaleFactor = 1; % No scaling for non-float data types
        end
        scaleResponse = input(['Apply default scale factor ', num2str(scaleFactor), '? Y/N/<custom scale factor> (Y=default, case ignored): '], 's');
        if isempty(scaleResponse) || strcmpi(scaleResponse, 'Y')
            signal = cast(signal * scaleFactor, precision);
            disp(['Scale factor applied: ', num2str(scaleFactor)]);
        elseif strcmpi(scaleResponse, 'N')
            signal = cast(signal, precision);
        else
            customScaleFactor = str2double(scaleResponse);
            if isnan(customScaleFactor)
                disp('Invalid scale factor entered. No scaling applied.');
                signal = cast(signal, precision);
            else
                signal = cast(signal * customScaleFactor, precision);
                disp(['Custom scale factor applied: ', num2str(customScaleFactor)]);
            end
        end
    end
    
    % Write data type information
    if isa(signal, 'double')
        fwrite(fid, 'D', 'char');
    elseif isfloat(signal)
        fwrite(fid, 'F', 'char');
    elseif islogical(signal)
        fwrite(fid, 'L', 'char');
    elseif ischar(signal)
        fwrite(fid, 'C', 'char');
    elseif isinteger(signal)
        typename = class(signal);
        if typename(1) == 'u'
            fwrite(fid, 'U', 'char');
        elseif typename(1) == 'i'
            fwrite(fid, 'I', 'char');
        end
    end

    if isempty(precision)
        precision = class(signal);
    end
    fwrite(fid, sizeof(precision) * 8, 'uint8');

    % Get signal dimensions
    signalSize = size(signal);
    numRows = signalSize(1);
    % Calculate number of blocks needed
    numBlocks = ceil(numRows / maxRowsPerBlock);

    % Process each block
    for blockIdx = 0:(numBlocks-1)
        % Calculate block dimensions
        startRow = blockIdx * maxRowsPerBlock + 1;
        endRow = min((blockIdx + 1) * maxRowsPerBlock, numRows);
        rowsInBlock = endRow - startRow + 1;

        % Write block header
        fwrite(fid, uint8(storageMajority), 'uint8'); % Storage majority for this block
        fwrite(fid, uint64(rowsInBlock), 'int64'); % Number of rows in this block

        blockData = signal(startRow:endRow, :);
        if storageMajority == SignalStorageMajority.ColumnMajor
            if ~isreal(blockData)
                blockDataC = cat(ndims(blockData) + 1, real(blockData), imag(blockData));
                blockData = permute(blockDataC, [ndims(blockData) + 1 1 : ndims(blockData)]);
            end

            blockData = reshape(blockData, [], 1);
        else
            if ~isreal(blockData)
                blockData = cat(ndims(blockData) + 1, real(blockData), imag(blockData));
            end
    
            blockData = permute(blockData, flip(1:ndims(blockData)));
            blockData = reshape(blockData, [], 1);
        end
        fwrite(fid, blockData, precision);
    end

    fclose(fid);

    disp(['.bbsignals file written (v3): ' bbFilePath]);
    fileInfo = dir(bbFilePath);
    disp(['File size: ' num2str(fileInfo.bytes) ' bytes']);
end

function S = sizeof(V)
    switch lower(V)
      case {'double', 'int64', 'uint64'}
        S = 8;
      case {'single', 'int32', 'uint32'}
        S = 4;
      case {'char', 'int16', 'uint16'}
        S = 2;
      case {'logical', 'int8', 'uint8'}
        S = 1;
      otherwise
        error('Class "%s" is not supported.', V);
    end
end