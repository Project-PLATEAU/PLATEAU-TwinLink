import { describe, expect, it, afterEach } from "vitest";
import { HoursPopulation } from "../../../domain/entity/population/population";
import { withoutDbTransaction } from "../mongoDbClient";
import {
  deleteAll,
  deleteManyByTimestamp,
  findMany,
  insertMany,
  insertOne,
} from "./hourlyAggregatedPopulationsRepository";

afterEach(async () => {
  await withoutDbTransaction(async (db) => {
    await deleteAll(db);
  });
});

describe("hourlyAggregatedPopulationsRepository", () => {
  it("insertとfind、deleteが一連でできること", async () => {
    const testData: HoursPopulation = {
      id: "/18/0/232791/103218",
      timestamp: "2023-10-12T04:00:00+09:00",
      type: "measured",
      peopleFlow: 100,
    };

    await withoutDbTransaction(async (db) => {
      const insertResult = await insertOne({ ...testData }, db);
      expect(insertResult.acknowledged).toBe(true);

      const findResult = await findMany({
        filter: {
          spatialIds: [testData.id],
          timestamp: testData.timestamp,
          unit: "hours",
        },
        db,
      });
      expect(findResult.length).toBe(1);
      expect(findResult[0]).toEqual(testData);

      const deleteResult = await deleteManyByTimestamp(testData.timestamp, db);
      expect(deleteResult.acknowledged).toBe(true);
      expect(deleteResult.deletedCount).toBe(1);
    });
  });

  it("timestampを指定してデータが取得できること", async () => {
    const testData: HoursPopulation[] = [
      {
        id: "/18/0/232791/103218",
        timestamp: "2023-10-12T04:00:00+09:00",
        type: "measured",
        peopleFlow: 100,
      },
      {
        id: "/18/0/232791/103218",
        timestamp: "2023-10-12T05:00:00+09:00",
        type: "measured",
        peopleFlow: 200,
      },
    ];

    const testDataCopy = JSON.parse(JSON.stringify(testData));

    await withoutDbTransaction(async (db) => {
      const insertResult = await insertMany(testDataCopy, db);
      expect(insertResult.acknowledged).toBe(true);

      const findResult1 = await findMany({
        filter: {
          spatialIds: [testDataCopy[0].id],
          timestamp: testDataCopy[0].timestamp,
          unit: "hours",
        },
        db,
      });
      expect(findResult1.length).toBe(1);
      expect(findResult1[0]).toEqual(testData[0]);

      const findResult2 = await findMany({
        filter: {
          spatialIds: [testDataCopy[1].id],
          timestamp: testDataCopy[1].timestamp,
          unit: "hours",
        },
        db,
      });
      expect(findResult2.length).toBe(1);
      expect(findResult2[0]).toEqual(testData[1]);
    });
  });

  it("複数のIDを指定してデータが取得できること", async () => {
    const testData: HoursPopulation[] = [
      {
        id: "/18/0/232791/103218",
        timestamp: "2023-10-12T04:00:00+09:00",
        type: "measured",
        peopleFlow: 100,
      },
      {
        id: "/18/0/232791/103218",
        timestamp: "2023-10-12T05:00:00+09:00",
        type: "measured",
        peopleFlow: 200,
      },
      {
        id: "/19/0/232791/10300",
        timestamp: "2023-10-12T04:00:00+09:00",
        type: "measured",
        peopleFlow: 300,
      },
      {
        id: "/19/0/232791/10300",
        timestamp: "2023-10-12T05:00:00+09:00",
        type: "measured",
        peopleFlow: 400,
      },
    ];
    const testDataCopy = JSON.parse(JSON.stringify(testData));

    await withoutDbTransaction(async (db) => {
      const insertResult = await insertMany(testDataCopy, db);
      expect(insertResult.acknowledged).toBe(true);

      const findResult1 = await findMany({
        filter: {
          spatialIds: [testDataCopy[0].id, testDataCopy[2].id],
          unit: "hours",
          timestamp: testDataCopy[0].timestamp,
        },
        db,
      });
      expect(findResult1.length).toBe(2);
      expect(findResult1[0]).toEqual(testData[0]);
      expect(findResult1[1]).toEqual(testData[2]);
    });
  });
});
