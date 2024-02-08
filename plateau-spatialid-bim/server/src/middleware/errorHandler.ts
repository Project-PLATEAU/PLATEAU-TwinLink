import { ErrorRequestHandler } from "express";
import HttpStatusCode from "../util/httpStatusCode";
import { Logger } from "../util/logger";

export class CustomError extends Error {
  statusCode: number;

  constructor(message: string, statusCode: number) {
    super(message);
    this.statusCode = statusCode;
  }
}

export const errorHandler: ErrorRequestHandler = (
  err: CustomError,
  _,
  res,
  next,
) => {
  if (res.headersSent) {
    return next(err);
  }
  Logger.error("Middleware Error: ", err);
  const errStatus = err.statusCode || HttpStatusCode.INTERNAL_SERVER_ERROR;
  const errMsg = err.message || "Something went wrong";
  return res.status(errStatus).json({
    success: false,
    status: errStatus,
    message: errMsg,
    stack: process.env.NODE_ENV === "development" ? err.stack : undefined,
  });
};
