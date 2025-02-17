#include "ModularPicoScenesFrame.hxx"
#ifdef CUSTOM_HEADER_MAPPING_EXISTS
    #include "CustomHeaderMapping.hxx"
#endif

#include "mex.hpp"
#include "mexAdapter.hpp"
#include "MatlabDataArray.hpp"
#include "AntStateInfoSegment.hxx"

//void printFieldNames(const matlab::data::StructArray& structData) {
//    auto fieldNames = structData.getFieldNames();
//    std::cout << "StructArray contains the following fields:" << std::endl;
//    for (const auto& fieldName : fieldNames) {
//        std::cout << "- " << static_cast<std::string>(fieldName) << std::endl;
//    }
//}

inline bool hasField(const matlab::data::StructArray& inStruct, const std::string& fieldName) {
    return std::any_of(
        inStruct.getFieldNames().begin(),
        inStruct.getFieldNames().end(),
        [&](const auto& field) { return static_cast<std::string>(field) == fieldName; }
    );
}

std::string arrayTypeToString(matlab::data::ArrayType type) {
    switch (type) {
        case matlab::data::ArrayType::DOUBLE:
            return "DOUBLE";
        case matlab::data::ArrayType::SINGLE:
            return "SINGLE";
        case matlab::data::ArrayType::INT8:
            return "INT8";
        case matlab::data::ArrayType::UINT8:
            return "UINT8";
        case matlab::data::ArrayType::INT16:
            return "INT16";
        case matlab::data::ArrayType::UINT16:
            return "UINT16";
        case matlab::data::ArrayType::INT32:
            return "INT32";
        case matlab::data::ArrayType::UINT32:
            return "UINT32";
        case matlab::data::ArrayType::INT64:
            return "INT64";
        case matlab::data::ArrayType::UINT64:
            return "UINT64";
        case matlab::data::ArrayType::CHAR:
            return "CHAR";
        case matlab::data::ArrayType::LOGICAL:
            return "LOGICAL";
        case matlab::data::ArrayType::CELL:
            return "CELL";
        case matlab::data::ArrayType::STRUCT:
            return "STRUCT";
        case matlab::data::ArrayType::OBJECT:
            return "OBJECT";
        case matlab::data::ArrayType::ENUM:
            return "ENUM";
        case matlab::data::ArrayType::UNKNOWN:
        default:
            return "UNKNOWN";
    }
}

template <typename T>
T extractScalarField(const matlab::data::StructArray& structArray, const std::string& fieldName) {
    try {
        const auto& field = structArray[0][fieldName];

        if (field.isEmpty()) {
            throw std::runtime_error("Field \"" + fieldName + "\" is empty.");
        }

        auto fieldType = field.getType();
        switch (fieldType) {
            case matlab::data::ArrayType::DOUBLE: {
                auto fieldArray = static_cast<matlab::data::TypedArray<double>>(field);
                return static_cast<T>(fieldArray[0]);
            }
            case matlab::data::ArrayType::SINGLE: {
                auto fieldArray = static_cast<matlab::data::TypedArray<float>>(field);
                return static_cast<T>(fieldArray[0]);
            }
            case matlab::data::ArrayType::INT8: {
                auto fieldArray = static_cast<matlab::data::TypedArray<int8_t>>(field);
                return static_cast<T>(fieldArray[0]);
            }
            case matlab::data::ArrayType::UINT8: {
                auto fieldArray = static_cast<matlab::data::TypedArray<uint8_t>>(field);
                return static_cast<T>(fieldArray[0]);
            }
            case matlab::data::ArrayType::INT16: {
                auto fieldArray = static_cast<matlab::data::TypedArray<int16_t>>(field);
                return static_cast<T>(fieldArray[0]);
            }
            case matlab::data::ArrayType::UINT16: {
                auto fieldArray = static_cast<matlab::data::TypedArray<uint16_t>>(field);
                return static_cast<T>(fieldArray[0]);
            }
            case matlab::data::ArrayType::INT32: {
                auto fieldArray = static_cast<matlab::data::TypedArray<int32_t>>(field);
                return static_cast<T>(fieldArray[0]);
            }
            case matlab::data::ArrayType::UINT32: {
                auto fieldArray = static_cast<matlab::data::TypedArray<uint32_t>>(field);
                return static_cast<T>(fieldArray[0]);
            }
            case matlab::data::ArrayType::INT64: {
                auto fieldArray = static_cast<matlab::data::TypedArray<int64_t>>(field);
                return static_cast<T>(fieldArray[0]);
            }
            case matlab::data::ArrayType::UINT64: {
                auto fieldArray = static_cast<matlab::data::TypedArray<uint64_t>>(field);
                return static_cast<T>(fieldArray[0]);
            }
            case matlab::data::ArrayType::CHAR: {
                auto fieldArray = static_cast<matlab::data::CharArray>(field);
                std::string charValue = fieldArray.toAscii();
                throw std::runtime_error("Field \"" + fieldName + "\" is CHAR and cannot be converted to a scalar.");
            }
            case matlab::data::ArrayType::LOGICAL: {
                auto fieldArray = static_cast<matlab::data::TypedArray<bool>>(field);
                return static_cast<T>(fieldArray[0]);
            }
            default:
                throw std::runtime_error("Unsupported data type for field \"" + fieldName + "\".");
        }
    } catch (const matlab::data::TypeMismatchException& e) {
        std::ostringstream oss;
        oss << "Type mismatch for field \"" << fieldName << "\". Expected type: "
            << typeid(T).name() << ", actual type: " << arrayTypeToString(structArray[0][fieldName].getType())
            << ". Exception: " << e.what();
        throw std::runtime_error(oss.str());
    } catch (const std::exception& e) {
        std::ostringstream oss;
        oss << "Error extracting field \"" << fieldName << "\": " << e.what()
            << ". Field type: " << arrayTypeToString(structArray[0][fieldName].getType());
        throw std::runtime_error(oss.str());
    }
}

