/*
할거
  1) 크롤링 예외처리
  2) 정규식 말고 형태소 분석기 써보기
  3) 아래 함수랑 메인페이지 함수랑 분류.. 메인페이지는 json 으로 미리 저장해놓기/.
  4) TF-IDF 결과 매트릭스 제작해서 Joson으로 저장하기. 
*/


import express from "express";
import request from "request"
import cheerio from "cheerio";
import iconvLite from "iconv-lite";
import rp from "request-promise-native";
import app from "./app";
import { SSL_OP_EPHEMERAL_RSA } from "constants";
const PORT = 3000;
app.listen(PORT, () => {
  console.log("Listening on PORT ");
});
// processing news text
var REGKOR = /[^ㄱ-ㅎㅏ-ㅣ가-힣]+/;
//조사를 거르는 정규식.
var REGJOSA = /(여전히)$|(않았다)$|(무려)$|(들도)$|(이어)$|(없다)$|(보인다)$|(그러나)$|(로는)$|[은는이가을를에이의와들과인로]$|(하다)$|(라며)$|(있다)$|(까지)$|(하면서)$|(이나)$|(으로)$|(에서)$|(에게)$|(에게)$|(이다)$|(이었다)$|(었다)$|(하는)$|(했다)$|(하고)$|(졌다)$/;

let IMPORTANT_WORD = [];

const rankUrl =
  "https://news.naver.com/main/ranking/popularDay.nhn?rankingType=popular_day&sectionId=100&date=20191105";
const rankUrl2 =
  "https://news.naver.com/main/ranking/popularDay.nhn?rankingType=popular_day&sectionId=100&date=20190920";
const rankUrl3 =
  "https://news.naver.com/main/ranking/popularDay.nhn?rankingType=popular_day&sectionId=100&date=20190919";

const NEWS_URL =
  "https://news.naver.com/main/ranking/read.nhn?rankingType=popular_day&oid=421&aid=0004287064&date=20191105&type=1&rankingSectionId=100&rankingSeq=2";

let rpOptions = {
  url: rankUrl,
  encoding: null,
  transform: body => {
    return iconvLite.decode(body, "euc-kr");
  }
};

/**8************************************************************************************8 */
const parseRankUrl = async (decodedBody, rankList) => {
  const $ = cheerio.load(decodedBody);
  let $rankings = $("ol.ranking_list").children("li.ranking_item");
  $rankings.each(function(i, elem) {
    rankList.push({
      url: $(this)
        .find("div.ranking_text div.ranking_headline a")
        .attr("href"),
      title: $(this)
        .find("div.ranking_text div.ranking_headline a")
        .attr("title"),
      cosineDistance: 0
    });
  });
};
/*
    하고 싶은거. 가장 높은 뉴스 조회순 3 페이지에 접속해서
    allRankingList 라는 기사에 각 타이틀, url, rank=0 정보를 넣기. async 하게 돌아가야함.
    필요한거 : urls-> 이거 조작이 가능 하면 좋은데 우선은, manual하게 넣고 되면 일반화ㄱ
*/
const makeRankList = async today => {
  let rankList = [];
  let tempPromise;
  
  // 생략 : set today => rpOption.url
  // TODO : 위쪽에 지정해 놓은 url 말고 날짜로 url 동적으로 만들기.
  for (let i = 0; i < 3; i++) {
    tempPromise = await rp(rpOptions);
    await parseRankUrl(tempPromise, rankList);
    // change  url date;
    console.log(rpOptions.url);
    let date = parseInt(rpOptions.url.substr(rpOptions.url.length - 8));
    rpOptions.url = rpOptions.url.replace(String(date), String(date - 1));
  }
  return new Promise((resolve, reject) => {
    resolve(rankList);
  });
};

/***************************************************************************************************888 */
//****************************************************************************************8888 */
// eraseJosa & readNews => 읽어와서 단어로 짜르기.
//곧 죽을 함수입니다...... 형태소 분석기 새로운거 써볼거임.

