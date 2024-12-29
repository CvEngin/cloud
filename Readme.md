## 通讯协议设计

-  协议设计（采用弹性结构体设计，从而保证其能动态改变）

  - 总的消息大小：`uiPDULen`
  - 消息类型：`uiMsgType`
  - 文件名：`caData`
  - 实际消息大小：`uiMsgLen`
  - 实际消息：`csMsg`

- 数据收发

  <img src="D:\sourceCode\cloud\assets\image-20241229151854403.png" alt="image-20241229151854403" style="zoom: 25%;" />

  - 服务器监听

    <img src="D:\sourceCode\cloud\assets\image-20241229152305993.png" alt="image-20241229152305993" style="zoom: 25%;" />