template <typename T, typename U, size_t N>
void convertTypedArray(const matlab::data::TypedArray<T>& fieldArray, U (&array)[N]) {
    size_t numElementsToCopy = std::min(N, fieldArray.getNumberOfElements());

    std::transform(fieldArray.begin(), fieldArray.begin() + numElementsToCopy, array,
                   [](const T& value) -> U { return static_cast<U>(value); });

    if (numElementsToCopy < N) {
        std::fill(array + numElementsToCopy, array + N, U{});
    }
}

template <typename T, size_t N>
void extractArrayField(const matlab::data::StructArray& structArray, const std::string& fieldName, T (&array)[N]) {
    try {
        auto field = structArray[0][fieldName];

        if (field.isEmpty()) {
            throw std::runtime_error("Field \"" + fieldName + "\" is empty.");
        }

        switch (field.getType()) {
            case matlab::data::ArrayType::DOUBLE:
                convertTypedArray(static_cast<matlab::data::TypedArray<double>>(field), array);
                break;
            case matlab::data::ArrayType::SINGLE:
                convertTypedArray(static_cast<matlab::data::TypedArray<float>>(field), array);
                break;
            case matlab::data::ArrayType::INT8:
                convertTypedArray(static_cast<matlab::data::TypedArray<int8_t>>(field), array);
                break;
            case matlab::data::ArrayType::UINT8:
                convertTypedArray(static_cast<matlab::data::TypedArray<uint8_t>>(field), array);
                break;
            case matlab::data::ArrayType::INT16:
                convertTypedArray(static_cast<matlab::data::TypedArray<int16_t>>(field), array);
                break;
            case matlab::data::ArrayType::UINT16:
                convertTypedArray(static_cast<matlab::data::TypedArray<uint16_t>>(field), array);
                break;
            case matlab::data::ArrayType::INT32:
                convertTypedArray(static_cast<matlab::data::TypedArray<int32_t>>(field), array);
                break;
            case matlab::data::ArrayType::UINT32:
                convertTypedArray(static_cast<matlab::data::TypedArray<uint32_t>>(field), array);
                break;
            case matlab::data::ArrayType::INT64:
                convertTypedArray(static_cast<matlab::data::TypedArray<int64_t>>(field), array);
                break;
            case matlab::data::ArrayType::UINT64:
                convertTypedArray(static_cast<matlab::data::TypedArray<uint64_t>>(field), array);
                break;
            case matlab::data::ArrayType::LOGICAL:
                convertTypedArray(static_cast<matlab::data::TypedArray<bool>>(field), array);
                break;
            default:
                throw std::runtime_error("Unsupported data type for field \"" + fieldName + "\".");
        }
    } catch (const matlab::data::TypeMismatchException& e) {
        std::cerr << "Type mismatch for field \"" << fieldName << "\": " << e.what() << std::endl;
        throw;
    } catch (const std::runtime_error& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
        throw;
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred while extracting field \"" << fieldName << "\": " << e.what() << std::endl;
        throw;
    } catch (...) {
        std::cerr << "Unknown error occurred while extracting field \"" << fieldName << "\"." << std::endl;
        throw;
    }
}

template <typename SourceType, typename TargetType>
std::vector<TargetType> convertVector(const matlab::data::TypedArray<SourceType>& fieldArray) {
    std::vector<TargetType> result;
    result.reserve(fieldArray.getNumberOfElements());
    for (const auto& value : fieldArray) {
        result.push_back(static_cast<TargetType>(value));
    }
    return result;
}

