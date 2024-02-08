/* eslint-disable no-nested-ternary */

import { Space } from "@spatial-id/javascript-sdk";
import { it, describe, expect } from "vitest";
import { MomentPopulation, groupMomentDataBySpatialId } from "./population";

const date = Date.now().toString();

const adjacentSpaces = (
  s: Space,
  by: number,
  direction: "north" | "south" | "east" | "west",
): Space[] => {
  if (by <= 0) return [];
  const newSpace =
    direction === "north"
      ? s.north(1)
      : direction === "south"
      ? s.south(1)
      : direction === "east"
      ? s.east(1)
      : s.west(1);
  return [newSpace, ...adjacentSpaces(newSpace, by - 1, direction)];
};

describe("groupMomentDataBySpatialId", () => {
  it("空間IDごとにグループ化が行われるか", () => {
    const testData: MomentPopulation[] = [
      { id: "a", timestamp: date, type: "measured", point: { type: "Point", coordinates: [0, 0] } },
      { id: "b", timestamp: date, type: "measured", point: { type: "Point", coordinates: [0, 0] }},
      { id: "a", timestamp: date, type: "measured", point: { type: "Point", coordinates: [0, 0] } },
      { id: "c", timestamp: date, type: "measured", point: { type: "Point", coordinates: [0, 0] } },
    ];
    const result = groupMomentDataBySpatialId(testData);
    expect(result).toEqual([
      [
        { id: "a", timestamp: date, type: "measured", point: { type: "Point", coordinates: [0, 0] } },
        { id: "a", timestamp: date, type: "measured", point: { type: "Point", coordinates: [0, 0] }},
      ],
      [{ id: "b", timestamp: date, type: "measured", point: { type: "Point", coordinates: [0, 0] } }],
      [{ id: "c", timestamp: date, type: "measured", point: { type: "Point", coordinates: [0, 0] } }],
    ]);
  });
});

describe("groupMomentData", () => {
  it("ズームレベルごとにグループ化されるか？", () => {
    const data: MomentPopulation[] = [17, 17, 18, 18, 19, 19].map(
      (zoomLevel) => ({
        id: new Space({ lng: 10, lat: 10, alt: 0 }, zoomLevel).id,
        timestamp: date,
        type: "measured",
        point: { type: "Point", coordinates: [0, 0] }
      }),
    );
    const result = groupMomentDataBySpatialId(data);
    expect(result?.length).toBe(3);
    const zoomLevels = result?.map((group) => new Space(group[0].id).zoom);
    expect(zoomLevels).toEqual([17, 18, 19]);
  });
});

// TODO: test for aggregatePopulation
// describe('interplateAggregatedData function', () => {
//     const s11 = new Space({ lng: 10, lat: 10, alt: 0 }, 16);

//     const [s12, s13, s14, s15] = adjacentSpaces(s11, 4, "east")
//     const [s21, s31] = adjacentSpaces(s11, 2, "south")
//     const [s22, s23, s24, s25] = adjacentSpaces(s21, 4, "east")

//     const input: PopulationAggregation[] = [
//       { id: s11.zfxyStr, timestamp: '2021-01-01T00:00:00.000Z', type: 'measured', peopleFlow: 10 },
//       { id: s13.zfxyStr, timestamp: '2021-01-01T00:00:00.000Z', type: 'measured', peopleFlow: 30 },
//       { id: s15.zfxyStr, timestamp: '2021-01-01T00:00:00.000Z', type: 'measured', peopleFlow: 50 },
//       { id: s21.zfxyStr, timestamp: '2021-01-01T00:00:00.000Z', type: 'measured', peopleFlow: 20 },
//       { id: s31.zfxyStr, timestamp: '2021-01-01T00:00:00.000Z', type: 'measured', peopleFlow: 30 },
//     ];
//     const expected: PopulationAggregation[] = [
//       { id: s11.id, timestamp: '2021-01-01T00:00:00.000Z', type: 'measured', peopleFlow: 10 },
//       { id: s12.id, timestamp: '2021-01-01T00:00:00.000Z', type: 'interpolated', peopleFlow: 20 },
//       { id: s13.id, timestamp: '2021-01-01T00:00:00.000Z', type: 'interpolated', peopleFlow: 30 },
//       { id: s14.id, timestamp: '2021-01-01T00:00:00.000Z', type: 'interpolated', peopleFlow: 40 },
//       { id: s15.id, timestamp: '2021-01-01T00:00:00.000Z', type: 'interpolated', peopleFlow: 50 },
//     ];

//     it('should interpolate data with weight parameter', () => {
//       const result = interpolateSameZoomLevel(input);
//     expect(result).toBeInstanceOf(Array);
//     expect(result).toHaveLength(2);
//   });

// });
