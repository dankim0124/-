
import cheerio from "cheerio";
import iconvLite from "iconv-lite";
import rp from "request-promise-native";
import app from "./app";
import fs from "fs";

const PORT = 3000;

let myTfIdf;

app.listen(PORT, () => {
  module.exports.myTfIdf = make_tfIdf_matrix(myTfIdf);  
  
});

//한글만 추출하는 정규식
var REGKOR = /[^ㄱ-ㅎㅏ-ㅣ가-힣]+/;
// 조사를 거르는 정규식
var REGJOSA = /(끼리)$|(여전히)$|(않았다)$|(무려)$|(들도)$|(이어)$|(없다)$|(보인다)$|(그러나)$|(로는)$|[은는이가을를에이의와들과로]$|(하다)$|(라며)$|(있다)$|(까지)$|(하면서)$|(이나)$|(으로)$|(에서)$|(에게)$|(에게)$|(이다)$|(이었다)$|(었다)$|(하는)$|(했다)$|(하고)$|(졌다)$/;

let rankUrl =
  "https://news.naver.com/main/ranking/popularDay.nhn?rankingType=popular_day&sectionId=100&date=20191113";

let rpOptions = {
  url: rankUrl,
  encoding: null,
  transform: body => {
    return iconvLite.decode(body, "euc-kr");
  }
};

// 뉴스 랭킹 페이지를 클로링 해와서 해당 뉴스의 url, 제목을 {url : int, title : str, cosineDistance : 0} 형식으로 저장
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



// 오늘 날짜의 랭킹 뉴스 페이지를 입력받으면 위의 parseRankingUrl을 이용하여, 최근 3일의 조회수 높은 뉴스를 크롤링해옴.
// 최근 3일동안의 랭킹 뉴스 90개를 {url : int, title : str, cosineDistance : 0} 형태로 저장하여 리턴한다.
const makeRankList = async today => {
  let rankList = [];
  let tempPromise;
  let i =0;
  while(i<3) {
    tempPromise = await rp(rpOptions);
    await parseRankUrl(tempPromise, rankList);
    // change  url date;
    let date = parseInt(rpOptions.url.substr(rpOptions.url.length - 8));
    rpOptions.url = rpOptions.url.replace(String(date), String(date - 1));
    i = i+1;
  }
  return new Promise((resolve, reject) => {
    resolve(rankList);
  });
};

// 뉴스의 본문을 처리함, 조사를 전부 버린다. 
const eraseJosa = (textArray, regJosa) => {
  textArray.forEach((element, index, array) => {
    textArray[index] = element.replace(regJosa, "");
  });
  // 어떤 본문에서 1번만 언급된 단어는 버린다.
  //  이유 1 : 처리해야할 단어의 양을 줄인다.
  //  이유 2 : 어처피 이 단어들은 전체 유사도를 평가하는데 큰 영향을 주지 않는다.
  textArray = textArray.filter(word => word.length > 1);
  return textArray;
};

// 뉴스 url 을 받아, 제목부터 본문까지의 텍스트를 읽어와 저장한다.
// 저장 할때는, 띄어쓰기단위로 한글만 저장한다.
// 즉, "나는 과제를 너무 잘했다" 라는 텍스트가 뉴스의 본문이면.
// ["나", "과제", "너무", "잘했다"] 로 저장한다.
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
  //위에서 정의한 정규식을 이용해, 한글만 가져온다.
  let splitNews = newsText.split(REGKOR);
  //한글단위로 나눈 텍스트에서 다시 조사를 자른다.
  splitNews = eraseJosa(splitNews, REGJOSA);
  return new Promise((resolve, reject) => {
    resolve(splitNews);
  });
};

// 위에서 만든 단어의 배열을
// {word: string, count: int} 의 객체의 배열로 만든다.
// ["바나나", "바나나", "사과", "사과", "사과"] 가 입력되면
// [{word: "바나나", count: 2}, {word: "사과", count : 3} 의 형식으로 바꾼다.
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