template <typename SourceType, typename TargetType>
std::vector<std::complex<TargetType>> convertComplexVector(const matlab::data::TypedArray<std::complex<SourceType>>& fieldArray) {
    std::vector<std::complex<TargetType>> result;
    result.reserve(fieldArray.getNumberOfElements());
    for (const auto& value : fieldArray) {
        result.emplace_back(static_cast<TargetType>(value.real()), static_cast<TargetType>(value.imag()));
    }
    return result;
}

template <typename T>
std::vector<T> extractVectorField(const matlab::data::StructArray& structArray, const std::string& fieldName) {
    try {
        auto field = structArray[0][fieldName];
        if (field.isEmpty()) {
            throw std::runtime_error("Field \"" + fieldName + "\" is empty.");
        }

        if constexpr (std::is_same_v<T, std::complex<float>>) {
            auto complexField = static_cast<matlab::data::TypedArray<std::complex<double>>>(field);
            return convertComplexVector<double, typename T::value_type>(complexField);
        }

        switch (field.getType()) {
            case matlab::data::ArrayType::DOUBLE:
                return convertVector<double, T>(static_cast<matlab::data::TypedArray<double>>(field));
            case matlab::data::ArrayType::SINGLE:
                return convertVector<float, T>(static_cast<matlab::data::TypedArray<float>>(field));
            case matlab::data::ArrayType::INT8:
                return convertVector<int8_t, T>(static_cast<matlab::data::TypedArray<int8_t>>(field));
            case matlab::data::ArrayType::UINT8:
                return convertVector<uint8_t, T>(static_cast<matlab::data::TypedArray<uint8_t>>(field));
            case matlab::data::ArrayType::INT16:
                return convertVector<int16_t, T>(static_cast<matlab::data::TypedArray<int16_t>>(field));
            case matlab::data::ArrayType::UINT16:
                return convertVector<uint16_t, T>(static_cast<matlab::data::TypedArray<uint16_t>>(field));
            case matlab::data::ArrayType::INT32:
                return convertVector<int32_t, T>(static_cast<matlab::data::TypedArray<int32_t>>(field));
            case matlab::data::ArrayType::UINT32:
                return convertVector<uint32_t, T>(static_cast<matlab::data::TypedArray<uint32_t>>(field));
            case matlab::data::ArrayType::INT64:
                return convertVector<int64_t, T>(static_cast<matlab::data::TypedArray<int64_t>>(field));
            case matlab::data::ArrayType::UINT64:
                return convertVector<uint64_t, T>(static_cast<matlab::data::TypedArray<uint64_t>>(field));
            case matlab::data::ArrayType::LOGICAL:
                return convertVector<bool, T>(static_cast<matlab::data::TypedArray<bool>>(field));
            default:
                throw std::runtime_error("Unsupported data type for field \"" + fieldName + "\".");
        }
    } catch (const matlab::data::TypeMismatchException& e) {
        std::cerr << "Type mismatch for field \"" << fieldName << "\": " << e.what() << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred while extracting field \"" << fieldName << "\": " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown error occurred while extracting field \"" << fieldName << "\"." << std::endl;
    }
    return {};
}
ieee80211_mac_frame_header convertStruct2StandardHeader(const matlab::data::StructArray& standardHeaderArray) {
    ieee80211_mac_frame_header header;

    try {
        auto controlField = standardHeaderArray[0]["ControlField"];
        auto controlFieldStruct = static_cast<matlab::data::StructArray>(controlField);

        header.fc.version = extractScalarField<uint16_t>(controlFieldStruct, "Version");
        header.fc.type = extractScalarField<uint16_t>(controlFieldStruct, "Type");
        header.fc.subtype = extractScalarField<uint16_t>(controlFieldStruct, "SubType");
        header.fc.toDS = extractScalarField<uint16_t>(controlFieldStruct, "ToDS");
        header.fc.fromDS = extractScalarField<uint16_t>(controlFieldStruct, "FromDS");
        header.fc.moreFrags = extractScalarField<uint16_t>(controlFieldStruct, "MoreFrags");
        header.fc.retry = extractScalarField<uint16_t>(controlFieldStruct, "Retry");
        header.fc.power_mgmt = extractScalarField<uint16_t>(controlFieldStruct, "PowerManagement");
        header.fc.more = extractScalarField<uint16_t>(controlFieldStruct, "More");
        header.fc.protect = extractScalarField<uint16_t>(controlFieldStruct, "Protected");
        header.fc.order = extractScalarField<uint16_t>(controlFieldStruct, "Order");

        header.frag = extractScalarField<uint16_t>(standardHeaderArray, "Fragment");
        header.seq = extractScalarField<uint16_t>(standardHeaderArray, "Sequence");
    } catch (const std::exception& e) {
        std::cerr << "Error converting standard header: " << e.what() << std::endl;
    }

    return header;
}

