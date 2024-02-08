import Papa from "papaparse";
import { SensorRawRecord } from "../../entity/sensorCamera/sensorRawRecord";

export const ParseSensorRawCsv = (csv: string): SensorRawRecord[] =>
  Papa.parse<SensorRawRecord>(csv, {
    header: true,
    skipEmptyLines: "greedy",
    newline: "\n",
  }).data;
