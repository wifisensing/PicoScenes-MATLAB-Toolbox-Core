function [rxsBundles] = parseRXSBundle(rxsBundleFilePathOrRxsCells, maxRxSLogNumber)

    if nargin < 2
        maxRxSLogNumber = intmax;
    end

    if ischar(rxsBundleFilePathOrRxsCells)
        rxs_cells = read_rxs_log(rxsBundleFilePathOrRxsCells, maxRxSLogNumber);
    elseif iscell(rxsBundleFilePathOrRxsCells)
        rxs_cells = rxsBundleFilePathOrRxsCells;
    end

    if isempty(rxs_cells)
        rxsBundles = [];
        return;
    end

    rtt_num = cellfun(@(x) numel(x), rxs_cells);
    rtt_outlierIndex = find(rtt_num ~= median(rtt_num));
    tones_num = cellfun(@(x) double(x(1).CSI.NumTones) * double(x(1).CSI.NumTx + x(1).CSI.NumESS) * double(x(1).CSI.NumRx) * double(x(1).CSI.NumCSI), rxs_cells);    
    tones_outlierIndex = find(tones_num ~= median(tones_num));
    outlierIndex = unique([rtt_outlierIndex; tones_outlierIndex]);
    if numel(outlierIndex) < numel(rxs_cells) * 0.05
        rxs_cells(outlierIndex) = [];
    end
%     if isfield(rxs_cells, 'BasebandSignals')
%         basebandSignalLengths = cellfun(@(x) numel(x.BasebandSignals) , rxs_cells);
%         outlierIndex = basebandSignalLengths ~= median(basebandSignalLengths);
%     end
    rxs_struct_array = [rxs_cells{:}]';
    rxsBundles = cell(1, size(rxs_struct_array, 2));

    for i = 1:size(rxs_struct_array, 2)
        rxsBundles{i} = structRecursiveMerge(rxs_struct_array(:, i));
    end

end

function mergedStruct = structRecursiveMerge(structArray)
    mergedStruct = struct;
    fieldNames = fieldnames(structArray)';
    numElement = numel(structArray);
    for fieldName = fieldNames
        nameString = fieldName{1};

        try
            if ~isstruct(structArray(1).(nameString))  && ~isscalar(structArray(1).(nameString)) && isnumeric(structArray(1).(nameString))
                if ~strcmp(fieldName, "PhaseSlope") && ~strcmp(fieldName, "PhaseIntercept")
                    mergedData = cell2mat(arrayfun(@(x) reshape(x.(nameString), 1, []), reshape(structArray, [], 1), 'UniformOutput', false));
                else % This is to avoid the crashing bug of MATLAB R2022b
                    mergedData = [];
                end
            else
                mergedData = [structArray.(nameString)];
            end
        catch e
            warning(['structRecursiveMerge fails on data merging for the field: ' nameString]);
            rethrow e;
        end
        numColumn = numel(mergedData) / numElement;

        
        if isnumeric(mergedData) && isrow(mergedData)
            mergedData = reshape(mergedData, numColumn, numElement).';
        elseif isstruct(mergedData)
            mergedData = structRecursiveMerge(mergedData);
        elseif iscell(mergedData)  
             mergedData = mergedData.';
        end
        mergedStruct.(nameString) = mergedData;
    end
%     mergedStruct
end