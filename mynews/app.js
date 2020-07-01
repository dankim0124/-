import express from "express";
import helmet from "helmet";
import bodyParser from "body-parser";
import cookieParser from "cookie-parser";
import morgan from "morgan";
import globalRouter from "./router/globalRouter";
import routes from "./routes";
import { basicMiddle,  middle2 } from "./middlewares";
const app = express();

app.use(helmet());

app.use(morgan("dev"));
// body-parser from express???
app.use(bodyParser.urlencoded({ extended: true }));
app.use(bodyParser.json());
app.use(cookieParser());

app.set("view engine", "pug");
app.use(express.static("styles"));
//여기 미들 웨어는 처음 부트할떄만 콜 됨.
app.use(basicMiddle);

console.log( "middle2 is called2")
app.use(routes.index, globalRouter);
export default app;
