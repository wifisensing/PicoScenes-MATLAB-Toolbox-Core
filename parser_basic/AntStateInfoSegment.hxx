//
// Created by huangfu on 23-12-1.
//

#ifndef PICOSCENES_THREE_PLUGINS_ANTSTATEINFO_H
#define PICOSCENES_THREE_PLUGINS_ANTSTATEINFO_H

#include <vector>
#include "AbstractPicoScenesFrameSegment.hxx"

struct AntStateInfo {
    double Angle{0};
    uint8_t AntId{0};

    [[nodiscard]] std::vector<uint8_t> toBuffer() const;

}__attribute__ ((__packed__));


class AntStateInfoSegment final : public AbstractPicoScenesFrameSegment{
public:
    AntStateInfoSegment();

    explicit AntStateInfoSegment(const AntStateInfo&);

    void setAntStateInfo(const AntStateInfo &antStateInfo);

    [[nodiscard]] std::string toString() const override;

private:
    AntStateInfo stateInfo{};
};


#endif //PICOSCENES_THREE_PLUGINS_ANTSTATEINFO_H