const eraseJosa = (textArray, regJosa) => {
  textArray.forEach((element, index, array) => {
    textArray[index] = element.replace(regJosa, "");
    //비어버린 배열을 버리기.... 1개짜리 스트링은 구분하는데 도움이 덜해서 임의로 걸렀습니다.
  });
  textArray = textArray.filter(word => word.length > 1);
  return textArray;
};

// input : url  -> output : 단어단위 문자 배열.
const readNews = async newsUrl => {
  let parsingOption = {
    url: newsUrl,
    encoding: null,
    transform: body => {
      return iconvLite.decode(body, "euc-kr");
    }
  };
  console.log("read news~~ : url: ", parsingOption.url,"\n")
  let decodedBody = await rp(parsingOption);
  console.log("after rp~ ")
  const $ = cheerio.load(decodedBody);
  let newsText = $("div._article_body_contents").text();
  let splitNews = newsText.split(REGKOR);
  splitNews = eraseJosa(splitNews, REGJOSA);
  return new Promise((resolve, reject) => {
    resolve(splitNews);
  });
};
// 예외처리 필요, 이유 불명, 크롤링 중. 404 unhandle promise err 뜨는게 있음.. 기사가 없어지거나, 요청을 제시간에 못찾는 경우 있을수도...
// 이런 경우, 그냥 splitedNews = 그거, 빈 [] 을 넣어서 관리하자 아니면 [state:"rejected"] 이런식으로 넣던가.

//array => obj .... ([{word:count},{word:count}.....])
const countArr = (newsArr, countObj) => {
  let wordCount = countObj || {};
  newsArr.forEach((element, index, array) => {
    if (wordCount.hasOwnProperty(element)) {
      wordCount[element] = wordCount[element] + 1;
    } else {
      wordCount[element] = 1;
    }
  });
  return wordCount;
};

//senario2 ..... cosine distance + array approach.
// imput => array   , output => processed "word counting array"
const countArr2 = (newsArr, countObj) => {
  let wordCount = countObj || [];
  console.log("count2 arr~")
  newsArr.forEach((element, index, array) => {
    //wordCount =[ {word: ~, count : ~}]
    const checkWord = wordCount.find(c => c.word == element);
    if (checkWord != undefined) {
      const index = wordCount.indexOf(checkWord);
      wordCount[index].count = wordCount[index].count + 1;
    } else {
      let makeObj = { word: element, count: 1 };
      wordCount.push(makeObj);
    }
  });
  //optional part => should functionalize.... with static var IMPORTANT_WORD
  let result = wordCount.filter(element => element.count > 1);
  console.log(result);
  console.log(result.length);
  return result;
};

//잘못만듬 -> TF-IDF 에서 전체 문자배열 찾을 때 이용.
// 코사인
const makeIDFobj = async rankList => {
  let IDFOBJ = {};
  let tmpUrl;
  for (const elem of rankList) {
    tmpUrl = "https://news.naver.com" + elem.url;
    let tmpNews = await readNews(tmpUrl);
    await countArr(tmpNews, IDFOBJ);
  }
  return IDFOBJ;
  /*
  await rankList.forEach(async (element, index, array) => {
    let tmpUrl = "https://news.naver.com" + element.url;
    let tmpNews = await readNews(tmpUrl);
    console.log(tmpNews);
    countArr(tmpNews, IDFOBJ);
  });*/
};