std::shared_ptr<RxSBasicSegment> convertStruct2RxSBasic(const matlab::data::StructArray& rxsArray) {
    auto segment = std::make_shared<RxSBasicSegment>();
    RxSBasic basic;
    auto structData = rxsArray[0];

    try {
        segment->segmentVersionId = extractScalarField<uint16_t>(rxsArray, "SegmentVersion");
        basic.deviceType = extractScalarField<uint16_t>(rxsArray, "DeviceType");
        basic.tstamp = extractScalarField<uint64_t>(rxsArray, "Timestamp");
        basic.systemTime = extractScalarField<uint64_t>(rxsArray, "SystemTime");
        basic.centerFreq = extractScalarField<int16_t>(rxsArray, "CenterFreq");
        basic.centerFreq2 = extractScalarField<int16_t>(rxsArray, "CenterFreq2");
        basic.controlFreq = extractScalarField<int16_t>(rxsArray, "ControlFreq");
        basic.cbw = extractScalarField<uint16_t>(rxsArray, "CBW");
        basic.packetFormat = extractScalarField<uint8_t>(rxsArray, "PacketFormat");
        basic.pkt_cbw = extractScalarField<uint16_t>(rxsArray, "PacketCBW");
        basic.guardInterval = extractScalarField<uint16_t>(rxsArray, "GI");
        basic.mcs = extractScalarField<uint8_t>(rxsArray, "MCS");
        basic.numSTS = extractScalarField<uint8_t>(rxsArray, "NumSTS");
        basic.numESS = extractScalarField<uint8_t>(rxsArray, "NumESS");
        basic.numRx = extractScalarField<uint8_t>(rxsArray, "NumRx");
        basic.noiseFloor = extractScalarField<int8_t>(rxsArray, "NoiseFloor");
        basic.rssi = extractScalarField<int8_t>(rxsArray, "RSSI");

        basic.rssi_ctl0 = extractScalarField<int8_t>(rxsArray, "RSSI1");
        basic.rssi_ctl1 = extractScalarField<int8_t>(rxsArray, "RSSI2");
        basic.rssi_ctl2 = extractScalarField<int8_t>(rxsArray, "RSSI3");
        basic.rssi_ctl3 = extractScalarField<int8_t>(rxsArray, "RSSI4");
        basic.rssi_ctl4 = extractScalarField<int8_t>(rxsArray, "RSSI5");
        basic.rssi_ctl5 = extractScalarField<int8_t>(rxsArray, "RSSI6");
        basic.rssi_ctl6 = extractScalarField<int8_t>(rxsArray, "RSSI7");
        basic.rssi_ctl7 = extractScalarField<int8_t>(rxsArray, "RSSI8");

        segment->setBasic(basic);
    } catch (const std::exception& e) {
        std::cerr << "Error converting RxSBasic: " << e.what() << std::endl;
    }

    return segment;
}

