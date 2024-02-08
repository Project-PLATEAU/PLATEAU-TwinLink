import { CustomError } from "../../../middleware/errorHandler";
import HttpStatusCode from "../../../util/httpStatusCode";
import { IntervalUnit } from "../../entity/intervalUnit/intervalUnit";

export const parseUnit = (unit?: string): IntervalUnit => {
  if (unit === undefined) return "hours";
  if (unit === "hours") {
    return "hours";
  }
  if (unit === "minutes") {
    return "minutes";
  }
  throw new CustomError("unit is invalid", HttpStatusCode.BAD_REQUEST);
};
