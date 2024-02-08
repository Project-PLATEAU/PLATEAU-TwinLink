import { format } from "date-fns";
import { toDate } from "date-fns-tz";
import { IntervalUnit } from "../../domain/entity/intervalUnit/intervalUnit";
import {
  MinutesPopulation,
  HoursPopulation,
} from "../../domain/entity/population/population";
import { FindPopulation } from "../../domain/repositoryInterface/populationRepository.types";
import { hoursPopulationsCollection } from "./sampleData/hoursPopulationsCollection";
import { minutesPopulationsCollection } from "./sampleData/minutesPopulationsCollection";

const hoursPopulationData = hoursPopulationsCollection;
const minutesPopulationData = minutesPopulationsCollection;

export const find: FindPopulation = async ({ filter }) => {
  const { unit, timestamp, spatialIds } = filter;
  const result: MinutesPopulation[] | HoursPopulation[] = [];
  const store =
    unit === "minutes" ? minutesPopulationData : hoursPopulationData;

  if (spatialIds === undefined) {
    const targets = filterTimestamp(unit!, timestamp, store);
    result.push(...targets);
  } else {
    const filteredByIds = store.filter((population) =>
      spatialIds.includes(population.id),
    );
    const targets = filterTimestamp(unit, timestamp, filteredByIds);
    result.push(...targets);
  }
  return result;
};

const filterTimestamp = (
  unit: IntervalUnit,
  time: string,
  store: MinutesPopulation[] | HoursPopulation[],
) => {
  const timeFormat = unit === "minutes" ? "yyyyMMddHHmm" : "yyyyMMddHH";
  return store.filter(
    (population) =>
      format(toDate(population.timestamp), timeFormat) ===
      format(toDate(time), timeFormat),
  );
};
