import { describe, expect, it } from "vitest";
import {
  Bbox,
  bboxMomentData,
  bboxSpatialId,
  bboxSpatialIdAllZoomLevel,
  bboxToVertex,
  isBboxValid,
} from "./bbox";

describe("bboxToVertex", () => {
  it("should return the correct vertices for a valid bbox", () => {
    const bbox: Bbox = [10, 20, 30, 40];
    const expectedVertices: [number, number][] = [
      [10, 30],
      [10, 40],
      [20, 40],
      [20, 30],
    ];
    expect(bboxToVertex(bbox)).toEqual(expectedVertices);
  });
});

describe("isBboxValid", () => {
  it("should return true for a valid bbox", () => {
    const bbox: Bbox = [10, 20, 30, 40];
    expect(isBboxValid(bbox)).toBe(true);
  });

  it("should return false for an invalid bbox", () => {
    const bbox: Bbox = [10, 10, 20, 20];
    expect(isBboxValid(bbox)).toBe(false);
  });

  it("should return false for a bbox with larger number of elements", () => {
    const bbox = [10, 10, 10, 10, 10, 10];
    expect(isBboxValid(bbox)).toBe(false);
  });

  it("should return false for a bbox with the smaller number of elements", () => {
    const bbox = [10, 20, 30];
    expect(isBboxValid(bbox)).toBe(false);
  });
});

describe("bboxSpatialId", () => {
  it("should return the correct spatial IDs for a valid bbox and zoom level", () => {
    const bbox: Bbox = [10, 20, 30, 40];
    const zoomLevel = 10;
    const result = bboxSpatialId(bbox, zoomLevel);
    expect(result.length).toBe(4);
  });
});

describe("bboxSpatialIdAllZoomLevel", () => {
  it("should return the correct spatial IDs for a valid bbox and zoom levels", () => {
    const bbox: Bbox = [10, 20, 30, 40];
    const zoomLevels = [10, 11];
    const result = bboxSpatialIdAllZoomLevel(bbox, zoomLevels);
    expect(result.length).toBe(8);
  });
});

describe("bboxMomentData", () => {
  it("should return the correct MomentPopulation objects for a valid bbox and zoom levels", () => {
    const bbox: Bbox = [10, 20, 30, 40];
    const zoomLevels = [10, 11];
    const result = bboxMomentData(bbox, zoomLevels);
    expect(result.every((r) => !!r.id)).toBe(true);
    expect(result.every((r) => !!r.timestamp)).toBe(true);
    expect(result.every((r) => r.type === "measured")).toBe(true);
    expect(result.length).toBe(8);
  });
});
