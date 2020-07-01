import express from "express";
import routes from "./routes";

export const basicMiddle = (req, res, next) => {
  const bgImage =
    "./myImages/bg_archi" + Math.floor(Math.random() * 4) + ".jpg";
  res.locals.bgImage = bgImage;
  res.locals.routes = routes;
  res.locals.page = "News Reccomendation";
  res.locals.pageName = "NAVER News Guide";
  next();
};
