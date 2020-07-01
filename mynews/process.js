import cheerio from "cheerio";
import request from "request";
import iconvLite from "iconv-lite";
import rp from "request-promise-native";

// processing news text

var REGKOR = /[^ㄱ-ㅎㅏ-ㅣ가-힣]+/;
//조사를 거르는 정규식.
var REGJOSA = /(이어)$|(없다)$|(보인다)$|(그러나)$|[은는이가을를에이의와들과인]$|(하다)$|(라며)$|(있다)$|(까지)$|(하면서)$|(이나)$|(으로)$|(에서)$|(에게)$|(에게)$|(이다)$|(이었다)$|(었다)$|(하는)$|(했다)$|(하고)$|(졌다)$/;

const rankUrl =
  "https://news.naver.com/main/ranking/popularDay.nhn?rankingType=popular_day&sectionId=100&date=20190921";
const rankUrl2 =
  "https://news.naver.com/main/ranking/popularDay.nhn?rankingType=popular_day&sectionId=100&date=20190920";
const rankUrl3 =
  "https://news.naver.com/main/ranking/popularDay.nhn?rankingType=popular_day&sectionId=100&date=20190919";

const NEWS_URL =
  "https://news.naver.com/main/read.nhn?mode=LSD&mid=shm&sid1=102&oid=032&aid=0002963914";

let rpOptions = {
  url: rankUrl,
  encoding: null,
  transform: body => {
    return iconvLite.decode(body, "euc-kr");
  }
};

/**8************************************************************************************8 */
const parseRankUrl = (decodedBody, rankList) => {
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
      rank: 0
    });
  });
};
/*
    하고 싶은거. 가장 높은 뉴스 조회순 3 페이지에 접속해서
    allRankingList 라는 기사에 각 타이틀, url, rank=0 정보를 넣기. async 하게 돌아가야함.
    필요한거 : urls-> 이거 조작이 가능 하면 좋은데 우선은, manual하게 넣고 되면 일반화ㄱ
*/
const IDFlist = async today => {
  let rankList = [];
  let tempPromise;

  // 생략 : set today => rpOption.url

  for (let i = 0; i < 3; i++) {
    tempPromise = await rp(rpOptions);
    parseRankUrl(tempPromise, rankList);

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
//
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
  let decodedBody = await rp(parsingOption);
  const $ = cheerio.load(decodedBody);
  let newsText = $("div._article_body_contents").text();
  let splitNews = newsText.split(REGKOR);
  splitNews = eraseJosa(splitNews, REGJOSA);
  return new Promise((resolve, reject) => {
    resolve(splitNews);
  });
};

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

export const init = async () => {
  let today = new Date().toISOString(); // 임시로 만들엇슴, 이후 rp.Option에 맞는 형태로 변경
  console.log(1);
  const rankList = await IDFlist(today); // 오늘 날짜 받아서, 오늘 부터 3일 전까지 랭킹 90위 뉴스 url 리스트 받아옴. ranklist에 저장.
  //const temp = await readNews(NEWS_URL); // input = news URL => output : words array
  //console.log(countArr(temp));
  //rankList to obj
  console.log(2);
  const IDFOBJ = await makeIDFobj(rankList);
  console.log(IDFOBJ);
};

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
