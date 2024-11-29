* Yoloのアノテーションデータを分割するユーティリティーです。
* Windows用です。
* 使い方 divfiles.exe 元データフォルダ 分割先フォルダ 分割割合
* 例 divfiles.exe c:\yoloyv5\data\my_data_org c:\yoloyv5\data\my_data_train
* 分割割合を80にすると、trainデータに80%が割り当てられます。

[元データフォルダ]  
 ┣images━jpgfiles  
 ┗labels━txtfiles  

[分割先フォルダ]  
 ┣train┳images━jpgfiles  
 ┃　　┗labels━txtfiles  
 ┗valid┳images━jpgfiles  
　　　 ┗labels━txtfiles  


Copy and divide files for yolo learning  
* usage: divfiles.exe [source_path] [dest_path] [divide_value:1-99]  
* example divfiles.exe c:\yoloyv5\data\my_data_org c:\yoloyv5\data\my_data_train
* If divide_value is 80, train data will get 80%.  

[source_path]  
┣images━jpgfiles  
┗labels━txtfiles  

[dest_path]  
┣train┳images━jpgfiles  
┃　　┗labels━txtfiles  
┗valid┳images━jpgfiles  
　　　 ┗labels━txtfiles  
