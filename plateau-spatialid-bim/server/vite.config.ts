import { defineConfig } from "vitest/config";

export default defineConfig({
  test: {
    env: {
      CAMERAS:
        '[{"id":"1","name":"西新宿003","lat":35.691149,"lng":139.695587,"height":39.3},{"id":"2","name":"西新宿005","lat":35.690905,"lng":139.69561,"height":39.2}]',
      MONGO_DB_NAME: "test",
      DATABASE_TYPE: "mongo",
      TZ: "UTC",
    },
    include: ["{src,test}/**/*.{test,spec}.{ts,mts,cts,tsx}"],
  },
});
