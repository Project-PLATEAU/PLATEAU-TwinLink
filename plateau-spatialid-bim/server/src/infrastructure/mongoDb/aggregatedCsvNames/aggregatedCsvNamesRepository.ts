import { Db, WithId } from "mongodb";
import { AggregatedCsvName } from "../../../domain/entity/aggregatedCsvName/aggregatedCsvName";
import { MongoDbCollections } from "../collections";

const Collection = MongoDbCollections.aggregatedCsvNames;

export const insertOne = async (
  aggregatedCsvName: AggregatedCsvName,
  db: Db,
) => {
  const collection = db.collection(Collection);
  const result = await collection.insertOne(aggregatedCsvName);
  return result;
};

export const deleteAll = async (db: Db) => {
  const collection = db.collection(Collection);
  const result = await collection.deleteMany();
  return result;
};

export const findManyByFileName = async (
  fileName: string,
  db: Db,
): Promise<AggregatedCsvName[]> => {
  const collection = db.collection(Collection);
  const docs = await collection
    .find<WithId<AggregatedCsvName>>({ fileName })
    .toArray();
  return docs.map((doc) => {
    const { _id, ...rest } = doc;
    return rest;
  });
};
