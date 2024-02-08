import { Db } from "mongodb";
import { IntervalUnit } from "../entity/intervalUnit/intervalUnit";
import {
  MinutesPopulation,
  HoursPopulation,
} from "../entity/population/population";

export type FindPopulation = (
  params: FindPopulationParams,
) => FindPopulationResult;

type FindPopulationParams = {
  filter: {
    spatialIds?: string[];
    timestamp: string;
    unit: IntervalUnit;
  };
  db: Db;
};

type FindPopulationResult = Promise<MinutesPopulation[] | HoursPopulation[]>;
