import { ServerEnv } from "../config";
import { Bbox, bboxMomentData } from "../domain/entity/bbox/bbox";
import { aggregatePopulation } from "../domain/entity/population/population";
import { sensorRawRecordsToMomentPopulations } from "../domain/service/population/sensorRawRecordsToMomentPopulation";
import { ParseSensorRawCsv } from "../domain/service/sensorCamera/parseCsv";
import * as aggregatedCsvNameRepository from "../infrastructure/mongoDb/aggregatedCsvNames/aggregatedCsvNamesRepository";
import * as hourlyAggregatedPopulationsRepository from "../infrastructure/mongoDb/hourlyAggregatedPopulations/hourlyAggregatedPopulationsRepository";
import {
  withoutDbTransaction,
  withDbTransaction,
} from "../infrastructure/mongoDb/mongoDbClient";
import { getFile, listFileName } from "../infrastructure/sftp/sftpRepository";
import { Logger } from "../util/logger";

export const aggregateLatestPopulation = async (
  bbox: Bbox,
  zoomLevels: number[],
) => {
  const fileNames = await listFileName(ServerEnv.filePath);
  const regex = new RegExp(ServerEnv.fileRegex);
  const possiblyTargetFileNames = fileNames.filter((fileName) =>
    regex.test(fileName),
  );

  const latestFileName = possiblyTargetFileNames.reduce((a, b) => {
    const aDate = Number(a.match(/\d+/)?.[0]);
    const bDate = Number(b.match(/\d+/)?.[0]);
    return aDate > bDate ? a : b;
  });

  Logger.info(`latestFileName: ${latestFileName}`);

  let isFileAlreadyExecuted = false;
  await withoutDbTransaction(async (db) => {
    const executedCsvByLatestFileName =
      await aggregatedCsvNameRepository.findManyByFileName(latestFileName, db);
    if (executedCsvByLatestFileName.length > 0) {
      isFileAlreadyExecuted = true;
    }
  });
  if (isFileAlreadyExecuted) {
    Logger.info(`csv ${latestFileName} already executed`);
    return;
  }

  const csv = await getFile(`/log/${latestFileName}`);
  const sensorRawRecords = ParseSensorRawCsv(csv);

  const MomentPopulations =
    sensorRawRecordsToMomentPopulations(sensorRawRecords);
  const bboxData = bboxMomentData(bbox, zoomLevels); // BBox of target city
  const hourlyAggregatedPopulations = aggregatePopulation([
    ...MomentPopulations,
    ...bboxData,
  ]);
  if (!hourlyAggregatedPopulations) {
    throw new Error(`failed to aggregate ${latestFileName}`);
  }

  await withDbTransaction(async (db) => {
    const populationInsertResult =
      await hourlyAggregatedPopulationsRepository.insertMany(
        hourlyAggregatedPopulations,
        db,
      );
    if (!populationInsertResult.acknowledged) {
      throw new Error(`failed to insert ${latestFileName}`);
    }

    const fileNameInsertResult = await aggregatedCsvNameRepository.insertOne(
      {
        fileName: latestFileName,
      },
      db,
    );
    if (!fileNameInsertResult.acknowledged) {
      throw new Error(`failed to insert ${latestFileName}`);
    }
    Logger.info(`inserted documents from ${latestFileName}`);
  });
};
