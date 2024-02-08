import { describe, expect, it } from "vitest";
import { ParseSensorRawCsv } from "./parseCsv";

describe("parseCsv", () => {
  const sampleCsv =
    "時刻,入退店,企業,エリア,店舗,カメラ,トラッキングID\n2023/10/12 04:18:43,退店,株式会社,西新宿,西新宿003,西新宿003,0df51dea-65c5-420c-9deb-a8fc078b72c9\n2023/10/12 04:49:37,入店,株式会社テスト,西新宿,西新宿003,西新宿003,225a6302-5342-434a-a71b-589d0f644a74\n2023/10/12 04:23:45,退店,株式会社テスト,西新宿,西新宿003,西新宿003,242e4a23-95e3-4a16-8376-6d5f0504a3b4";
  it("csvをsensorRawRecordに変換できること", () => {
    const momentPopulations = ParseSensorRawCsv(sampleCsv);
    const expected = JSON.stringify([
      "時刻",
      "入退店",
      "企業",
      "エリア",
      "店舗",
      "カメラ",
      "トラッキングID",
    ]);

    expect(
      momentPopulations.every(
        (p) => JSON.stringify(Object.keys(p)) === expected,
      ),
    ).toBe(true);
  });
});
