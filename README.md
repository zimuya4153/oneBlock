# oneBlock - 单方块空岛维度

此插件会创建一个名为`one_block`的维度，里面只会有一个无限随机生成的方块

> 温馨提示：生成的方块会自动排除`无法破坏`(如基岩),`无翻译方块`(如:乱码方块)

## 如何前往此维度？
- 打开聊天框，输入`/tp <玩家名> 0 1 0 one_block`

## 方块不生成怎么办？
- 放置一个方块，再挖掉

## 为什么生成不是循序渐进的？
- 我懒，直接就随机从已注册的方块中随机抽一个放置

## 支持addon方块吗？
- 支持

## 为什么不能设置重生点？
- 因为不是主世界

## 如何重载配置文件？
- 输入命令`/reload`

## 配置文件
```jsonc
{
    "version": 1, // 配置文件版本
    "blacklist": [ // 黑名单列表
        "minecraft:unknown", // 未知方块
        "minecraft:border_block" // 边界方块
    ]
}
```

## 安装方法

- 手动安装
  - 前往[Releases](https://github.com/zimuya4153/oneBlock/releases)下载最新版本的`oneBlock-windows-x64.zip`
  - 解压`压缩包内的`文件夹到`./plugins/`目录
- Lip 安装
  - 输入命令`lip install -y github.com/zimuya4153/oneBlock`
- ~~一条龙安装~~
  - ~~去 Q 群，喊人，帮你安装~~