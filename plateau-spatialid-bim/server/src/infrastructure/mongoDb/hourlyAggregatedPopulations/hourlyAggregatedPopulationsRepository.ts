import { Db, WithId } from "mongodb";
import {
  HoursPopulation,
  PopulationAggregation,
} from "../../../domain/entity/population/population";
import { FindPopulation } from "../../../domain/repositoryInterface/populationRepository.types";
import { Logger } from "../../../util/logger";
import { MongoDbCollections } from "../collections";

const Collection = MongoDbCollections.hourlyAggregatedPopulations;

export const insertOne = async (document: HoursPopulation, db: Db) => {
  const collection = db.collection(Collection);
  const result = await collection.insertOne(document);
  return result;
};

export const insertMany = async (documents: HoursPopulation[], db: Db) => {
  const collection = db.collection(Collection);
  const result = await collection.insertMany(documents);
  return result;
};

export const deleteManyByTimestamp = async (timestamp: string, db: Db) => {
  const collection = db.collection(Collection);
  const result = await collection.deleteMany({
    timestamp,
  });
  return result;
};

export const deleteAll = async (db: Db) => {
  const collection = db.collection(Collection);
  const result = await collection.deleteMany();
  return result;
};

export const findMany: FindPopulation = async ({ filter, db }) => {
  const collection = db.collection(Collection);
  type Query = Parameters<typeof collection.find>[0];
  const query: Query = {};
  query.id = { $in: filter.spatialIds };
  query.timestamp = filter.timestamp;
  Logger.info("findMany query", query);
  const docs = await collection
    .find<WithId<PopulationAggregation>>(query)
    .toArray();
  return docs.map((doc) => {
    const { _id, ...rest } = doc;
    return rest;
  });
};
