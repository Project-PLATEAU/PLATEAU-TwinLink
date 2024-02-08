export type MongoDbCollections =
  (typeof MongoDbCollections)[keyof typeof MongoDbCollections];

export const MongoDbCollections = {
  hourlyAggregatedPopulations: "hourlyAggregatedPopulations",
  aggregatedCsvNames: "aggregatedCsvNames",
} as const;
