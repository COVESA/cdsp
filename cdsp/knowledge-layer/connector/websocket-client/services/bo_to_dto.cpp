#include "bo_to_dto.h"

#include <iostream>

#include "data_types.h"
#include "nlohmann/json.hpp"

/**
 * Converts a GetMessage business object (BO) into a vector of GetMessageDTOs.
 *
 * @param bo The GetMessage business object to be converted.
 * @return A vector of GetMessageDTOs representing the converted data.
 */
std::vector<GetMessageDTO> BoToDto::convert(const GetMessage& bo) {
    std::vector<GetMessageDTO> dtos;
    auto schema = schemaTypeToString(bo.getHeader().getSchemaType(), true);

    /**
     * Helper lambda function to create a GetMessageDTO from a given path.
     *
     * @param path An optional string representing the path of the node.
     */
    auto createDto = [&](const std::optional<std::string>& path) {
        GetMessageDTO dto;
        dto.type = "get";
        dto.schema = schema;
        dto.instance = bo.getHeader().getId();
        dto.path = path;
        dto.format = MessageStructureFormatToString(
            MessageStructureFormat::FLAT);  // Default format for GetMessageDTO
        dtos.push_back(dto);
    };

    if (bo.getNodes().empty()) {
        createDto(std::nullopt);
    } else {
        for (const auto& node : bo.getNodes()) {
            std::string node_name = node.getName().substr(schema.size() + 1);
            createDto(node_name);
        }
    }

    return dtos;
}

/**
 * Converts a SubscribeMessage business object (BO) into a vector of SubscribeMessageDTOs.
 *
 * @param bo The SubscribeMessage business object to be converted.
 * @return A vector of SubscribeMessageDTOs representing the converted data.
 */
std::vector<SubscribeMessageDTO> BoToDto::convert(const SubscribeMessage& bo) {
    std::vector<SubscribeMessageDTO> dtos;

    /**
     * Helper lambda function to create a SubscribeMessageDTO from a given path.
     *
     * @param path An optional string representing the path of the node.
     */
    auto createDto = [&](const std::optional<std::string>& path) {
        SubscribeMessageDTO dto;
        dto.type = "subscribe";
        dto.schema = schemaTypeToString(bo.getHeader().getSchemaType(), true);
        dto.instance = bo.getHeader().getId();
        dto.path = path;
        dto.format = MessageStructureFormatToString(
            MessageStructureFormat::FLAT);  // Default format for SubscribeMessageDTO
        dtos.push_back(dto);
    };

    if (bo.getNodes().empty()) {
        createDto(std::nullopt);
    } else {
        for (const auto& node : bo.getNodes()) {
            createDto(node.getName());
        }
    }
    return dtos;
}

/**
 * Converts a SetMessage business object (BO) into a SetMessageDTO.
 *
 * @param bo The SetMessage business object to be converted.
 * @return A SetMessageDTO representing the converted data.
 */
SetMessageDTO BoToDto::convert(const SetMessage& bo) {
    SetMessageDTO dto;

    dto.schema = schemaTypeToString(bo.getHeader().getSchemaType(), true);
    dto.instance = bo.getHeader().getId();
    dto.requestId = std::nullopt;
    dto.schema = schemaTypeToString(bo.getHeader().getSchemaType(), true);

    for (const auto& node : bo.getNodes()) {
        DataDTO data;
        data.name = node.getName();
        if (node.getValue()) {
            try {
                data.value = nlohmann::json::parse(*node.getValue());
            } catch (const std::exception& e) {
                std::cerr << "JSON parsing failed for value: " << *node.getValue() << " - "
                          << e.what() << std::endl;
                data.value = *node.getValue();  // fallback
            }
        } else {
            data.value = "";
        }
        auto [seconds, nanos] =
            Helper::getSecondsAndNanosecondsSinceEpoch(node.getMetadata().getGenerated().value());
        dto.metadata.nodes[node.getName()] =
            MetadataDTO::NodeMetadata{.generated = {seconds, nanos}};
        dto.data.push_back(data);
    }

    return dto;
}