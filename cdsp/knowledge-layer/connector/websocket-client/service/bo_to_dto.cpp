#include "bo_to_dto.h"

/**
 * @brief Converts a GetMessage business object to a vector of GetMessageDto objects.
 *
 * This function takes a GetMessage object and an optional MessageStructureFormat,
 * converting them into a vector of GetMessageDto objects. Each node in the GetMessage
 * is converted into a GetMessageDto with the type set to "get", the schema derived
 * from the message's header, the instance set to the header's ID, and the path
 * set to the node's name. If no nodes are present, a single GetMessageDto is created
 * with the path set to nullopt. The format is set based on the provided optional
 * MessageStructureFormat.
 *
 * @param bo The GetMessage business object to be converted.
 * @param format An optional MessageStructureFormat to be applied to each GetMessageDto.
 * @return A vector of GetMessageDto objects representing the converted data.
 */
std::vector<GetMessageDto> BoToDto::convert(const GetMessage& bo,
                                            const std::optional<MessageStructureFormat>& format) {
    std::vector<GetMessageDto> dtos;
    auto schema = SchemaTypeToString(bo.getHeader().getSchemaType(), true);
    auto createDto = [&](const std::optional<std::string>& path) {
        GetMessageDto dto;
        dto.type = "get";
        dto.schema = schema;
        dto.instance = bo.getHeader().getId();
        dto.path = path;
        dto.format = format ? std::optional<std::string>(stringToMessageStructureFormat(*format))
                            : std::nullopt;
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
 * @brief Converts a SubscribeMessage business object to a vector of SubscribeMessageDto objects.
 *
 * This function takes a SubscribeMessage object and an optional MessageStructureFormat,
 * converting them into a vector of SubscribeMessageDto objects. Each node in the
 * SubscribeMessage is converted into a SubscribeMessageDto with the type set to
 * "subscribe", the schema derived from the message's header, the instance set
 * to the header's ID, and the path set to the node's name. If no nodes are present,
 * a single SubscribeMessageDto is created with the path set to nullopt. The format
 * is set based on the provided optional MessageStructureFormat.
 *
 * @param bo The SubscribeMessage business object to be converted.
 * @param format An optional MessageStructureFormat that specifies the format of the message
 * structure.
 * @return A vector of SubscribeMessageDto objects representing the converted business object.
 */
std::vector<SubscribeMessageDto> BoToDto::convert(
    const SubscribeMessage& bo, const std::optional<MessageStructureFormat>& format) {
    std::vector<SubscribeMessageDto> dtos;

    auto createDto = [&](const std::optional<std::string>& path) {
        SubscribeMessageDto dto;
        dto.type = "subscribe";
        dto.schema = SchemaTypeToString(bo.getHeader().getSchemaType(), true);
        dto.instance = bo.getHeader().getId();
        dto.path = path;
        dto.format = format ? std::optional<std::string>(stringToMessageStructureFormat(*format))
                            : std::nullopt;
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
