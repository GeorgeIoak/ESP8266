#include <ESP8266WiFi.h>

const char* ssid     = "jyoun-soohee";//와이파이 이름
const char* password = "07040251995";//와이파이 비밀번호

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);  
  delay(1000);
  
  WiFi.disconnect();//기존 설정이 있을지 모르니 연결을 해제 합니다.

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);//와이파이 연결
  Serial.println(WiFi.SSID());// 저장된 와이파이 명확인

  WiFi.setAutoConnect(true);//자동 연결 설정
  WiFi.setAutoReconnect(true);//자동 재연결 설정
  WiFi.waitForConnectResult();//공유기에서 정보를 받을때까지 대기

  Serial.println(WiFi.localIP());//아이피 주소 출력
}

void loop() {
  // put your main code here, to run repeatedly:

}



