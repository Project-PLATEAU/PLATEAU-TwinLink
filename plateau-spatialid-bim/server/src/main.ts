import "reflect-metadata";
import express from "express";
import helmet from "helmet";
import { errorHandler } from "./middleware/errorHandler";
import populationRouter from "./presentation/populationController";
import { dependencyInjectHandler } from "./util/dependencyInjection";
import { Logger } from "./util/logger";

const app = express();
const port = process.env.PORT || "9081";

// helmetによるセキュリティ対策 https://helmetjs.github.io/
app.use(helmet());

app.use(express.json());

app.use(Logger.initialize);

// health check
app.get("/api/_health", async (req, res) => {
  res.send("OK");
});

app.get("/", (req, res) => {
  res.send("Hello World!");
});

app.use(dependencyInjectHandler);

app.use("/population", populationRouter);

app.use(errorHandler);

app.listen(port, () => console.log(`Listening on http://localhost:${port}/`));
