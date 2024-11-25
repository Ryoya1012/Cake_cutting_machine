//2024年11月25日
//Author Ryoya SATO
//All rights reserved

String str; //変数宣言
float target_angle; //目標角度を格納する変数

void setup() {
  Serial.begin(115200);
}

void loop() {
  if(Serial.available() > 0) //シリアルデータが受信されているか確認
  {
    str = Serial.readString(); //シリアルから文字列を読み取る

    //文字列に':'が含まれているか確認
    if( str.indexOf(':') != -1) 
    {
      //コロン(:)で区切られたデータを処理
      int colonIndex = str.indexOf(':'); //コロンの位置を取得
      String command = str.substring(0, colonIndex); //コロンの前の文字列(コマンド部分)
      String angle_str = str.substring(colonIndex + 1); //コロンの後の文字列(角度部分)

      //角度部分が数値に変換できるか確認
      if(isNumber(angle_str))
      {
        target_angle = angle_str.toFloat(); //角度を浮動小数点に変換
      /*
      処理内容を記載(円板回転)
      */
      Serial.println("READY"); //PCに処理が完了したことを送信
      }
    }else{
      //コロンが含まれていない場合の(台座昇降)
      if(str == "UP_to_DOWN")
      {
        /*
        処理内容を記載(ケーキ台座昇降処理)
        リミットスイッチなどで昇降の完了を確認し次の動作に移る
        */
        Serial.println("OK"); //PCに処理が完了したことを送信
      }
    }
  }
}

//数字かどうかを確認する関数
bool isNumber(String str)
{
  for(int i = 0; i < str.length(); i++)
  {
    if(!isdigit(str.charAt(i)) && str.charAt(i) !='.' && str.charAt(i) != '-')
    {
      return false;
    }
  }
  return true;
}
