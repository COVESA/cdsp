import {
  DataResponseDTO,
  ErrorResponseDTO,
  GetMessageDTO,
  NewMessageDTO,
  OkResponseDTO,
  SetMessageDTO,
  SubscribeMessageDTO,
  UnsubscribeMessageDTO,
} from "./MessageDTO";
import {
  GetMessageType,
  NewMessageType,
  NewMessage,
  SetMessageType,
  SubscribeMessageType,
  UnsubscribeMessageType,
  DataContentMessage,
  StatusMessage,
  ErrorMessage,
} from "./NewMessage";
import { replaceDotsWithUnderscore } from "../../handlers/utils/transformations";

export class MessageMapper {
  toDomain(dto: NewMessageDTO): NewMessage {
    this.modifyRequest(dto);
    switch (dto.method) {
      case NewMessageType.Get:
        return this.toGetBO(dto as GetMessageDTO);
      case NewMessageType.Set:
        return this.toSetBO(dto as SetMessageDTO);
      case NewMessageType.Subscribe:
        return this.toSubscribeBO(dto as SubscribeMessageDTO);
      case NewMessageType.Unsubscribe:
        return this.toUnsubscribeBO(dto as UnsubscribeMessageDTO);
      default:
        throw new Error(`Unknown method received in ${dto}`);
    }
  }

  toDTO(
    bo: StatusMessage | DataContentMessage | ErrorMessage,
  ): DataResponseDTO | OkResponseDTO | ErrorResponseDTO {
    switch (bo.type) {
      case "status":
        return this.toOkResponseDTO(bo);

      case "data":
        return this.toDataResponseDTO(bo);

      case "error":
        return this.toErrorResponseDTO(bo);

      default:
        throw new Error(`Unknown message type to convert to DTO ${bo}`);
    }
  }

  private toGetBO(dto: GetMessageDTO): GetMessageType {
    return {
      type: dto.method,
      path: dto.params.path,
      instance: dto.params.instance,
      requestId: dto.id,
      root: dto.params.root ?? "relative",
      format: dto.params.format ?? "nested",
    };
  }

  private toSetBO(dto: SetMessageDTO): SetMessageType {
    return {
      type: dto.method,
      path: dto.params.path,
      instance: dto.params.instance,
      data: dto.params.data,
      metadata: dto.params.metadata,
      timeseries: dto.params.timeseries,
      requestId: dto.id,
    };
  }

  private toSubscribeBO(dto: SubscribeMessageDTO): SubscribeMessageType {
    return {
      type: dto.method,
      path: dto.params.path,
      instance: dto.params.instance,
      requestId: dto.id,
      root: dto.params.root ?? "absolute",
      format: dto.params.format ?? "nested",
    };
  }

  private toUnsubscribeBO(dto: UnsubscribeMessageDTO): UnsubscribeMessageType {
    return {
      type: dto.method,
      path: dto.params.path,
      instance: dto.params.instance,
      requestId: dto.id,
    };
  }

  private toDataResponseDTO(bo: DataContentMessage): DataResponseDTO {
    return {
      jsonrpc: "2.0",
      id: bo.requestId ?? null,
      result: {
        instance: bo.instance,
        data: bo.data,
        metadata: bo.metadata,
      },
    };
  }

  private toOkResponseDTO(bo: StatusMessage): OkResponseDTO {
    return {
      jsonrpc: "2.0",
      id: bo.requestId,
      result: {},
    };
  }

  private toErrorResponseDTO(bo: ErrorMessage): ErrorResponseDTO {
    return {
      jsonrpc: "2.0",
      id: bo.requestId ?? null,
      error: {
        code: bo.code,
        message: bo.message,
        data: {
          reason: bo.reason,
        },
      },
    };
  }

  /**
   * Apply modifications to the request structure and transformations to request values.
   */
  private modifyRequest(request: NewMessageDTO) {
    this.moveSchemaToPathField(request.params);
    request.params.path = replaceDotsWithUnderscore(request.params.path);
  }

  /**
   * Merges `schema` into `path` and removes `schema`.
   */
  private moveSchemaToPathField(params: any) {
    if (params.schema && params.path) {
      params.path = `${params.schema}_${params.path}`;
    } else if (params.schema && !params.path) {
      params.path = params.schema;
    }
    delete params.schema;
  }
}
