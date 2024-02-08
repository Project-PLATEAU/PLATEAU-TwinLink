import { Bbox, isBboxValid } from "./domain/entity/bbox/bbox";

// Config will be loaded from envvars
type ServerConfig = {
  zoomLevels: number[];
  databaseType: DataBaseType;
  sftpConfig: SftpConfig;
  cameras: string;
  filePath: string;
  fileRegex: string;
  mongoConfig: MongoConfig;
  bbox?: Bbox;
};

type SftpConfig = {
  host: string;
  port: number;
  username: string;
  passphrase: string;
  privateKey: string;
};

type MongoConfig = {
  uri: string;
  dbName: string;
};

type DataBaseType = "mongo" | "mock" | "inMemory";

const bbbox = process.env.BBOX?.split(",").map((v) => Number(v.trim())) ?? [
  35.681794717155014, 35.69797641916519, 139.6831691582549, 139.70015371376525,
]; // bbox of Nishi-shinjuku;

export const ServerEnv: ServerConfig = {
  zoomLevels: process.env.ZOOM_LEVELS
    ? JSON.parse(process.env.ZOOM_LEVELS)
    : [15, 16, 17, 18, 19, 20, 21, 22, 23],
  databaseType: (process.env.DATABASE_TYPE as DataBaseType) ?? "mock",
  sftpConfig: {
    host: process.env.SFTP_HOST ?? "",
    port: Number(process.env.SFTP_PORT ?? ""),
    username: process.env.SFTP_USERNAME ?? "",
    passphrase: process.env.SFTP_PASSPHRASE ?? "",
    privateKey: process.env.SFTP_PRIVATE_KEY ?? "",
  },
  filePath: process.env.FILE_PATH ?? "/log",
  fileRegex: process.env.FILE_REGEX ?? "入退店分析.*csv",
  cameras: process.env.CAMERAS ?? "",
  mongoConfig: {
    uri: process.env.MONGO_URI ?? "mongodb://root:example@127.0.0.1:27017",
    dbName: process.env.MONGO_DB_NAME ?? "common",
  },
  bbox: isBboxValid(bbbox) ? bbbox : undefined,
};
