import fs from "fs";
import { Request, Response } from "express";
import { test, vi, expect, describe } from "vitest";
import { hoursPopulationsCollection } from "../../src/infrastructure/inMemory/sampleData/hoursPopulationsCollection";
import { minutesPopulationsCollection } from "../../src/infrastructure/inMemory/sampleData/minutesPopulationsCollection";
import { get } from "../../src/presentation/populationController";
import { PopulationResponseModel } from "../../src/usecase/populationInteractor.types";
import { dependencyInjection } from "../../src/util/dependencyInjection";

describe.skip("人流データの取得テスト", () => {
  describe("インメモリでサンプルテスト", () => {
    test("1のの空間IDを指定して取得できること", async () => {
      dependencyInjection();
      const req = {} as Request;
      const res = {} as Response;
      const next = vi.fn();

      const expectedData: PopulationResponseModel = {
        id: hoursPopulationsCollection[0].id,
        type: hoursPopulationsCollection[0].type,
        values: {
          timestamp: hoursPopulationsCollection[0].timestamp,
          unit: "hours",
          peopleFlow: hoursPopulationsCollection[0].peopleFlow,
        },
      };

      req.query = {
        spatialId: "/15/0/1111/1111",
        time: "2023-09-14T15:00:00+09:00",
      };
      res.status = vi.fn().mockReturnThis();
      res.json = vi.fn().mockReturnThis();

      await get(req, res, next);

      expect(res.json).toHaveBeenCalledWith([expectedData]);
      expect(res.status).toHaveBeenCalledWith(200);
    });

    test("2つの空間IDを指定して取得できること", async () => {
      dependencyInjection();
      const req = {} as Request;
      const res = {} as Response;
      const next = vi.fn();

      const expectedData: PopulationResponseModel[] = [
        {
          id: hoursPopulationsCollection[0].id,
          type: hoursPopulationsCollection[0].type,
          values: {
            timestamp: hoursPopulationsCollection[0].timestamp,
            unit: "hours",
            peopleFlow: hoursPopulationsCollection[0].peopleFlow,
          },
        },
        {
          id: hoursPopulationsCollection[1].id,
          type: hoursPopulationsCollection[1].type,
          values: {
            timestamp: hoursPopulationsCollection[1].timestamp,
            unit: "hours",
            peopleFlow: hoursPopulationsCollection[1].peopleFlow,
          },
        },
      ];
      req.query = {
        spatialId: "/15/0/1111/1111,/15/0/2222/2222",
        time: "2023-09-14T15:00:00+09:00",
      };
      res.status = vi.fn().mockReturnThis();
      res.json = vi.fn().mockReturnThis();

      fs.writeFileSync(
        "minutes.csv",
        JSON.stringify(minutesPopulationsCollection),
      );

      await get(req, res, next);

      expect(res.json).toHaveBeenCalledWith(expectedData);
      expect(res.status).toHaveBeenCalledWith(200);
    });

    test("minutesを指定できること", async () => {
      dependencyInjection();
      const req = {} as Request;
      const res = {} as Response;
      const next = vi.fn();

      const expectedData: PopulationResponseModel = {
        id: minutesPopulationsCollection[1].id,
        type: minutesPopulationsCollection[1].type,
        values: {
          timestamp: minutesPopulationsCollection[1].timestamp,
          unit: "minutes",
          peopleFlow: minutesPopulationsCollection[1].peopleFlow,
        },
      };
      req.query = {
        time: "2023-09-14T15:30:10+09:00",
        unit: "minutes",
      };
      res.status = vi.fn().mockReturnThis();
      res.json = vi.fn().mockReturnThis();

      await get(req, res, next);

      expect(res.json).toHaveBeenCalledWith([expectedData]);
      expect(res.status).toHaveBeenCalledWith(200);
    });

    test("JST以外のTZでも取得できること", async () => {
      dependencyInjection();
      const req = {} as Request;
      const res = {} as Response;
      const next = vi.fn();

      const expectedData: PopulationResponseModel = {
        id: hoursPopulationsCollection[0].id,
        type: hoursPopulationsCollection[0].type,
        values: {
          timestamp: hoursPopulationsCollection[0].timestamp,
          unit: "hours",
          peopleFlow: hoursPopulationsCollection[0].peopleFlow,
        },
      };

      req.query = {
        spatialId: "/15/0/1111/1111",
        time: "2023-09-14T06:30:00+00:00",
      };
      res.status = vi.fn().mockReturnThis();
      res.json = vi.fn().mockReturnThis();

      await get(req, res, next);

      expect(res.json).toHaveBeenCalledWith([expectedData]);
      expect(res.status).toHaveBeenCalledWith(200);
    });
  });
});
