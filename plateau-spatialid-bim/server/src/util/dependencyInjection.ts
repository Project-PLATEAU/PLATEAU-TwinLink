import { RequestHandler } from "express";
import { container } from "tsyringe";
import { ServerEnv } from "../config";
import * as inMemoryPopulationRepository from "../infrastructure/inMemory/populationRepository";
import * as mockPopulationRepository from "../infrastructure/mock/populationRepository";
import * as mongoPopulationRepository from "../infrastructure/mongoDb/hourlyAggregatedPopulations/hourlyAggregatedPopulationsRepository";

export const DIKeys = {
  findPopulation: "findPopulation",
} as const;

export const dependencyInjectHandler: RequestHandler = (_, __, next) => {
  dependencyInjection();
  next();
};

export const dependencyInjection = (): void => {
  const { databaseType } = ServerEnv;

  if (databaseType === "mock") {
    container.register(DIKeys.findPopulation, {
      useValue: mockPopulationRepository.find,
    });
  }
  if (databaseType === "inMemory") {
    container.register(DIKeys.findPopulation, {
      useValue: inMemoryPopulationRepository.find,
    });
  }
  if (databaseType === "mongo") {
    container.register(DIKeys.findPopulation, {
      useValue: mongoPopulationRepository.findMany,
    });
  }
};
