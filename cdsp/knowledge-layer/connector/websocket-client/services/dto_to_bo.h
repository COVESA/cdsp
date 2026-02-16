#ifndef DTO_TO_BO_H
#define DTO_TO_BO_H

#include "data_message_converter.h"
#include "data_message_dto.h"
#include "i_file_handler.h"
#include "model_config.h"
#include "model_config_converter.h"
#include "model_config_dto.h"
#include "request_registry.h"
#include "status_message.h"
#include "status_message_converter.h"
#include "status_message_dto.h"

class DtoToBo {
   public:
    explicit DtoToBo(std::shared_ptr<IFileHandler> file_handler = nullptr)
        : file_handler_(std::move(file_handler)) {}

    static DataMessage convert(const DataMessageDTO &dto, RequestRegistry &registry) {
        return DataMessageConverter::convert(dto, registry);
    }
    static StatusMessage convert(const StatusMessageDTO &dto, RequestRegistry &registry) {
        return StatusMessageConverter::convert(dto, registry);
    }
    ModelConfig convert(const ModelConfigDTO &dto) {
        if (file_handler_ == nullptr) {
            throw std::runtime_error("File handler is not initialized");
        }
        ModelConfigConverter model_file_converter(file_handler_);
        return model_file_converter.convert(dto);
    }

   private:
    std::shared_ptr<IFileHandler> file_handler_;
};

#endif  // DTO_TO_BO_H