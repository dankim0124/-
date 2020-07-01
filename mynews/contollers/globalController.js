import {  myTfIdf, getCosRanking, updateTfIdfDistance } from "../init";
export const index = async (req, res) => {
 
  /*
  result = myTfIdf;
  res.locals.headLine1= result[0].url;
  res.locals.headLine2 = result[1].url;
  res.locals.headLine3 = result[2].url;
  res.locals.headLine4 = result[3].url;
  res.locals.headLine5 = result[4].url;
  */
  //checkNewsDate(req,res,"1123", myTfIdf)
  res.render("index");
};

export const postIndex = async (req,res,next)=>{
 
  const {
    body: { inputUrl}
  } = req;

  console.log("input url : " , inputUrl)
  let i =0;  
  let tfIdf_object;
  let urlList=[];
  let rankList;
  let tmp=0;
  await myTfIdf.then(data => {
    tfIdf_object = data.result;  
    tfIdf_object.forEach(element => {
      console.log(i)
      i= i+1;
      console.log(element.url)
      urlList.push(element.url);
      if (element.url == inputUrl){
        tmp = tmp+1;
      }
    });  
  });
    console.log(inputUrl);
    if(tmp != 0){
      console.log( "tf-idf start");
      rankList = updateTfIdfDistance(tfIdf_object, inputUrl);
      rankList.reverse();
      }
    else{
      console.log("cosdis start");
      rankList = await getCosRanking(inputUrl);      
    }
    console.log(rankList);
    res.locals.headLine1 = rankList[0].title;
    res.locals.headLine2 = rankList[1].title;
    res.locals.headLine3 = rankList[2].title;
    res.locals.headLine4 = rankList[3].title;
    res.locals.headLine5 = rankList[4].title;

    res.locals.url1 = rankList[0].url
    res.locals.url2 = rankList[1].url
    res.locals.url3 = rankList[2].url
    res.locals.url4 = rankList[3].url
    res.locals.url5 = rankList[4].url
    res.render("output" , {headLine1 : res.locals.headLine1});
  
  //console.log(rankList);
  //await asyncMove(req,res,inputUrl)
  //res.redirect(routes.index);
    //res.render("index" , {headLine1 : res.locals.headLine1});
}


export const asyncMove = async (req,res,inputUrl)=>{
  //let result = await init(inputUrl);
  let result = await tfidf_news(inputUrl);
 // console.log(result);
 
  res.locals.headLine1= result[0].title;
  res.locals.headLine2 = result[1].title;
  res.locals.headLine3 = result[2].title;
  res.locals.headLine4 = result[3].title;
  res.locals.headLine5 = result[4].title;
  
  res.locals.url1 = result[0].url
  res.locals.url2 = result[1].url
  res.locals.url3 = result[2].url
  res.locals.url4 = result[3].url
  res.locals.url5 = result[4].url
  return new Promise((resolve, reject) => {
    resolve(res)
  });
} 

export const checkNewsDate = async(req,res,inputUrl,myTfIdf) =>{
  let TFIDF  =null; // 나중에 여기에 파일 결과 가져올거임
  let tmpUrls = [];
  console.log( "check news date call")
  /*
  for( element in TFIDF) {
    tmpUrls.push(element.url);
  }
  if( inputUrl in tmpUrls){
    getTFIDFrank();
  }
  else{
    asyncMove(inputUrl);
  }*/
}


export const loading = (req, res) => {
  res.render("loading");
};

export const output = (req, res) => {
  res.render("output");
};
