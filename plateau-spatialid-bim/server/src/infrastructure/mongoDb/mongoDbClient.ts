import { Db, MongoClient, ServerApiVersion } from "mongodb";
import { ServerEnv } from "../../config";

const Client = new MongoClient(ServerEnv.mongoConfig.uri, {
  serverApi: {
    version: ServerApiVersion.v1,
    strict: true,
    deprecationErrors: true,
  },
});

let MongoDb: Db | null = null;

const getClient = async () => {
  const client = await Client.connect();
  return client;
};

const getMongoDb = async (mongoClient?: MongoClient) => {
  if (MongoDb) {
    return MongoDb;
  }
  const client = mongoClient || (await getClient());
  MongoDb = client.db(ServerEnv.mongoConfig.dbName);
  return MongoDb;
};

export const withoutDbTransaction = async (
  callback: (DB: Db) => Promise<void>
) => {
  const dbType = ServerEnv.databaseType;
  const db = await getMongoDb();
  if (dbType === "mongo") {
    await callback(db);
  } else {
    callback(db!);
  }
};

export const withDbTransaction = async (
  callback: (DB: Db) => Promise<void>
) => {
  const dbType = ServerEnv.databaseType;
  let db: Db;

  if (dbType === "mongo") {
    const client = await getClient();
    db = await getMongoDb(client);
    const session = await client.startSession();
    session.startTransaction();
    try {
      await callback(db);
      await session.commitTransaction();
      await session.endSession();
    } catch (error) {
      await session.abortTransaction();
      await session.endSession();
      throw error;
    }
  } else {
    await callback(db!);
  }
};
