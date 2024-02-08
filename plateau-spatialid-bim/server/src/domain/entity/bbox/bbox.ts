import { Space } from "@spatial-id/javascript-sdk";
import { MomentPopulation } from "../population";

export type Bbox = [number, number, number, number]; // [minLat, maxLat, minLon, maxLon]

export const isBboxValid = (
  bbox?: number[],
): bbox is [number, number, number, number] => {
  if (!bbox) return false;
  if (bbox.length !== 4) return false;
  if (bbox[0] >= bbox[1]) return false;
  if (bbox[2] >= bbox[3]) return false;
  return true;
};

export const bboxToVertex = (bbox: Bbox): [number, number][] => [
  [bbox[0], bbox[2]],
  [bbox[0], bbox[3]],
  [bbox[1], bbox[3]],
  [bbox[1], bbox[2]],
];

export const bboxSpatialId = (bbox: Bbox, zoom: number): string[] => {
  const vertex = bboxToVertex(bbox);
  const spaces = vertex.map(
    (v) => new Space({ lat: v[0], lng: v[1], alt: 0 }, zoom).zfxyStr,
  );
  return spaces;
};

export const bboxSpatialIdAllZoomLevel = (
  bbox: Bbox,
  zoomLevels: number[],
): string[] => {
  if (!bbox.length || zoomLevels.length === 0) return [];
  const spaces: string[] = [];
  zoomLevels.forEach((zoom) => {
    spaces.push(...bboxSpatialId(bbox, zoom));
  });
  return spaces;
};

export const bboxMomentData = (
  bbox: Bbox,
  zoomLevels: number[],
): MomentPopulation[] => {
  if (!bbox.length || zoomLevels.length === 0) return [];
  const spaces = bboxSpatialIdAllZoomLevel(bbox, zoomLevels);
  const momentData: MomentPopulation[] = [];
  spaces.forEach((space) => {
    const s = new Space(space);
    momentData.push({
      id: space,
      timestamp: new Date().toString(),
      type: "measured",
      point: {
        type: "Point",
        coordinates: [s.center.lng, s.center.lat],
      },
    });
  });
  return momentData;
};