std::shared_ptr<ExtraInfoSegment> convertStruct2ExtraInfo(const matlab::data::StructArray& structData) {
    if (structData.getFieldNames().begin() == structData.getFieldNames().end()) {
        return nullptr;
    }
    
    ExtraInfo ei; 
    auto segment = std::make_shared<ExtraInfoSegment>();
    segment->segmentVersionId = extractScalarField<uint16_t>(structData, "SegmentVersion");
    ei.featureCode = extractScalarField<uint32_t>(structData, "FeatureCode");
    ei.hasLength = extractScalarField<bool>(structData, "HasLength");
    ei.hasVersion = extractScalarField<bool>(structData, "HasVersion");
    ei.hasMacAddr_cur = extractScalarField<bool>(structData, "HasMacAddr_cur");
    ei.hasMacAddr_rom = extractScalarField<bool>(structData, "HasMacAddr_rom");
    ei.hasChansel = extractScalarField<bool>(structData, "HasChansel");
    ei.hasBMode = extractScalarField<bool>(structData, "HasBMode");
    ei.hasEVM = extractScalarField<bool>(structData, "HasEVM");
    ei.hasTxChainMask = extractScalarField<bool>(structData, "HasTxChainMask");
    ei.hasRxChainMask = extractScalarField<bool>(structData, "HasRxChainMask");
    ei.hasTxpower = extractScalarField<bool>(structData, "HasTxpower");
    ei.hasCF = extractScalarField<bool>(structData, "HasCF");
    ei.hasTxTSF = extractScalarField<bool>(structData, "HasTxTSF");
    ei.hasLastHWTxTSF = extractScalarField<bool>(structData, "HasLastHwTxTSF");
    ei.hasChannelFlags = extractScalarField<bool>(structData, "HasChannelFlags");
    ei.hasTxNess = extractScalarField<bool>(structData, "HasTxNess");
    ei.hasTuningPolicy = extractScalarField<bool>(structData, "HasTuningPolicy");
    ei.hasPLLRate = extractScalarField<bool>(structData, "HasPLLRate");
    ei.hasPLLClkSel = extractScalarField<bool>(structData, "HasPLLClkSel");
    ei.hasPLLRefDiv = extractScalarField<bool>(structData, "HasPLLRefDiv");
    ei.hasAGC = extractScalarField<bool>(structData, "HasAGC");
    ei.hasAntennaSelection = extractScalarField<bool>(structData, "HasAntennaSelection");
    ei.hasSamplingRate = extractScalarField<bool>(structData, "HasSamplingRate");
    ei.hasCFO = extractScalarField<bool>(structData, "HasCFO");
    ei.hasSFO = extractScalarField<bool>(structData, "HasSFO");
    ei.hasTemperature = extractScalarField<bool>(structData, "HasTemperature");
    
    if (ei.hasLength) {
        ei.length = extractScalarField<uint16_t>(structData, "Length");
    }
    if (ei.hasVersion) {
        ei.version = extractScalarField<uint64_t>(structData, "Version");
    }
    if (ei.hasMacAddr_cur) {
        extractArrayField<uint8_t, 6>(structData, "MACAddressCurrent", ei.macaddr_cur);
    }
    if (ei.hasMacAddr_rom) {
        extractArrayField<uint8_t, 6>(structData, "MACAddressROM", ei.macaddr_rom);
    }
    if (ei.hasChansel) {
        ei.chansel = extractScalarField<uint32_t>(structData, "CHANSEL");
    }
    if (ei.hasBMode) {
        ei.bmode = extractScalarField<uint8_t>(structData, "BMode");
    }
    if (ei.hasEVM) {
        extractArrayField<int8_t, 20>(structData, "EVM", ei.evm);
    }
    if (ei.hasTxChainMask) {
        ei.txChainMask = extractScalarField<uint8_t>(structData, "TxChainMask");
    }
    if (ei.hasRxChainMask) {
        ei.rxChainMask = extractScalarField<uint8_t>(structData, "RxChainMask");
    }
    if (ei.hasTxpower) {
        ei.txpower = extractScalarField<uint8_t>(structData, "TxPower");
    }
    if (ei.hasCF) {
        ei.cf = extractScalarField<uint64_t>(structData, "CF");
    }
    if (ei.hasTxTSF) {
        ei.txTSF = extractScalarField<uint64_t>(structData, "TxTSF");
    }
    if (ei.hasLastHWTxTSF) {
        ei.lastHwTxTSF = extractScalarField<uint64_t>(structData, "LastTXTSF");
    }
    if (ei.hasChannelFlags) {
        ei.channelFlags = extractScalarField<uint16_t>(structData, "ChannelFlags");
    }
    if (ei.hasTxNess) {
        ei.tx_ness = extractScalarField<uint8_t>(structData, "TxNess");
    }
    if (ei.hasTuningPolicy) {
        ei.tuningPolicy = static_cast<AtherosCFTuningPolicy>(extractScalarField<uint8_t>(structData, "TuningPolicy"));
    }
    if (ei.hasPLLRate) {
        ei.pll_rate = extractScalarField<uint16_t>(structData, "PLLRate");
    }
    if (ei.hasPLLRefDiv) {
        ei.pll_refdiv = extractScalarField<uint8_t>(structData, "PLLRefDiv");
    }
    if (ei.hasPLLClkSel) {
        ei.pll_clock_select = extractScalarField<uint8_t>(structData, "PLLClkSel");
    }
    if (ei.hasAGC) {
        ei.agc = extractScalarField<uint8_t>(structData, "AGC");
    }
    if (ei.hasAntennaSelection) {
        extractArrayField<uint8_t, 3>(structData, "ANTSEL", ei.ant_sel);
    }
    if (ei.hasSamplingRate) {
        ei.samplingRate = extractScalarField<uint64_t>(structData, "SF");
    }
    if (ei.hasCFO) {
        ei.cfo = extractScalarField<int32_t>(structData, "CFO");
    }
    if (ei.hasSFO) {
        ei.sfo = extractScalarField<int32_t>(structData, "SFO");
    }
    if (ei.hasTemperature) {
        ei.temperature = extractScalarField<int8_t>(structData, "Temperature");
    }
    segment->setExtraInfo(ei);
    return segment;
}

