function results = read_rxs_log(filename, maxCSINumber)
%read_rxs_log Parse PicoScenes generated .csi file using mex function RXSParser.
%   It invokes the mex function RXSParser to decode the data entries.
%   RXSParser will be automatically compiled, if the mex function does not exist.
%
    if nargin < 2
        maxCSINumber = intmax;
    end

    if exist('RXSParser', 'file') ~= 3
        compileRXSParser
    end
    
    if exist('RXSParser', 'file') ~= 3
        error('RXSParer compilation fails');
    end
    
    ticStart = tic;
    fp = fopen(filename, 'rb');
    if (fp < 0)
        error('Cannot open file %s', filename);
    end

    resultBatchSize = 10000;
    results = cell(resultBatchSize, 1);
    count = 1;

    while ~feof(fp) && count <= maxCSINumber
        segmentLength = fread(fp, 1, 'uint32') + 4;
        if isempty(segmentLength)
            break;
        end
        fseek(fp, -4, 'cof');
        bytes=fread(fp, segmentLength,'uint8=>uint8');
        csi_entry = RXSParser(bytes);
        if isempty(csi_entry) % in very rare case, the data is corrupted.
            continue;
        end
        
        if count == numel(results)
            results = [results; cell(resultBatchSize, 1)];
        end
        results{count} = csi_entry;
        count = count + 1;
    end
    results(count:end)= [];

    disp([num2str(length(results)) ' PicoScenes frames are decoded in ' num2str(toc(ticStart)) ' seconds.']);
    fclose(fp);
    end
