import { Space } from "@spatial-id/javascript-sdk";
import {
  FeatureCollection,
  Point,
  distance,
  interpolate,
  point,
} from "@turf/turf";
import { PopulationType } from "./populationType";

export type MomentPopulation = {
  id: string;
  type: "measured";
  timestamp: string;
  point: Point;
};

export type MinutesPopulation = PopulationAggregation;

export type HoursPopulation = PopulationAggregation;

export type PopulationAggregation = {
  id: string;
  type: PopulationType;
  timestamp: string;
  peopleFlow: number;
};

export const aggregatePopulation = (
  data: MomentPopulation[],
): PopulationAggregation[] | undefined => {
  if (!data.length) return;

  const groupedData = groupMomentDataBySpatialId(data);
  if (!groupedData) return;
  const accumulatedData: PopulationAggregation[] = groupedData.map((group) => {
    const value = group.length;
    return {
      id: group[0].id,
      type: "measured",
      timestamp: group[0].timestamp,
      peopleFlow: value,
    };
  });

  const maxPeopleFlowOnSpace = removeDuplicate(accumulatedData);
  const interpolated = interpolateAllZoomLevel(maxPeopleFlowOnSpace);
  return interpolated;
};

const removeDuplicate = (data: PopulationAggregation[]): PopulationAggregation[] => {
  const uniqueData: PopulationAggregation[] = [];
  data.forEach((d) => {
    const found = uniqueData.find((ud) => ud.id === d.id);
    if (!found) {
      uniqueData.push(d);
    } else if(found.peopleFlow < d.peopleFlow) {
        uniqueData[uniqueData.indexOf(found)] = d;
    }
  });
  return uniqueData;
}


/*
 * Group moment data by spatial id
 * @param [MomentPopulation[]] momentData - moment data
 * @returns [MomentPopulation[][]] grouped moment data
 * @example
 * // Returns [[{id: "a", timestamp: "2021-01-01T00:00:00.000Z"}, {id: "a", timestamp: "2021-01-01T00:00:00.000Z"}], [{id: "b", timestamp: "2021-01-01T00:00:00.000Z"}]]
 * groupMoment([ {id: "a", timestamp: "2021-01-01T00:00:00.000Z"}, {id: "b", timestamp: "2021-01-01T00:00:00.000Z"}, {id: "a", timestamp: "2021-01-01T00:00:00.000Z"}])
 */
export const groupMomentDataBySpatialId = (
  momentData: MomentPopulation[],
): MomentPopulation[][] | undefined => {
  if (!momentData.length) return undefined;
  const groupedData: MomentPopulation[][] = [];
  momentData.forEach((data) => {
    const found = groupedData.find((group) => group[0].id === data.id && group[0].point.coordinates.every((coord, index) => coord === data.point.coordinates[index]));
    if (found) {
      found.push(data);
    } else {
      groupedData.push([data]);
    }
  });
  return groupedData;
};

export const groupMomentDataByZoomLevels = (
  popData: PopulationAggregation[],
): PopulationAggregation[][] | undefined => {
  const zoomLevels = new Set(popData.map((data) => new Space(data.id).zoom));
  // group by zoom level
  const groupedData: PopulationAggregation[][] = [];
  zoomLevels.forEach((zoomLevel) => {
    const found = popData.filter(
      (data) => new Space(data.id).zoom === zoomLevel,
    );
    if (found) {
      groupedData.push(found);
    }
  });

  return groupedData;
};

// This function interpolates data in the same zoom level. It expects to recieve data grouped by spatial id.
export const interpolateSameZoomLevel = (
  popData: PopulationAggregation[],
  weight: number = 1,
): PopulationAggregation[] | undefined => {
  if (!popData.length) return;
  console.log("params", popData);
  const zoomLevel = new Space(popData[0].id).zoom;

  // Extract centroid of each voxel
  const centerPoints: FeatureCollection<Point> = {
    type: "FeatureCollection",
    features: popData.map((data) => {
      const space = new Space(data.id);
      const { center } = space;
      const centroid = point([center.lng, center.lat], {
        peopleFlow: data.peopleFlow,
        height: center.alt,
        type: "measured",
        id: data.id,
      });
      return centroid;
    }),
  };

  if (centerPoints.features.length < 2) return popData;

  // It needs to keep ids of measured points because turf's interpolate function discards properties
  const measuredPoints = centerPoints.features.map((f) => f.properties?.id);

  const [point1, point2] = new Space(popData[0].id).vertices3d();
  const cellSize = distance(point1, point2); // km


  const interpolatedPoints = interpolate(centerPoints, cellSize, {
    gridType: "point",
    property: "peopleFlow",
    weight,
    units: "kilometers",
  });


  // Calculate average height of all centroids as interpolated voxel doesn't have height
  const averageHeight =
    centerPoints.features.reduce(
      (acc, cur) => acc + (cur.properties?.height ?? 0),
      0,
    ) / centerPoints.features.length;

  const interpolatedData: PopulationAggregation[] =
    interpolatedPoints.features.map((f) => {
      const peopleFlow = f.properties?.peopleFlow;
      const [lng, lat] = f.geometry.coordinates;
      const id = new Space(
        { lat, lng, alt: f.properties?.height ?? averageHeight },
        zoomLevel,
      ).zfxyStr;
      return {
        id,
        peopleFlow,
        timestamp: popData[0].timestamp,
        type: measuredPoints.includes(id) ? "measured" : "interpolated",
      };
    });
  return interpolatedData;
};

// This function interpolates all zoom levels. It expects to recieve data grouped by zoom level.
export const interpolateAllZoomLevel = (
  popData: PopulationAggregation[],
  weight: number = 1,
): PopulationAggregation[] | undefined => {
  if (!popData.length) return;
  const groupedData = groupMomentDataByZoomLevels(popData);
  if (!groupedData) return [];
  const interpolatedData = groupedData.map((group) =>
    interpolateSameZoomLevel(group, weight),
  );
  return interpolatedData
    .flat()
    .filter((data): data is PopulationAggregation => data !== undefined);
};
