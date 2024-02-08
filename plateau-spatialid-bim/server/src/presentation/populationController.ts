import "reflect-metadata";
import { RequestHandler } from "express";
import PromiseRouter from "express-promise-router";
import { getPopulation } from "../usecase/populationInteractor";
import HttpStatusCode from "../util/httpStatusCode";

const router = PromiseRouter();

export const get: RequestHandler = async (req, res) => {
  const spatialIds =
    req.query.spatialId === undefined
      ? undefined
      : String(req.query.spatialId).replace(/"/g, "").split(",");
  const time =
    req.query.time === undefined
      ? undefined
      : String(req.query.time).replace(/"/g, "").replace(" 09:00", "+09:00");
  const unit =
    req.query.unit === undefined ? undefined : String(req.query.unit);

  const population = await getPopulation({ spatialIds, time, unit });

  res.status(HttpStatusCode.OK).json({ data: population });
};

export const post: RequestHandler = async (req, res) => {
  const spatialIds =
    req.body.spatialId === undefined
      ? undefined
      : String(req.body.spatialId).replace(/"/g, "").split(",");
  const time =
    req.body.time === undefined
      ? undefined
      : String(req.body.time).replace(/"/g, "").replace(" 09:00", "+09:00");
  const unit = req.body.unit === undefined ? undefined : String(req.body.unit);

  const population = await getPopulation({ spatialIds, time, unit });

  res.status(HttpStatusCode.OK).json({ data: population });
};

router.get("/", get);
router.post("/", post);

// eslint-disable-next-line import/no-default-export
export default router;
