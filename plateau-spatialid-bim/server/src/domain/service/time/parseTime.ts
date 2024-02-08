import { formatInTimeZone } from "date-fns-tz";
import { CustomError } from "../../../middleware/errorHandler";
import HttpStatusCode from "../../../util/httpStatusCode";

export const parseTimeToHourlyJst = (time?: string): string => {
  try {
    const date = time ? new Date(time) : new Date();
    date.setMinutes(0);
    date.setSeconds(0);
    if (!time) {
      date.setHours(date.getHours() - 1);
    }
    return formatInTimeZone(date, "Asia/Tokyo", "yyyy-MM-dd'T'HH:mm:ssXXX");
  } catch (error) {
    throw new CustomError("Invalid time format", HttpStatusCode.BAD_REQUEST);
  }
};
