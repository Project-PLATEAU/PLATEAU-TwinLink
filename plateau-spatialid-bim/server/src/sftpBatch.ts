import { ServerEnv } from "./config";
import { aggregateLatestPopulation } from "./usecase/aggregateLatestPopulationInteractor";
import { Logger } from "./util/logger";

const populationSFTPBatch = async () => {
  Logger.info("start aggregate population with sftp");
  const { bbox } = ServerEnv;
  if (!bbox) throw new Error("bbox is not defined");
  const { zoomLevels } = ServerEnv;
  await aggregateLatestPopulation(bbox, zoomLevels);
};

populationSFTPBatch()
  .then(() => {
    Logger.info("finish aggregate population");
    process.exit(0);
  })
  .catch((e) => {
    Logger.error("error aggregate population", e);
    process.exit(1);
  });
