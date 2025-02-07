function writeBBSignals(signal, bbFileName, storageMajority, precision)
    if nargin < 3
        % Default to row-major storage (counter-intuitive for MATLAB) for compatibility with LoadBBSignals
        storageMajority = SignalStorageMajority.RowMajor;
    end
    
    if nargin < 4
        precision = 'int16';
    end

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
        if isfloat(signal) && strcmpi('int16', precision)
            signal = cast(signal * 32768, precision);
        elseif isfloat(signal) && strcmpi('int8', preendcision)
            signal = cast(signal * 256, precision);
        else
            signal = cast(signal, precision);
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