const getCosineDistance = (wordArray, compareArray) => {
  let COSINE_DISTANCE = 0;
  let wordArraySize = 0;
  let compareArraySize = 0;
  for (let i = 0; i < wordArray.length; i++) {
    wordArraySize += wordArray[i].count * wordArray[i].count;
  }
  for (let i = 0; i < compareArray.length; i++) {
    compareArraySize += compareArray[i].count * compareArray[i].count;
  }
  const denomitator = Math.sqrt(wordArraySize) * Math.sqrt(compareArraySize);
  for (let i = 0; i < wordArray.length; i++) {
    for (let j = 0; j < compareArray.length; j++) {
      if (wordArray[i].word == compareArray[j].word) {
        console.log(
          "mathing word : ",
          wordArray[i].word,
          "count => ",
          wordArray[i].count,
          " & ",
          compareArray[j].count
        );
        COSINE_DISTANCE += wordArray[i].count * compareArray[j].count;
      }
    }
  }
  console.log(COSINE_DISTANCE / denomitator);
  return COSINE_DISTANCE / denomitator;
};

/******************************************************************************8 */

/*
    해야 할 것 
        1. 배열 -> 오브젝트
            let countNum = [];
            forEach((elem,index)=>{
                if (elem이 countNum의 key로 없으면)
                    countNum.push({elem: 1});
                else
                    countNum[elem] = countNum[elem] +1;
            })

            2.
            rankList -> 90개의 url
            readNews -> url => 배열
            배열 -> obj로 ..... IDF_OBJ 로 다시 생성 해야함.  
                let IDF_OBJ = [];
                let IDF_ARR = [];
                for( let i = 0; i< rankList.length ; i++){
                    let tempArr = readNews(rpOption.url);
                    IDF_ARR.push(tempArr);                    
                }
                IDF_ARR => IDF_OBJ;

            */

const sortRankList = rankList => {
  return rankList.sort((a, b) => {
    return a.cosineDistance < b.cosineDistance
      ? -1
      : a.cosineDistance > b.cosineDistance
      ? 1
      : 0;
  });
};

const updateCosineDistance = async (rankList, NEWS_URL) => {
  const controllArray = countArr2(await readNews(NEWS_URL));

  let tmpUrl, compareArray, cosDis;

  for (const elem of rankList) {
    console.log("Before read URL : ", elem.url);
    tmpUrl = "https://news.naver.com" + elem.url;
    compareArray = countArr2(await readNews(tmpUrl));
    console.log("after read URL ", elem.url)
    cosDis = getCosineDistance(controllArray, compareArray);
    elem.cosineDistance = cosDis;
  }
  sortRankList(rankList);
  for (let i = 0; i < rankList.length; i++) {
    console.log(
      "title: ",
      rankList[i].title,
      "\t\t\t score : ",
      rankList[i].cosineDistance
    );
  }
  return rankList;
};

const init = async () => {
  let today = new Date().toISOString(); // 임시로 만들엇슴, 이후 rp.Option에 맞는 형태로 변경
  const rankList = await makeRankList(today); // 오늘 날짜 받아서, 오늘 부터 3일 전까지 랭킹 90위 뉴스 url 리스트 받아옴. ranklist에 저장.
  let i = 0
  console.log(rankList)
  await updateCosineDistance(rankList, NEWS_URL);
  console.log("DONE");
  //const IDFOBJ = await makeIDFobj(rankList);
};

//init();

init();

/*
    TO DO : 
        version 1 해야 할 일 == 이번주에 해야 할 일.
            1. IDF LIST 만드는 함수 안에 각 url 읽어와서 배열로 만들어야함.
            2. readnews에서 만든 배열로 배열 =-> object 로 만들어서 카운팅 하고, IDF object 만드는 거 시간측정.
            3. 2에서 시간이 너무 오래 걸리면 60개로 줄여야 할수도....
            ............ ㄴ> 꼮 해야 하는거 

            4. 뉴스 내에서 뒷쪽 쓸모없는 단어들, [리포터] [엥커] ,, 조선일보, 한국일보 .... 이런 단어들 거르기...
            5. 성능 높이기 .... -> 아이디어 생기면 추후 기록.

        version 2 부터 해야 할 일
             1. 웹 요청 중 예외처리.***** => 젤중요
             2. html view 제작.+ loading page 제작.
            3. github.io에 서버 만들기.
            4. url 에서 sid 번호로 뉴스카테고리 분류.
*/