std::shared_ptr<CSISegment> convertStruct2CSISegment(const matlab::data::StructArray& structData) {
    auto segment = std::make_shared<CSISegment>();
    auto csi = std::make_shared<CSI>();
    auto structElement = structData[0];

    try {
        segment->segmentVersionId = extractScalarField<uint16_t>(structData, "SegmentVersion");
        csi->deviceType = static_cast<PicoScenesDeviceType>(extractScalarField<uint16_t>(structData, "DeviceType"));
        csi->firmwareVersion = extractScalarField<uint8_t>(structData, "FirmwareVersion");
        csi->packetFormat = static_cast<PacketFormatEnum>(extractScalarField<int8_t>(structData, "PacketFormat"));
        csi->cbw = static_cast<ChannelBandwidthEnum>(extractScalarField<uint16_t>(structData, "CBW"));
        csi->carrierFreq = extractScalarField<uint64_t>(structData, "CarrierFreq");
        csi->carrierFreq2 = extractScalarField<uint64_t>(structData, "CarrierFreq2");
        csi->isMerged = extractScalarField<bool>(structData, "IsMerged");
        csi->samplingRate = extractScalarField<uint64_t>(structData, "SamplingRate");
        csi->subcarrierBandwidth = extractScalarField<uint32_t>(structData, "SubcarrierBandwidth");
        csi->antSel = extractScalarField<uint8_t>(structData, "ANTSEL");
        csi->dimensions.numTones = extractScalarField<uint16_t>(structData, "NumTones");
        csi->dimensions.numTx = extractScalarField<uint8_t>(structData, "NumTx");
        csi->dimensions.numRx = extractScalarField<uint8_t>(structData, "NumRx");
        csi->dimensions.numESS = extractScalarField<uint8_t>(structData, "NumESS");
        csi->dimensions.numCSI = extractScalarField<uint16_t>(structData, "NumCSI");
        csi->subcarrierIndices = extractVectorField<int16_t>(structData, "SubcarrierIndex");
        csi->timingOffsets = extractVectorField<uint32_t>(structData, "TimingOffsets");

        auto csiData = extractVectorField<std::complex<float>>(structData, "CSI");
        const auto& csiDimensions = csi->dimensions;
        const auto expectedSize = csiDimensions.numTones *
                                  (csiDimensions.numTx + csiDimensions.numESS) *
                                  csiDimensions.numRx *
                                  csiDimensions.numCSI;
        if (csiData.size() != expectedSize) {
            throw std::runtime_error("CSI data size does not match expected dimensions.");
        }
        
        csi->CSIArray = SignalMatrix<std::complex<float>>(
            csiData,
            std::array<uint32_t, 4>{
                csiDimensions.numTones,
                static_cast<uint32_t>(csiDimensions.numTx + csiDimensions.numESS),
                csiDimensions.numRx,
                csiDimensions.numCSI},
                SignalMatrixStorageMajority::ColumnMajor);

        auto magData = extractVectorField<float>(structData, "Mag");
        csi->magnitudeArray = SignalMatrix<float>(
            magData,
            std::array<uint32_t, 4>{
                csiDimensions.numTones,
                static_cast<uint32_t>(csiDimensions.numTx + csiDimensions.numESS),
                csiDimensions.numRx,
                csiDimensions.numCSI},
                SignalMatrixStorageMajority::ColumnMajor);

        auto phaseData = extractVectorField<float>(structData, "Phase");
        csi->phaseArray = SignalMatrix<float>(
            phaseData,
            std::array<uint32_t, 4>{
                csiDimensions.numTones,
                static_cast<uint32_t>(csiDimensions.numTx + csiDimensions.numESS),
                csiDimensions.numRx,
                csiDimensions.numCSI},
                SignalMatrixStorageMajority::ColumnMajor);

        if (structElement["PhaseSlope"].getNumberOfElements() > 0) {
            auto phaseSlopeData = extractVectorField<float>(structData, "PhaseSlope");
            csi->phaseSlope = SignalMatrix<float>(
                phaseSlopeData,
                std::array<uint32_t, 3>{
                    static_cast<uint32_t>(csiDimensions.numTx + csiDimensions.numESS),
                    csiDimensions.numRx,
                    csiDimensions.numCSI},
                    SignalMatrixStorageMajority::ColumnMajor);
        }

        if (structData[0]["PhaseIntercept"].getNumberOfElements() > 0) {
            auto phaseInterceptData = extractVectorField<float>(structData, "PhaseIntercept");
            csi->phaseIntercept = SignalMatrix<float>(
                phaseInterceptData,
                std::array<uint32_t, 3>{
                    static_cast<uint32_t>(csiDimensions.numTx + csiDimensions.numESS),
                    csiDimensions.numRx,
                    csiDimensions.numCSI},
                    SignalMatrixStorageMajority::ColumnMajor);
        }

        segment->setCSI(csi);
    } catch (const std::exception& e) {
        std::cerr << "Error converting CSISegment: " << e.what() << std::endl;
    }
    return segment;
}

