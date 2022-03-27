function openbbsignals(filePath, varargin)

if isempty(varargin)
    skipLength = 0;
    readLength = inf;
else
    skipLength = varargin{1};
    readLength = varargin{2};
end

[~,fileName,extension] = fileparts(filePath);
validName = matlab.lang.makeValidName(fileName);
disp(['Loading PicoScenes SDR baseband signal file: ' fileName extension]);
bb_signal = loadBasebandSignalFile(filePath, skipLength, readLength);
assignin('base', validName, bb_signal);
disp(['Loaded variable name: ' validName]);

end

function data = loadBasebandSignalFile(bbFilePath,skipLength, readLength)
    fid = fopen(bbFilePath);
    fileHeader = fread(fid, 2, 'char=>char');
    bbFileVersion = fread(fid, 2, 'char=>char');
    numDimensions = fread(fid, 1, 'uint8=>double');
    dimensions = ones(1, numDimensions);
    if bbFileVersion(2) - 48 == 1
        for i = 1: numDimensions
            dimensions(i) = fread(fid, 1, 'int32=>double');
         end
    elseif bbFileVersion(2) - 48  == 2
        for i = 1: numDimensions
            dimensions(i) = fread(fid, 1, 'int64=>double');
         end
    end
    isComplexMatrix = double(fread(fid, 1, 'char=>double') == 'C');
    typeChar = fread(fid, 1, 'char=>double');
    typeBits = fread(fid, 1, 'uint8=>double');
    majority = SignalStorageMajority(fread(fid, 1, 'uint8=>double'));

    if any(fileHeader' ~= 'BB') || (any(bbFileVersion' ~= 'v1') && any(bbFileVersion' ~= 'v2'))
        error(' ** incompatible .bbsignals file format! **');
    end
    
    if typeChar == 'D'
        precision = 'float';
    elseif typeChar == 'F'
        precision = 'float';
    elseif typeChar =='U'
        precision = 'uint';
    elseif typeChar == 'I'
        precision = 'int';
    elseif typeChar == 'L'
        precision = 'uint';
    end
    precision = [precision num2str(typeBits)];
    precision = [precision '=>' precision];

    skipBytes = typeBits * skipLength / 8 * 2^isComplexMatrix * dimensions(2);
    readBytes = readLength * 2^isComplexMatrix * dimensions(2);

    fseek(fid, skipBytes, 'cof');
    data = fread(fid, readBytes, precision);
    if isComplexMatrix
        data = reshape(data, 2, []).';
        data = complex(data(:,1), data(:,2));
    end
    if dimensions(1) < 0 % streaming file
        dimensions(1) = numel(data) / prod(dimensions(2:end));
    end

    if majority == SignalStorageMajority.ColumnMajor
        data = reshape(data, dimensions);
    elseif majority == SignalStorageMajority.RowMajor
        data = reshape(data, flip(dimensions));
        data = ipermute(data, numel(dimensions):-1:1);
    end
end