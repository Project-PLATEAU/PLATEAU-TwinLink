import { describe, expect, it } from "vitest";
import { ServerEnv } from "../../../config";
import { aggregatePopulation } from "../../entity/population/population";
import { SensorRawRecord } from "../../entity/sensorCamera/sensorRawRecord";
import { sensorRawRecordsToMomentPopulations } from "./sensorRawRecordsToMomentPopulation";

describe("sensorRawRecordsToMomentPopulation", () => {
  const sampleSensorRawRecords: SensorRawRecord[] = [
    {
      時刻: "2023/10/12 04:18:43",
      入退店: "退店",
      企業: "株式会社テスト",
      エリア: "西新宿",
      店舗: "西新宿003",
      カメラ: "西新宿003",
      トラッキングID: "0df51dea-65c5-420c-9deb-a8fc078b72c9",
    },
    {
      時刻: "2023/10/12 04:49:37",
      入退店: "入店",
      企業: "株式会社テスト",
      エリア: "西新宿",
      店舗: "西新宿003",
      カメラ: "西新宿003",
      トラッキングID: "225a6302-5342-434a-a71b-589d0f644a74",
    },
    {
      時刻: "2023/10/12 04:23:45",
      入退店: "退店",
      企業: "株式会社テスト",
      エリア: "西新宿",
      店舗: "西新宿003",
      カメラ: "西新宿003",
      トラッキングID: "242e4a23-95e3-4a16-8376-6d5f0504a3b4",
    },
    {
      時刻: "2023/10/12 04:10:57",
      入退店: "退店",
      企業: "株式会社テスト",
      エリア: "西新宿",
      店舗: "西新宿005",
      カメラ: "西新宿005",
      トラッキングID: "2c6798ec-cd60-4ed7-a7d9-3a5ec681f26b",
    },
    {
      時刻: "2023/10/12 04:23:07",
      入退店: "入店",
      企業: "株式会社テスト",
      エリア: "西新宿",
      店舗: "西新宿005",
      カメラ: "西新宿005",
      トラッキングID: "4e57dd6e-b93a-44fd-806f-8949cd973365",
    },
    {
      時刻: "2023/10/12 04:13:52",
      入退店: "退店",
      企業: "株式会社テスト",
      エリア: "西新宿",
      店舗: "西新宿005",
      カメラ: "西新宿005",
      トラッキングID: "515793ed-1e5c-4469-aff2-4a5aa3c0539a",
    },
    {
      時刻: "2023/10/12 04:37:06",
      入退店: "退店",
      企業: "株式会社テスト",
      エリア: "西新宿",
      店舗: "西新宿005",
      カメラ: "西新宿005",
      トラッキングID: "62498f1a-19ca-41eb-9c4b-2c0bc158faf0",
    },
  ];

  it("sensorRawRecordsをmomentPopulationに変換できること", () => {
    const momentPopulations = sensorRawRecordsToMomentPopulations(
      sampleSensorRawRecords,
    );

    const expectedLength =
      sampleSensorRawRecords.length * ServerEnv.zoomLevels.length;
    expect(momentPopulations.length).toBe(expectedLength);

    const aggregated = aggregatePopulation(momentPopulations);

    console.log(aggregated);

    const expectedKeys = JSON.stringify(["id", "type", "timestamp", "point"]);
    expect(
      momentPopulations.every(
        (p) => JSON.stringify(Object.keys(p)) === expectedKeys,
      ),
    ).toBe(true);
  });
});