std::shared_ptr<SDRExtraSegment> convertStruct2SDRExtra(const matlab::data::StructArray& structData) {
    auto segment = std::make_shared<SDRExtraSegment>();
    SDRExtra sdrExtra;

    segment->segmentVersionId = extractScalarField<uint16_t>(structData, "SegmentVersion");
    sdrExtra.scramblerInit = extractScalarField<int8_t>(structData, "ScramblerInit");
    sdrExtra.packetStartInternal = extractScalarField<int64_t>(structData, "PacketStartInternal");
    sdrExtra.hardwareRxSamplingIndex = extractScalarField<int64_t>(structData, "PreciseRxSampleIndex");
    sdrExtra.preciseRxTime = extractScalarField<double>(structData, "PreciseRxTime");
    sdrExtra.lastTxTime = extractScalarField<double>(structData, "LastTxTime");
    sdrExtra.signalInputSystemTime = extractScalarField<double>(structData, "SignalInputTime");
    sdrExtra.signalDecodeSystemTime = extractScalarField<double>(structData, "SignalDecodingTime");
    sdrExtra.sigEVM = extractScalarField<double>(structData, "SIGEVM");
    sdrExtra.initialCFO = extractScalarField<double>(structData, "InitialCFO");
    sdrExtra.residualCFO = extractScalarField<double>(structData, "ResidualCFO");

    segment->setSdrExtra(sdrExtra);
    return segment;
}

std::optional<PicoScenesFrameHeader> convertStruct2PicoScenesHeader(const matlab::data::StructArray& structData) {
    if(structData.getNumberOfFields() <= 0) {
        return std::nullopt;
    }

    PicoScenesFrameHeader header;
    try {
        header.magicValue = extractScalarField<uint32_t>(structData, "MagicValue");
        header.version = extractScalarField<uint32_t>(structData, "Version");
        header.deviceType = static_cast<PicoScenesDeviceType>(
            extractScalarField<uint16_t>(structData, "DeviceType")
        );
        header.frameType = extractScalarField<uint8_t>(structData, "FrameType");
        header.taskId = extractScalarField<uint16_t>(structData, "TaskId");
        header.txId = extractScalarField<uint16_t>(structData, "TxId");
    } catch (const std::exception& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
        return std::nullopt;
    }
    return header;
}

