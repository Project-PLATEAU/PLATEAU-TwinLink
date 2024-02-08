import { AsyncLocalStorage } from "async_hooks";
import { RequestHandler } from "express";

import { pino, Logger as PinoLogger } from "pino";
import PinoPretty from "pino-pretty";
import { v4 as uuidv4 } from "uuid";

type StorageMap = Map<string, string>;
const asyncLocalStorage = new AsyncLocalStorage<StorageMap>();

type MetaItem = HttpMeta;

const CustomMeta = {
  RequestId: "requestId",
};

const HttpMeta = {
  RequestMethod: "requestMethod",
  RequestUrl: "requestUrl",
  RemoteIp: "remoteIp",
  Host: "host",
  UserAgent: "userAgent",
} as const;
type HttpMeta = (typeof HttpMeta)[keyof typeof HttpMeta];

const Severity = {
  INFO: "INFO",
  WARNING: "WARNING",
  ERROR: "ERROR",
} as const;

type Severity = (typeof Severity)[keyof typeof Severity];

type MetaLogData = {
  severity: Severity;
  httpRequest: {
    requestMethod: string;
    remoteIp: string;
    requestUrl: string;
    userAgent: string;
  };
  "logging.googleapis.com/labels": {
    requestId: string;
    host: string;
    error?: unknown;
  };
};

export class Logger {
  private static parentLogger: PinoLogger;

  private constructor() {
    // eslint-disable-next-line no-constructor-return, no-useless-return
    return;
  }

  static setMetaItem(key: MetaItem, value: string): void {
    const store = asyncLocalStorage.getStore();
    if (store) {
      store.set(key, value);
    }
  }

  static info(message: string, error?: unknown): void {
    const msgTitle = Logger.makeMsgTitle(message, error);
    Logger.getChildLogger(Severity.INFO, error).info(msgTitle);
  }

  static warn(message: string, error?: unknown): void {
    const msgTitle = Logger.makeMsgTitle(message, error);
    Logger.getChildLogger(Severity.WARNING, error).warn(msgTitle);
  }

  static error(message: string, error?: unknown): void {
    const msgTitle = Logger.makeMsgTitle(message, error);
    Logger.getChildLogger(Severity.ERROR, error).error(msgTitle);
  }

  private static makeMsgTitle(msg: string, error?: unknown): string {
    if (!error) return msg;
    const strErr =
      error instanceof Error
        ? error.stack || error.message
        : JSON.stringify(error);
    const idxOfNewLineChar = strErr.indexOf("\n");
    const topErrLine =
      idxOfNewLineChar === -1 ? strErr : strErr.substring(0, idxOfNewLineChar);
    return `${msg} - ${topErrLine}`;
  }

  private static getChildLogger(severity: Severity, error?: unknown) {
    Logger.setParentLogger();
    const err =
      error instanceof Error
        ? error.stack || error.message
        : JSON.stringify(error);

    const store = asyncLocalStorage.getStore();

    const requestId = store?.get(CustomMeta.RequestId) || "";
    const remoteIp = store?.get(HttpMeta.RemoteIp) || "";
    const requestMethod = store?.get(HttpMeta.RequestMethod) || "";
    const requestUrl = store?.get(HttpMeta.RequestUrl) || "";
    const userAgent = store?.get(HttpMeta.UserAgent) || "";
    const host = store?.get(HttpMeta.Host) || "";

    const metadata = (sev: Severity): MetaLogData => ({
      severity: sev,
      httpRequest: {
        remoteIp,
        requestMethod,
        requestUrl,
        userAgent,
      },
      "logging.googleapis.com/labels": {
        requestId,
        host,
        error: err,
      },
    });

    switch (severity) {
      case Severity.INFO:
        return Logger.parentLogger.child(metadata(Severity.INFO));
      case Severity.WARNING:
        return Logger.parentLogger.child(metadata(Severity.WARNING));
      case Severity.ERROR:
        return Logger.parentLogger.child(metadata(Severity.ERROR));
      default:
        return Logger.parentLogger;
    }
  }

  static initialize: RequestHandler = (req, res, next) => {
    Logger.setParentLogger();
    try {
      asyncLocalStorage.run(new Map(), () => {
        const store = asyncLocalStorage.getStore();

        if (store) {
          store.set(CustomMeta.RequestId, uuidv4());
          store.set(HttpMeta.RemoteIp, req.ip ?? "");
          store.set(HttpMeta.RequestMethod, req.method);
          store.set(HttpMeta.RequestUrl, req.originalUrl);
          store.set(HttpMeta.UserAgent, req.headers["user-agent"] ?? "");
          store.set(HttpMeta.Host, req.headers.host ?? "");
        }

        next();
      });
    } catch {
      next();
    }
  };

  private static setParentLogger(): void {
    if (!Logger.parentLogger) {
      const messageKey = "message";
      const option = {
        messageKey,
        base: undefined,
      };
      const prettier = PinoPretty({
        messageKey,
      });

      Logger.parentLogger =
        process.env.NODE_ENV === "development"
          ? pino(option, prettier)
          : pino(option);
    }
  }
}
