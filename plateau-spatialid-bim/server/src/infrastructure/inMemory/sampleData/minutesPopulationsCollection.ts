import { MinutesPopulation } from "../../../domain/entity/population/population";

export const minutesPopulationsCollection: MinutesPopulation[] = [
  {
    id: "/15/0/1111/1111",
    type: "measured", // 実測データ
    timestamp: "2023-09-14T15:10:00+09:00", // accumulative data of past 1 hour
    peopleFlow: 20,
  },
  {
    id: "/15/0/2222/2222",
    type: "interpolated", // 補完データ
    timestamp: "2023-09-14T15:30:00+09:00",
    peopleFlow: 10,
  },
];
