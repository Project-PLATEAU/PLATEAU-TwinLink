import { ServerEnv } from "../../../config";
import { Logger } from "../../../util/logger";
import { MomentPopulation } from "../../entity/population/population";
import { Cameras, getSpatialId } from "../../entity/sensorCamera/sensorCamera";
import { SensorRawRecord } from "../../entity/sensorCamera/sensorRawRecord";
import { parseTimeToHourlyJst } from "../time/parseTime";

export const sensorRawRecordsToMomentPopulations = (
  rawRecords: SensorRawRecord[],
): MomentPopulation[] => {
  const { zoomLevels } = ServerEnv;

  return rawRecords.flatMap<MomentPopulation>((rawRecord) => {
    const camera = Cameras.find((c) => c.name === rawRecord["カメラ"]);
    if (!camera) {
      Logger.error(`Camera not found: ${JSON.stringify(rawRecord)}`);
      return [];
    }
    return zoomLevels.flatMap<MomentPopulation>((zoomLevel) => {
      const id = getSpatialId(camera, zoomLevel);
      return {
        id,
        type: "measured",
        timestamp: parseTimeToHourlyJst(rawRecord["時刻"]),
        point: {
          type: "Point",
          coordinates: [camera.lng, camera.lat],
        },
      };
    });
  });
};
