export type ErrorMessage = {
  category: string;
  statusCode: number;
  message: string;
};

export const STATUS_ERRORS = {
  BAD_REQUEST: 400,
  UNAUTHORIZED: 401,
  FORBIDDEN: 403,
  NOT_FOUND: 404,
  INTERNAL_SERVER_ERROR: 500,
  NOT_IMPLEMENTED: 501,
  SERVICE_UNAVAILABLE: 503,
} as const;

export const STATUS_SUCCESS = {
  OK: 200
}
