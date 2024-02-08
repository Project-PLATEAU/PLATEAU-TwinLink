import { Space } from "@spatial-id/javascript-sdk";
import { ServerEnv } from "../../../config";

export type Point = {
  id: string;
  name: string;
  lat: number;
  lng: number;
  height: number;
};

export const getSpatialId = (point: Point, zoomLevel: number) => {
  const space = new Space(
    { lat: point.lat, lng: point.lng, alt: point.height },
    zoomLevel,
  );
  return space.zfxyStr;
};

export const Cameras: Point[] = JSON.parse(ServerEnv.cameras);
