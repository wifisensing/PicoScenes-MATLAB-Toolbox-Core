//
// Created by huangfu on 23-12-1.
//

#include "AntStateInfoSegment.hxx"

std::vector<uint8_t> AntStateInfo::toBuffer() const{
    return std::vector<uint8_t>{(uint8_t *) this, (uint8_t *) this + sizeof(AntStateInfo)};
}

AntStateInfoSegment::AntStateInfoSegment() : AbstractPicoScenesFrameSegment("AntStateInfo", 0x01){
}

AntStateInfoSegment::AntStateInfoSegment(const AntStateInfo & antStateInfo) : AntStateInfoSegment() {
    setAntStateInfo(antStateInfo);
}

void AntStateInfoSegment::setAntStateInfo(const AntStateInfo &antStateInfo) {
    this->stateInfo = antStateInfo;
    setSegmentPayload(antStateInfo.toBuffer());
}

std::string AntStateInfoSegment::toString() const {
    std::string res = "rxAntId : " + std::to_string(stateInfo.AntId) + " | angle : " + std::to_string(stateInfo.Angle);
    return res;
}


