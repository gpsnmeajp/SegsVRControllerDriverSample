# SegsVRControllerDriverSample
OpenVRのコントローラドライバのサンプルです。   
日本語のコメントと、Windowsの共有メモリ経由で操作する機能がついています。  
  
srcがドライバのソースです。  
jsonで通信するためpicojsonに依存しています。  
  
ビルドしたい場合は以下のチュートリアルがわかりやすいです。  
https://github.com/terminal29/Simple-OpenVR-Driver-Tutorial  
  
bin以下がドライバです。  
C:\Program Files (x86)\Steam\steamapps\common\SteamVR\drivers  
に配置すると読み込まれます。  
  
clientは、共有メモリ経由で通信するコンソールアプリケーションのサンプルです。  
jsonで通信するためpicojsonに依存しています。  
キーボードで仮想コントローラの座標を制御できます。  
  
ドライバはOpenVRのドライバサンプルを改造して作成しています。  
https://github.com/ValveSoftware/openvr  
  
picojsonは以下からダウンロードできます。  
https://github.com/kazuho/picojson  
