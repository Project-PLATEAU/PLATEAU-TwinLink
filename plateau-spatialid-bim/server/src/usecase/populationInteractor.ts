import { container } from "tsyringe";
import { PopulationAggregation } from "../domain/entity/population/population";
import { FindPopulation } from "../domain/repositoryInterface/populationRepository.types";
import { parseUnit } from "../domain/service/intervalUnit/parseUnit";
import { parseTimeToHourlyJst } from "../domain/service/time/parseTime";
import { withoutDbTransaction } from "../infrastructure/mongoDb/mongoDbClient";
import { DIKeys } from "../util/dependencyInjection";
import {
  GetPopulation,
  PopulationResponseModel,
} from "./populationInteractor.types";

export const getPopulation: GetPopulation = async ({
  spatialIds,
  unit = "hours",
  time,
}) => {
  const parsedUnit = parseUnit(unit);
  const parsedTime = parseTimeToHourlyJst(time);

  const findPopulation = await container.resolve<FindPopulation>(
    DIKeys.findPopulation,
  );

  if (!spatialIds) return [];

  let populationAggregation: PopulationAggregation[] = [];
  await withoutDbTransaction(async (db) => {
    populationAggregation = await findPopulation({
      filter: {
        spatialIds,
        timestamp: parsedTime,
        unit: parsedUnit,
      },
      db,
    });
  });

  return populationAggregation.map<PopulationResponseModel>((population) => ({
    id: population.id,
    type: population.type,
    values: [
      {
        timestamp: population.timestamp,
        unit: parsedUnit,
        peopleFlow: population.peopleFlow,
      },
    ],
  }));
};
