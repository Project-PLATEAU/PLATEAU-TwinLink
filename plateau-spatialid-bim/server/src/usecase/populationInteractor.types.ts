import { IntervalUnit } from "../domain/entity/intervalUnit/intervalUnit";
import { PopulationType } from "../domain/entity/population/populationType";

export type GetPopulation = ({
  spatialIds,
  time,
  unit = "hours",
}: GetPopulationParam) => GetPopulationResult;

type GetPopulationParam = {
  spatialIds?: string[];
  time?: string;
  unit?: string;
};

export type PopulationResponseModel = {
  id: string;
  type: PopulationType;
  values: [
    {
      timestamp: string;
      unit: IntervalUnit;
      peopleFlow: number;
    },
  ];
};

type GetPopulationResult = Promise<PopulationResponseModel[]>;