// 위에서 만든 단어의 배열을
// {word: string, count: int} 의 객체의 배열로 만든다.
// ["바나나", "바나나", "사과", "사과", "사과"] 가 입력되면
// [{word: "바나나", count: 2}, {word: "사과", count : 3} 의 형식으로 바꾼다.
const countArr2 = (newsArr, countObj) => {
  let wordCount = countObj || [];
  newsArr.forEach((element, index, array) => {
    const checkWord = wordCount.find(c => c.word == element);
    if (checkWord != undefined) {
      const index = wordCount.indexOf(checkWord);
      wordCount[index].count = wordCount[index].count + 1;
    } else {
      let makeObj = { word: element, count: 1 };
      wordCount.push(makeObj);
    }
  });
  //위와 같은 이유로, 1개짜리 단어까지 새면 효율이 너무 떨어져서, 배열에서 제외한다.
  let result = wordCount.filter(element => element.count > 1);
  return result;
};


// 위에서 만든 [{word: str, count: int}, ...] 형식의 배열을 이용해서 두 문서의 코사인 거리를 구하는 함수 
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

// rank 리스트를 위에서 구한 cosineDistance 의 크기순으로 배열한다. 
const sortRankList = rankList => {
  return rankList.sort((a, b) => {

    return a.cosineDistance - b.cosineDistance;
    /*
    a.cosineDistance > b.cosineDistance
      ? -1
      : a.cosineDistance < b.cosineDistance
      ? 1
      : 0;*/
  });
};


