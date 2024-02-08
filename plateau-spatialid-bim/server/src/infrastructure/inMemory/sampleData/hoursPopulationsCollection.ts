import { HoursPopulation } from "../../../domain/entity/population/population";

export const hoursPopulationsCollection: HoursPopulation[] = [
  {
    id: "/15/0/1111/1111",
    type: "measured", // 実測データ
    timestamp: "2023-09-14T15:00:00+09:00", // accumulative data of past 1 hour
    peopleFlow: 100,
  },
  {
    id: "/15/0/2222/2222",
    type: "interpolated", // 補完データ
    timestamp: "2023-09-14T15:00:00+09:00",
    peopleFlow: 50,
  },
  {
    id: "/15/0/1111/1111",
    type: "measured", // 実測データ
    timestamp: "2023-09-14T16:00:00+09:00", // accumulative data of past 1 hour
    peopleFlow: 200,
  },
  {
    id: "/15/0/2222/2222",
    type: "interpolated", // 補完データ
    timestamp: "2023-09-14T16:00:00+09:00",
    peopleFlow: 100,
  },
  {
    id: "/15/0/3333/3333",
    type: "measured", // 実測データ
    timestamp: "2023-09-14T16:00:00+09:00", // accumulative data of past 1 hour
    peopleFlow: 200,
  },
  {
    id: "/15/0/3333/3333",
    type: "interpolated", // 補完データ
    timestamp: "2023-09-14T16:00:00+09:00",
    peopleFlow: 100,
  },
];
