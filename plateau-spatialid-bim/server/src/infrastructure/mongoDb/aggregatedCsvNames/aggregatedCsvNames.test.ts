import { describe, expect, it, afterEach } from "vitest";
import { AggregatedCsvName } from "../../../domain/entity/aggregatedCsvName/aggregatedCsvName";
import { withoutDbTransaction } from "../mongoDbClient";
import * as repository from "./aggregatedCsvNamesRepository";

afterEach(async () => {
  withoutDbTransaction(async (db) => {
    await repository.deleteAll(db);
  });
});

describe("aggregatedCsvNameRepository", () => {
  it("insertとfindが一連でできること", async () => {
    withoutDbTransaction(async (db) => {
      const testData: AggregatedCsvName = {
        fileName: "sample.csv",
      };

      const insertResult = await repository.insertOne({ ...testData }, db);
      expect(insertResult.acknowledged).toBe(true);

      const findResult = await repository.findManyByFileName(
        testData.fileName,
        db,
      );
      expect(findResult.length).toBe(1);
      expect(findResult[0]).toEqual(testData);
    });
  });
});
