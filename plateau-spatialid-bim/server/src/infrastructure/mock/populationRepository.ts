import {
  MinutesPopulation,
  HoursPopulation,
} from "../../domain/entity/population/population";
import { FindPopulation } from "../../domain/repositoryInterface/populationRepository.types";

export const find: FindPopulation = async ({ filter }) => {
  const { spatialIds, timestamp } = filter;
  if (spatialIds === undefined) {
    return [];
  }
  return generateMockPopulation(spatialIds, timestamp);
};

function templateElement(
  parsedTime: string,
): MinutesPopulation | HoursPopulation {
  return {
    id: "/15/0/2222/2222",
    type: "measured",
    timestamp: parsedTime,
    peopleFlow: 100,
  };
}

function generateMockPopulation(spatialIds: string[], parsedTime: string) {
  return spatialIds.map((id) => {
    const element = templateElement(parsedTime);
    element.id = id;

    const splittedId = id.split("/").slice(1);
    // 仮の人流値(0~999の適当な値)
    const basePeopleFlow =
      splittedId.reduce((acc, current) => Number(current) * 41 + acc * 37, 0) %
      1000;

    const date = new Date(parsedTime);
    const hours = date.getHours();
    const months = date.getMonth();
    const days = date.getDay();

    const scale = randoms[(hours + months + days) % 10] ?? 1;
    // 空間ボクセルのサイズによってスケールさせて格納
    element.peopleFlow = Math.floor(
      (basePeopleFlow / 4 ** (Number(splittedId[0]) - 21)) * scale,
    );

    return element;
  });
}

const randoms: Record<number, number> = {
  1: 0.6,
  2: 3,
  3: 1,
  4: 2,
  5: 3,
  6: 0.3,
  7: 2,
  8: 0.4,
  9: 4,
  10: 0.2,
  11: 3,
  12: 4,
  13: 2,
  14: 0.7,
  15: 0.6,
  16: 1,
  17: 0.7,
  18: 0.8,
  19: 1,
  20: 0.9,
  21: 1,
  22: 0.4,
  23: 0.8,
  24: 1,
};
