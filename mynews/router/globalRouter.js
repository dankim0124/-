import express from "express";
import routes from "../routes";
import { loading, index, output, postIndex } from "../contollers/globalController";

const globalRouter = express.Router();

globalRouter.get(routes.index, index);
//나중에 post 요청에 대해서 업데이트

globalRouter.post(routes.index, postIndex);
globalRouter.get(routes.loading, loading);

globalRouter.get(routes.output, output);

export default globalRouter;
