import { describe, expect, it, vi } from "vitest";
import { parseTimeToHourlyJst } from "./parseTime";

describe("parseTime", () => {
  // システムTZの関係でgithub actionsでテストが通らないので一旦skip
  it("引数なしの場合、現在日時が返ること", () => {
    vi.useFakeTimers();
    const date = new Date(2023, 10, 10);
    vi.setSystemTime(date);

    const expected = "2023-11-10T08:00:00+09:00";
    const result = parseTimeToHourlyJst();

    expect(result).toStrictEqual(expected);
  });

  it("引数ありの場合、引数の日時がJSTで返ること", () => {
    const arg = "2023-09-14T00:59:00-08:00";

    const expected = "2023-09-14T17:00:00+09:00";
    const result = parseTimeToHourlyJst(arg);

    expect(result).toStrictEqual(expected);
  });
});