U8Vector convertStruct2Mpdu(const matlab::data::Array& mpduData) {
    U8Vector mpdu;

    try {
        if (mpduData.isEmpty()) {
            std::cout << "MPDU data is empty." << std::endl;
            return mpdu;
        }

        if (mpduData.getType() == matlab::data::ArrayType::CELL) {
            const matlab::data::CellArray& cellData = mpduData;

            if (cellData.isEmpty()) {
                std::cout << "MPDU CellArray is empty." << std::endl;
                return mpdu;
            }

            const matlab::data::Array& firstElement = cellData[0];

            if (firstElement.getType() == matlab::data::ArrayType::UINT8) {
                const matlab::data::TypedArray<uint8_t>& uint8Data = firstElement;
                mpdu.assign(uint8Data.begin(), uint8Data.end());
            } else {
                throw matlab::data::TypeMismatchException("Unsupported data type in MPDU CellArray.");
            }
        } else {
            std::cerr << "MPDU data is not of type CellArray." << std::endl;
        }
    } catch (const matlab::data::TypeMismatchException& e) {
        std::cerr << "Type mismatch error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return mpdu;
}

std::shared_ptr<AbstractPicoScenesFrameSegment> convertStruct2RxForeignSegments(const matlab::data::StructArray& structData) {
    std::string fieldName = "AntStateInfo";
    if (!hasField(structData, fieldName)) {
        return nullptr;
    }

    auto AntInfoField = structData[0][fieldName];
    AntStateInfo antStateInfo;
    antStateInfo.Angle = extractScalarField<double>(AntInfoField, "Angle");
    antStateInfo.AntId = extractScalarField<uint8_t>(AntInfoField, "AntId");
    AntStateInfoSegment segment(antStateInfo);
    return std::make_shared<AntStateInfoSegment>(segment);
}

std::shared_ptr<BasebandSignalSegment> convertStruct2BasebandSignal(const matlab::data::TypedArray<std::complex<double>>& structData) {
    if (structData.isEmpty()) {
        return nullptr;
    }

    auto segment = std::make_shared<BasebandSignalSegment>();
    try {
        auto dimensions = structData.getDimensions();
        if (dimensions.size() != 2) {
            throw std::invalid_argument("TypedArray dimensions must be 2-dimensional (rows and columns).");
        }

        size_t rows = dimensions[0];
        size_t cols = dimensions[1];

        std::vector<std::complex<float>> signalData(rows * cols);

        for (size_t col = 0; col < cols; ++col) {
            for (size_t row = 0; row < rows; ++row) {
                size_t index = row + col * rows;
                std::complex<double> complexValue = static_cast<std::complex<double>>(structData[index]);
                signalData[index] = std::complex<float>(
                    static_cast<float>(complexValue.real()),
                    static_cast<float>(complexValue.imag())
                );
            }
        }

        SignalMatrix<std::complex<float>> signalMatrix(
            std::move(signalData),
            std::array<size_t, 2>{rows, cols},
            SignalMatrixStorageMajority::ColumnMajor
        );

        segment->setSignals(std::move(signalMatrix));

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return nullptr;
    }
    return segment;
}

ModularPicoScenesRxFrame convertStructArray2Frame(const matlab::data::StructArray& structData, int index) {
    ModularPicoScenesRxFrame frame;
    frame.standardHeader = convertStruct2StandardHeader(structData[index]["StandardHeader"]);
    frame.rxSBasicSegment = convertStruct2RxSBasic(structData[index]["RxSBasic"]);
    frame.rxExtraInfoSegment = convertStruct2ExtraInfo(structData[index]["RxExtraInfo"]);
    frame.csiSegment = convertStruct2CSISegment(structData[index]["CSI"]);
    frame.PicoScenesHeader = convertStruct2PicoScenesHeader(structData[index]["PicoScenesHeader"]);
    frame.txExtraInfoSegment = convertStruct2ExtraInfo(structData[index]["TxExtraInfo"]);

    if (hasField(structData, "SDRExtra")) {
        frame.sdrExtraSegment = convertStruct2SDRExtra(structData[index]["SDRExtra"]);
    }
    if (hasField(structData, "LegacyCSI")) {
        frame.legacyCSISegment = convertStruct2CSISegment(structData[index]["LegacyCSI"]);
        frame.legacyCSISegment->segmentName = "LegacyCSI";
    }
    if (hasField(structData, "BasebandSignals")) {
        frame.basebandSignalSegment = convertStruct2BasebandSignal(structData[index]["BasebandSignals"]);
    }
    if (hasField(structData, "RxForeignSegments")) {
        auto rxForeignSegments = convertStruct2RxForeignSegments(structData[index]["RxForeignSegments"]);
        if(rxForeignSegments) {
           frame.rxUnknownSegments.insert({"AntStateInfo", rxForeignSegments});
        }
    }
    
    frame.mpdus.emplace_back(convertStruct2Mpdu(structData[index]["MPDU"]));
    return frame;
}

class MexFunction : public matlab::mex::Function {
public:
    void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) override {
        try {
            validateInputs(inputs);

            const auto cellArray = static_cast<matlab::data::CellArray>(inputs[0]);
            const auto pathArray = static_cast<matlab::data::StringArray>(inputs[1]);
            std::string filePath = std::string(pathArray[0]);

            writeToFile(filePath, cellArray);

            std::cout << "Write cellArray to File " << filePath << " Success!" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            throw;
        }
    }

private:
    void validateInputs(matlab::mex::ArgumentList& inputs) {
        if (inputs.size() != 2) {
            throw std::invalid_argument("Two inputs required: a cell array containing structs and a file path.");
        }

        if (inputs[0].getType() != matlab::data::ArrayType::CELL) {
            throw std::invalid_argument("First input must be a cell array.");
        }

        const auto& pathArray = inputs[1];
        if (pathArray.getType() != matlab::data::ArrayType::MATLAB_STRING) {
            throw std::invalid_argument("Second input must be a string representing the file path.");
        }

        if (static_cast<matlab::data::StringArray>(pathArray).getNumberOfElements() != 1) {
            throw std::invalid_argument("File path must be a single string.");
        }
    }

    void writeToFile(const std::string& filePath, const matlab::data::CellArray& cellArray) {
        std::ofstream outFile(filePath, std::ios::binary | std::ios::trunc);
        if (!outFile.is_open()) {
            throw std::ios_base::failure("Failed to open file for writing.");
        }

        for (size_t i = 0; i < cellArray.getNumberOfElements(); ++i) {
            if (cellArray[i].getType() != matlab::data::ArrayType::STRUCT) {
                throw std::invalid_argument("Cell element must be a struct.");
            }

            const auto structData = static_cast<matlab::data::StructArray>(cellArray[i]);
            ModularPicoScenesRxFrame frame = convertStructArray2Frame(structData, 0);
            std::vector<uint8_t> byteStream = frame.toBuffer();

            outFile.write(reinterpret_cast<const char*>(byteStream.data()), byteStream.size());
            if (!outFile) {
                throw std::ios_base::failure("Failed to write frame data to file.");
            }
        }

        outFile.close();
    }
};