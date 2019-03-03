# EasyDX
Make DirectX easy to use.

## Warning

Code is in a bad shape by now. This should be fixed in near future.

# TODO

* 更加一致的命名规则，包括对 `ID3D11DeviceContext` `Cb` 等；
* `CBStructs.hpp` 是一个糟糕的名字
* ~~`GameWindow` 不该存放 `*Buffer`~~;
* 移除 Warnings;
* ~~将目前的消息收发改为多线程~~；
* 恢复 Direct2D & DirectWrite 支持；
* ~~将目前的 Vertex 传入形式改为多 slots；~~
* 恢复原来的示例;
* 更多的 Shader 内置变量（参考 [Unity](https://docs.unity3d.com/Manual/SL-UnityShaderVariables.html)）；
* 对 Shader constant buffer 字段的名字的字符串使用 intern。

Special thanks to the [aMazing Project](https://github.com/AntiMoron/aMazing).