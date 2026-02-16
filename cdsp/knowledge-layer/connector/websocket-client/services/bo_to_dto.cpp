#include "bo_to_dto.h"

#include <iostream>

#include "data_types.h"
#include "nlohmann/json.hpp"

/**
 * Converts a GetMessage business object (BO) into a vector of GetMessageDTOs.
 *
 * @param b_obj The GetMessage business object to be converted.
 * @return A vector of GetMessageDTOs representing the converted data.
 */
std::vector<GetMessageDTO> BoToDto::convert(const GetMessage& b_obj) {
    std::vector<GetMessageDTO> dtos;
    auto schema = schemaTypeToString(b_obj.getHeader().getSchemaType(), true);

    /**
     * Helper lambda function to create a GetMessageDTO from a given path.
     *
     * @param path An optional string representing the path of the node.
     */
    auto createDto = [&](const std::optional<std::string>& path) {
        GetMessageDTO dto;
        dto.schema = schema;
        dto.instance = b_obj.getHeader().getInstance();
        dto.path = path;
        dto.format = MessageStructureFormatToString(
            MessageStructureFormat::FLAT);  // Default format for GetMessageDTO
        dtos.push_back(dto);
    };

    if (b_obj.getNodes().empty()) {
        createDto(std::nullopt);
    } else {
        for (const auto& node : b_obj.getNodes()) {
            createDto(node.getName());
        }
    }

    return dtos;
}

/**
 * Converts a SubscribeMessage business object (BO) into a vector of
 * SubscribeMessageDTOs.
 *
 * @param b_obj The SubscribeMessage business object to be converted.
 * @return A vector of SubscribeMessageDTOs representing the converted data.
 */
std::vector<SubscribeMessageDTO> BoToDto::convert(const SubscribeMessage& b_obj) {
    std::vector<SubscribeMessageDTO> dtos;

    /**
     * Helper lambda function to create a SubscribeMessageDTO from a given path.
     *
     * @param path An optional string representing the path of the node.
     */
    auto createDto = [&](const std::optional<std::string>& path) {
        SubscribeMessageDTO dto;
        dto.schema = schemaTypeToString(b_obj.getHeader().getSchemaType(), true);
        dto.instance = b_obj.getHeader().getInstance();
        dto.path = path;
        dto.format = MessageStructureFormatToString(
            MessageStructureFormat::FLAT);  // Default format for SubscribeMessageDTO
        dto.root = "relative";              // Default root for SubscribeMessageDTO
        dtos.push_back(dto);
    };

    if (b_obj.getNodes().empty()) {
        createDto(std::nullopt);
    } else {
        for (const auto& node : b_obj.getNodes()) {
            createDto(node.getName());
        }
    }
    return dtos;
}

/**
 * Converts an UnsubscribeMessage business object (BO) into a vector of
 * UnsubscribeMessageDTOs.
 *
 * @param b_obj The UnsubscribeMessage business object to be converted.
 * @return A vector of UnsubscribeMessageDTOs representing the converted data.
 */
std::vector<UnsubscribeMessageDTO> BoToDto::convert(const UnsubscribeMessage& b_obj) {
    std::vector<UnsubscribeMessageDTO> dtos;
    /** Helper lambda function to create an UnsubscribeMessageDTO.
     * @param path An optional string representing the path of the node.
     */
    auto createDto = [&](const std::optional<std::string>& path) {
        UnsubscribeMessageDTO dto;
        dto.schema = schemaTypeToString(b_obj.getHeader().getSchemaType(), true);
        dto.instance = b_obj.getHeader().getInstance();
        dto.path = path;
        dtos.push_back(dto);
    };

    if (b_obj.getNodes().empty()) {
        createDto(std::nullopt);
    } else {
        for (const auto& node : b_obj.getNodes()) {
            createDto(node.getName());
        }
    }
    return dtos;
}

/**
 * Converts a SetMessage business object (BO) into a SetMessageDTO.
 *
 * @param b_obj The SetMessage business object to be converted.
 * @return A vector with a single SetMessageDTO representing the converted data.
 */
std::vector<SetMessageDTO> BoToDto::convert(const SetMessage& b_obj) {
    SetMessageDTO dto;
    dto.schema = schemaTypeToString(b_obj.getHeader().getSchemaType(), true);
    dto.instance = b_obj.getHeader().getInstance();

    for (const auto& node : b_obj.getNodes()) {
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

        // Populate the generated timestamp if it exists
        if (auto generated = node.getMetadata().getGenerated()) {
            auto [seconds, nanos] = Helper::getSecondsAndNanosecondsSinceEpoch(*generated);
            dto.metadata.nodes[node.getName()] =
                MetadataDTO::NodeMetadata{.generated = {seconds, nanos}};
        }

        // Populate the origin type if it exists
        if (auto origin = node.getMetadata().getOriginType()) {
            auto name = origin->name ? *origin->name : "";
            auto uri = origin->uri ? *origin->uri : "";

            dto.metadata.nodes[node.getName()].origin_type = {.name = name, .uri = uri};
        }

        dto.data.push_back(data);
    }

    // Returns a vector with a single SetMessageDTO object in order to match the
    // logic of the other convert functions
    return {dto};
}