// ranklist 를 돌면서 각 뉴스 본문을 {word: str, count: int} 으로 만들고,
// ranklist의 요소별로 코사인 거리를 구해서, 그 크기순으로 정렬한 결과를 리턴해 준다.
const updateCosineDistance = async (rankList, NEWS_URL) => {
  // 입력된 뉴스 url 본문 크롤링, 처리
  const controllArray = countArr2(await readNews(NEWS_URL));
  let tmpUrl, compareArray, cosDis;
  
  for (const elem of rankList) {
    // 최신 랭킹 뉴스들의 크롤링, 처리
    tmpUrl = "https://news.naver.com" + elem.url;
    compareArray = countArr2(await readNews(tmpUrl));
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


// 코사인 거리를 구하는 함수.
export const getCosRanking = async (inputUrl) => {
  
  // 오늘 뉴스의 url 만들기.
  let today = new Date().toISOString();  
  let intToday = today.substr(0,4) + today[5] + today[6] + today[8]+today[9]
  let baseUrl = "https://news.naver.com/main/ranking/popularDay.nhn?rankingType=popular_day&sectionId=100&date="
  rpOptions.url = baseUrl + intToday

  // rankList : 최근 3일 동안 랭킹 뉴스들의 url과 제목 받아온다.
  const rankList = await makeRankList(today);
  
  // 코사인 거리를 구함.
  await updateCosineDistance(rankList, inputUrl);
  return new Promise((resolve, reject) => {
    resolve(rankList);
  });
};


// TF IDF정보가 포함된 뉴스들의 리스트 생성.
// news_array[{url: url, title: str, index: int, news : [{word: str ,count: int} ] } ]
// 위의 객체를 리턴한다.
const news_countArr = async (rankList) =>{
  
  let news_array = []
  let i =0
  let tmpUrl = null
  let wordCountArray =null

  for (let elem of rankList){
    tmpUrl ="https://news.naver.com" + elem.url
    wordCountArray = countArr2(await readNews(tmpUrl))
    news_array.push({url:tmpUrl,title:elem.title  ,index:i, news: wordCountArray})
    i += 1
  }
  return new Promise((resolve, reject) => {
    resolve(news_array);
  });
}

// tf-idf 를 계산하기 위한 함수., 먼저 뉴스 본문을 읽어 각 단어의 idf를 계싼한다.
const idf_count = async (allNews)=>{
  let result = [];
  let checkWord = null;
  let wordAndCount = null;
  let tmpWordArr = [];
  
  allNews.forEach(element1 =>{
    wordAndCount = element1.news;
    // 모든 뉴스의 news 객체를 받아옴.
    wordAndCount.forEach(element2=>{
      tmpWordArr.push(element2.word);
    })
  })
  tmpWordArr.sort();
  // 모든 뉴스의 news 객체에 대해, 단어를 검사해서 이미 있으면 +1 을하고 없으면 {word: str, count: int } 의 객체로 리턴될 객체에 집어 넣음. 
  tmpWordArr.forEach(element =>{
    checkWord = result.find(c=>c.word == element)
    if (checkWord != undefined){
      let index = result.indexOf(checkWord);
      result[index].count = result[index].count +1;
    }else{
      let makeObj = {word:element , count:1};
      result.push(makeObj)
    }
  })
  return new Promise((resolve, reject) => {
    resolve(result);
  });
}
//위에서 계산된 idf 값을 이용하여 tf-idf 를 계산함.
const setTfIdf = (allNews,idf )=>{
  let word = null;
  let idfValue = 0;
  let dfValue = 0 ;
  let tfIdfValue = 0;
  allNews.forEach(element=>{
    element.news.forEach(element2 =>{
      word = idf.find(c=>c.word == element2.word)
      dfValue = word.count;
      idfValue = Math.log( 90 / (1+ dfValue));
      tfIdfValue = element2.count * idfValue;
      element2.tfIdf = tfIdfValue;
    })
  })
return allNews  
}


// 배열을 tfidf 로 만든, cosine distance 값을 기준으로 정렬함. 
const sortTfIdfList = rankList => {
  return rankList.sort((a, b) => {
    /*return a.tfIDfDistance < b.tfIdfDistance
      ? -1
      : a.tfIdfDistance >= b.tfIdfDistance
      ? 1 : 0;*/
      return a.tfIdfDistance - b.tfIdfDistance;
  });
};

// 위에서 만든 tf-idf 에 대해 코사인 거리를 적용하여 문서 유사도를 구한다.
const getTfIdfDistance = (target, compareArray) => {
  let TFIDF_DISTANCE = 0;
  let wordArraySize = 0;
  let compareArraySize = 0;
  let wordArray = target;
  for (let i = 0; i < wordArray.length; i++) {
    wordArraySize += wordArray[i].tfIdf * wordArray[i].tfIdf;
  }

  for (let i = 0; i < compareArray.length; i++) {
    compareArraySize += compareArray[i].tfIdf * compareArray[i].tfIdf;
  }
  const denomitator = Math.sqrt(wordArraySize) * Math.sqrt(compareArraySize);
  for (let i = 0; i < wordArray.length; i++) {
    for (let j = 0; j < compareArray.length; j++) {
      if (wordArray[i].word == compareArray[j].word) {
        TFIDF_DISTANCE += wordArray[i].tfIdf * compareArray[j].tfIdf;
      }
    }
  }
  return TFIDF_DISTANCE / denomitator;
};

// tf-idf 거리를 이용하여 입력받은 뉴스 사이의 코사인 거리를 구하고,
// 내림차순으로 정렬하여 리턴해준다.
export const updateTfIdfDistance = (allNews, newsURL)=>{

let targetNews = (allNews.find(c=>c.url == newsURL)); 
targetNews = targetNews.news; 
let compareNews;
let distanceArray = [];
let tmpObj = null;
let distance= 0;

allNews.forEach(element =>{
  compareNews = element.news;
  distance = getTfIdfDistance(targetNews,compareNews );
  tmpObj = {tfIdfDistance : distance, url: element.url, title: element.title};
  distanceArray.push(tmpObj)
 })
distanceArray = sortTfIdfList(distanceArray)
return distanceArray;

};

export const make_tfIdf_matrix = async (inputUrl) =>{
  console.log( "crolling start!")
  let today = new Date().toISOString();   
  let intToday = today.substr(0,4) + today[5] + today[6] + today[8]+today[9]
  let baseUrl = "https://news.naver.com/main/ranking/popularDay.nhn?rankingType=popular_day&sectionId=100&date="
  rpOptions.url = baseUrl + intToday
  const rankList = await makeRankList(today); // 오늘 날짜 받아서, 오늘 부터 3일 전까지 랭킹 90위 뉴스 url 리스트 받아옴. ranklist에 저장.

  // 순위권 뉴스를 [url,index, title, news] 배열로 만듬. 
  let allNews = await news_countArr(rankList);
  // allNews 어레이 안의 news 속성에 tf-idf 값을 추가함.
  let idf = await idf_count(allNews)
  allNews = setTfIdf(allNews, idf)
  
  console.log( "crolling done! ")


  return new Promise((resolve, reject) => {
    resolve({result: allNews});
  });

}